#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <opencv2/core/types.hpp>

using namespace cv;

/*
    Given a matrix:
    [a11 a12 a13]
    [a21 a22 a23]
    [a31 a32 a33]

    Where, a<row><col>

    OpenCV uses row-major order [i.e. follow across the row]:
    - a11 --> a12 --> a13 --> a21 --> a22 ...
    --> this means mat.cols = width, mat.rows = height

    OpenGL uses column-major order [i.e. follow down the col]:
    - a11 --> a21 --> a31 --> a12 --> a22 ...
*/


//Operations I need to be able to perform:
/*
    vector transpose x
    vector * vector --> scalar (or vector) x
    vector * vector --> Matrix x
    matrix transpose x
    matrix inverse x
    matrix * vector --> vector x
    
    if these operations cannot be performed,
    choose a linear algebra library.

    Comparing OpenCV, GLM, Eigen
    https://www.mgaillard.fr/2021/12/29/matrix-vector-product.html
    note: 'Never use OpenCV’s cv::Mat for 3D geometry computations, it is extremely slow!'
*/

int main() {
    //0. Ensure printing and iteration is done in the same fasion.
    Mat test = (Mat_<int>(3,3) << 1, 2, 3, 4, 5, 6, 7, 8, 9);
    std::cout << "Test (print): " << std::endl;
    std::cout << test << std::endl;


    std::cout << "Iterating test: " << std::endl;
    for (int i = 0; i < test.rows; i++) {
        std::cout << "[";
    for (int j = 0; j < test.cols; j++) {
        //! note: Mat::at<type>(row, col)
        std::cout << test.at<int>(i,j) << ", ";
    }
        std::cout << "]" << std::endl;
    }


    //1. vector transpose
    //NOTE! OpenCV considers A a 3 row x 1 col vector.
    Vec3f a(1.0f, 2.0f, 3.0f);       //seems 1x3, but is actually 3x1 when converted to matrix.
    Vec3f b(3.0f, 5.0f, 7.0f);
    Mat A = Mat(a);                 
    Mat B = Mat(b);

    std::cout << "vector (a): " << a << std::endl;
    std::cout << "Vector (Mat A): " << std::endl << A << std::endl;
    std::cout << "Matrix (A) RowsxCols: " << std::endl << A.rows << "x" << A.cols << std::endl;

    b[2] = 2.0f;
    std::cout << "vector (b): " << b << std::endl;
    std::cout << "Vector (Mat B): " << std::endl << B << std::endl;

    transpose(B,B);
    std::cout << "Matrix (B) tranpose: " << B << std::endl;
    std::cout << "Matrix (B) tranpose RowsxCols: " << std::endl << B.rows << "x" << B.cols << std::endl;


    //Order and shape matter.
    //2. vec*vec --> Scalar [B (1x3) * A (3*1)] = 1x1 Scalar
    Mat C;
    C = B*A;
    std::cout << "vec*vec --> Scalar (C): " << std::endl << C << std::endl;


    //3: vector * vector --> Matrix [A (3x1) * B (1x3) ] = 3x3 Matrix
    Mat D;
    D = A*B;
    std::cout << "vec*vec --> Matrix (D): " << std::endl << D << std::endl;


    //4. Matrix transpose
    Mat D_transpose;
    transpose(D, D_transpose);
    std::cout << "Matrix (D) tranpose: " << std::endl << D_transpose << std::endl;


    //5. Matrix inverse
    //note: the inverse may not exist if: the matrix is not square OR the determinant is 0 or divides by 0.
    Mat D_inverse;
    D_inverse = D.inv();
    std::cout << "Matrix (D) inverse: " << std::endl << D_inverse << std::endl;

    //5.a determinant
    std::cout << "Matrix (D) determinant: " << determinant(D) << std::endl;


    //6. matrix*vec --> vector [D (3x3) * A (3x1) ] = 3x1 vector
    Mat E;
    E = D*A;
    std::cout << "mat*vec --> Vector (E): " << std::endl << E << std::endl;

    return 0;
}