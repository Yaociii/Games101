#include<cmath>
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
#include<iostream>

int main() {

    // Basic Example of cpp
    std::cout << "Example of cpp \n";
    float a = 1.0, b = 2.0;
    std::cout << a << std::endl;
    std::cout << a / b << std::endl;
    std::cout << std::sqrt(b) << std::endl;
    std::cout << std::acos(-1) << std::endl;
    std::cout << std::sin(30.0 / 180.0 * acos(-1)) << std::endl;

    // Example of vector
    std::cout << "Example of vector \n";
    // vector definition
    Eigen::Vector3f v(1.0f, 2.0f, 3.0f);
    Eigen::Vector3f w(1.0f, 0.0f, 0.0f);
    // vector output
    std::cout << "Example of output \n";
    std::cout << v << std::endl;
    // vector add
    std::cout << "Example of add \n";
    std::cout << v + w << std::endl;
    // vector scalar multiply
    std::cout << "Example of scalar multiply \n";
    std::cout << v * 3.0f << std::endl;
    std::cout << 2.0f * v << std::endl;
    //vector dot product
    std::cout << "Example of dot product i*j\n" << v.dot(w)<<std::endl;
    //vector cross product
    std::cout << "Example of cross product i*j\n" << v.cross(w)<<std::endl;

    // Example of matrix
    std::cout << "Example of matrix \n";
    // matrix definition
    Eigen::Matrix3f i, j;
    i << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0;
    j << 2.0, 3.0, 1.0, 4.0, 6.0, 5.0, 9.0, 7.0, 8.0;
    // matrix output
    std::cout << "Example of output \n";
    std::cout <<"i\n"<< i <<"\nj\n" <<j<< std::endl;
    // matrix add i + j
    std::cout << "Example of add \n" << i + j<<std::endl;
    // matrix scalar multiply i * 2.0
    std::cout << "Example of multiply i*2.0 \n" << i * 2<<std::endl;
    // matrix multiply i * j
    std::cout << "Example of i*j\n" << i * j<<std::endl;
    // matrix multiply vector i * v
    std::cout << "Example of multiply vector i*v\n" << i * v<<std::endl;


    //homework0:
    // 给定一个点P=(2,1),将该点绕原点先逆时针旋转45◦，再平移(1,2),计算出变换后点的坐标（要求用齐次坐标进行计算）。
    Eigen::Vector3f P(2.0f, 1.0f, 1.0f);
    std::cout << P << std::endl;
    Eigen::Matrix3f R, T;
    float cos45 = std::cos(std::acos(-1)/4);
    R << cos45,-1*cos45, 0.0, cos45, cos45, 0.0, 0.0, 0.0, 1.0;
    T << 1, 0, 1.0, 0, 1, 2.0, 0, 0, 1.0;
    Eigen::Vector3f nP = T * R * P;
    std::cout << nP << std::endl;
    return 0;
}