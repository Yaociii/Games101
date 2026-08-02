// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


//将x,y的类型改为float以便传入中心点坐标及后续SSAA的实现


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    //先定义好受检点、三角形的边、受检点与顶点连线
    Eigen::Vector3f p(x, y, 0);
    Eigen::Vector3f s[3];
    s[0] = _v[1] - _v[0], s[1] = _v[2] - _v[1], s[2] = _v[0] - _v[2];
    Eigen::Vector3f v[3];
    for (int i = 0; i < 3; i++)
    {
        v[i] = p - _v[i];
    }

    //借助叉乘所得z坐标判断是否在内
    float cross_z[3] = { 0 };
    for (int i = 0; i < 3; i++)
    {
        cross_z[i] = (v[i].cross(s[i])).z();
    }
    if ((cross_z[0] > 0 && cross_z[1] > 0 && cross_z[2] > 0) || (cross_z[0] < 0 && cross_z[1] < 0 && cross_z[2] < 0))  return true;
    return false;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();//返回顶点数组
    
     //TODO : Find out the bounding box of current triangle.
     //iterate through the pixel and find if the current pixel is inside the triangle
    float xM=v[0].x(), xm=v[0].x(), yM=v[0].y(), ym=v[0].y();
    for (int i = 1; i < 3; i++)
    {
        xM = std::max(xM, v[i].x());
        xm = std::min(xm, v[i].x());
        yM = std::max(yM, v[i].y());
        ym = std::min(ym, v[i].y());
    }
    Eigen::Vector3f _v[3];

   for (int i = 0; i < 3; i++) _v[i] = v[i].head<3>();
   //
   ////以下为基础实现
   ////注意计算时均需使用中心点坐标 
    for (int x = xm; x <= xM; x++)
        for (int y = ym; y <= yM; y++)
        {

            // If so, use the following code to get the interpolated z value.
   auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
   float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
   float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
   z_interpolated *= w_reciprocal;

   // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.


            if (insideTriangle((float)x+0.5, (float)y+0.5, _v))
            {
                auto[alpha, beta, gamma] = computeBarycentric2D((float)x+0.5, (float)y+0.5, t.v);
                float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                int index = rst::rasterizer::get_index(x, y);
                if (z_interpolated >depth_buf[index])
                {
                    depth_buf[index] = z_interpolated;
                    Eigen::Vector3f p(x, y, 0);
                    set_pixel(p, t.getColor());
                }
            }
        }
 
  

    // 提高项：SSAA的实现
    //思路：将每个像素拆分为4个进行检验，对于像素内的每一个样本都需要维护它自己的深度值，即每一个像素都需要维护一个 sample list
   //最终颜色应有四个样本平均得出,为此还需维护每个样本的颜色

   //for(int x=xm;x<=xM;x++)
   //    for (int y = ym; y <= yM; y++)
   //    {
   //        int ind = get_index(x, y);
   //        int i = 0;
   //        for(float dx=0.25;dx<=1;dx+=0.5)
   //            for (float dy = 0.25; dy <=1; dy+=0.5,i++)
   //            {
   //                float px = (float)x + dx;
   //                float py = (float)y + dy;
   //                if (insideTriangle(px, py, _v))
   //                {
   //                    auto[alpha, beta, gamma] = computeBarycentric2D(px, py, t.v);
   //                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
   //                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
   //                    z_interpolated *= w_reciprocal;
   //                    if (z_interpolated > this->ssaa_depth_buf[ind][i])
   //                    {
   //                        this->ssaa_depth_buf[ind][i] = z_interpolated;
   //                        this->ssaa_frame_buf[ind][i] = t.getColor();
   //                    }
   //                    //最终平均得到颜色
   //                    if (i == 3)
   //                    {
   //                        this->frame_buf[ind] = ssaa_frame_buf[ind][0] + ssaa_frame_buf[ind][1] + ssaa_frame_buf[ind][2] + ssaa_frame_buf[ind][3];
   //                        this->frame_buf[ind] /= 4;
   //                    }
   //                }
   //            }

   //       
   //    }
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        for (auto& row :this-> ssaa_frame_buf)
            std::fill(row.begin(), row.end(), Eigen::Vector3f{ 0,0,0 });
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        //初始化改为负无穷以使用负数深度值进行比较

        std::fill(depth_buf.begin(), depth_buf.end(), -std::numeric_limits<float>::infinity());
        for (auto& row : this->ssaa_depth_buf)
            std::fill(row.begin(), row.end(), -std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    this->ssaa_frame_buf.resize(w * h);
    for (auto& row : this->ssaa_frame_buf)
        row.resize(4, Eigen::Vector3f{ 0,0,0 });
    this->ssaa_depth_buf.resize(w * h);
    for (auto& row : this->ssaa_depth_buf)
        row.resize(4, -std::numeric_limits<float>::infinity());
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

