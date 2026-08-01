#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
//homework1
//以下是你需要在 main.cpp 中修改的函数（请不要修改任何的函数名和其他
//已经填写好的函数，并保证提交的代码是已经完成且能运行的）：
//• get_model_matrix(float rotation_angle) : 逐个元素地构建模型变换矩
//阵并返回该矩阵。在此函数中，你只需要实现三维中绕 z 轴旋转的变换矩阵，
//而不用处理平移与缩放。
//• get_projection_matrix(float eye_fov, float aspect_ratio, float
//    zNear, float zFar) : 使用给定的参数逐个元素地构建透视投影矩阵并返回
//    该矩阵。
//    •[Optional] main() : 自行补充你所需的其他操作。

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    //只需注意将角度转为弧度
    model << std::cos((rotation_angle / 180)*MY_PI), -std::sin((rotation_angle / 180)*MY_PI), 0, 0,
        std::sin((rotation_angle / 180) * MY_PI), std::cos((rotation_angle / 180) * MY_PI), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    //由于实际传入的zNear与zFar为正值，在后续计算中需乘-1
    float width,height;
    height = 2 * zNear * std::tan(0.5 * eye_fov * MY_PI / 180.0);
    width = height * aspect_ratio;

    Eigen::Matrix4f proToOrt = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f ort = Eigen::Matrix4f::Identity();

    proToOrt << -1*zNear, 0, 0, 0,
        0, -1*zNear, 0, 0,
        0, 0, -1*(zNear + zFar), -1 * zNear * zFar,
        0, 0, 1, 0;
    ort << 2 / width, 0, 0, 0,
        0, 2 / height, 0, 0,
        0, 0, 2 / (zFar-zNear), -1 * (zNear + zFar) / (zNear-zFar),
        0, 0, 0, 1;

    projection = ort * proToOrt;

    return projection;
}

//[提高项 5 分] 在 main.cpp 中构造一个函数，该函数的作用是得到绕任意
//过原点的轴的旋转变换矩阵。
Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    //弧度转角度
    angle = angle * MY_PI / 180;
    Eigen::Matrix4f rotation = Matrix4f::Identity();
    //旋转轴归一化
    Eigen::Vector3f n = axis.normalized();
    float c = std::cos(angle);
    float s = std::sin(angle);
    //罗德里格斯公式计算三维矩阵
    Eigen::Matrix3f rot3 = Matrix3f::Identity();
    //先得到n的对偶矩阵
    Eigen::Matrix3f skew;
    skew << 0, -1 * (n.z()), n.y(), n.z(), 0, -1 * n.x(), -1 * n.y(), n.x(), 0;
    rot3 = c * rot3 + (1 - c) * n * n.transpose() + s * skew;
    //写入旋转矩阵
    rotation.block<3, 3>(0, 0) = rot3;
    return rotation;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);
        static Eigen::Vector3f axis(0, 0, 1);
        if (key == 13)
        {
            std::cin >> axis.x() >> axis.y() >> axis.z();
        }
        r.set_model(get_rotation(axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
