// clang-format off
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.hpp"
#include "global.hpp"
#include "Triangle.hpp"

//你需要修改的函数如下：
//• rasterize_triangle() : 执行三角形栅格化算法
//• static bool insideTriangle() : 测试点是否在三角形内。你可以修改此函
//数的定义，这意味着，你可以按照自己的方式更新返回类型或函数参数。
//[提高项 5 分] 用 super - sampling 处理 Anti - aliasing : 你可能会注意
//到，当我们放大图像时，图像边缘会有锯齿感。我们可以用 super - sampling
//来解决这个问题，即对每个像素进行 2 * 2 采样，并比较前后的结果(这里
//    并不需要考虑像素与像素间的样本复用)。需要注意的点有，对于像素内的每
//    一个样本都需要维护它自己的深度值，即每一个像素都需要维护一个 sample
//    list。最后，如果你实现正确的话，你得到的三角形不应该有不正常的黑边。


constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
    // TODO: Copy-paste your implementation from the previous assignment.
    Eigen::Matrix4f projection;
   

    //由于实际传入的zNear与zFar为正值，在后续计算中需乘-1
    float width, height;
    height = 2 * zNear * std::tan(0.5 * eye_fov * MY_PI / 180.0);
    width = height * aspect_ratio;

    Eigen::Matrix4f proToOrt = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f ort = Eigen::Matrix4f::Identity();

    proToOrt << -1 * zNear, 0, 0, 0,
        0, -1 * zNear, 0, 0,
        0, 0, -1 * (zNear + zFar), -1 * zNear * zFar,
        0, 0, 1, 0;
    ort << 2 / width, 0, 0, 0,
        0, 2 / height, 0, 0,
        0, 0, 2 / (zFar - zNear), -1 * (zNear + zFar) / (zNear - zFar),
        0, 0, 0, 1;

    projection = ort * proToOrt;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc == 2)
    {
        command_line = true;
        filename = std::string(argv[1]);
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0,0,5};


    std::vector<Eigen::Vector3f> pos
            {
                    {2, 0, -2},
                    {0, 2, -2},
                    {-2, 0, -2},
                    {3.5, -1, -5},
                    {2.5, 1.5, -5},
                    {-1, 0.5, -5}
            };

    std::vector<Eigen::Vector3i> ind
            {
                    {0, 1, 2},
                    {3, 4, 5}
            };

    std::vector<Eigen::Vector3f> cols
            {
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0}
            };

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    auto col_id = r.load_colors(cols);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1,50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
    }

    return 0;
}
// clang-format on