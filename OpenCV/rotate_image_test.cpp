#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

/*
    The purpose of this is to test the performance between
    1. (from scratch) matrix / vector rotation.
    2. Rotating an image using built-in opencv affine transforms   
*/

float convertToRadians(float theta) {
    return theta * (CV_PI/180.0f);
}

const Point2f cos_sin_of_angle(float theta, bool degrees=true) {
    float angle = 0;
    if (degrees) {
        angle = convertToRadians(theta);
    } else {
        angle = theta;                      //otherwise keep as radians
    }
    return Point2f(cos(angle), sin(angle));
}

Point2i rotate_pt_CW(const Point2i& pt, const Point2i& center, const Point2f& angles) {
    Point2i rot(pt.x,pt.y);
    rot -= center;
    int x_r = rot.x*angles.x - rot.y*angles.y;      //nearest-int interpolation on new pixels.
    int y_r = rot.x*angles.y + rot.y*angles.x;
    rot.x = x_r + center.x;
    rot.y = y_r + center.y;
    return rot;
}

Point2i rotate_pt_CCW(const Point2i& pt, const Point2i& center, const Point2f& angles) {
    Point2i rot(pt.x,pt.y);
    rot -= center;
    int x_r = rot.x*angles.x + rot.y*angles.y;
    int y_r = -rot.x*angles.y + rot.y*angles.x;
    rot.x = x_r + center.x;
    rot.y = y_r + center.y;
    return rot;
}


Point2i rotate_pt_CCW(const Point2i& pt, const Point2i& center, float theta, bool degrees=true) {
    Point2i rot(pt.x,pt.y);
    const Point2f angles = cos_sin_of_angle(theta, degrees);

    rot -= center;
    int x_r = rot.x*angles.x + rot.y*angles.y;
    int y_r = -rot.x*angles.y + rot.y*angles.x;
    rot.x = x_r + center.x;
    rot.y = y_r + center.y;
    return rot;
}

//see: https://en.wikipedia.org/wiki/Rotation_matrix
//note: this is for GRAYSCALE only. Will need to be adapted for color images.
Mat rotate_mat_CCW(Mat& I, const Point2i& center, const Point2f& angles) {
    Mat rotated = Mat::zeros(I.size(), I.type());
    
    Point2i pt(0, 0);
    for (int y = 0; y < I.rows; ++y) {
        for (int x = 0; x < I.cols; ++x) {
            pt = rotate_pt_CW(Point2i(x,y), center, angles);
            if (pt.x < I.cols && pt.x > 0 && pt.y < I.rows && pt.y > 0) {
                rotated.at<uchar>(y,x) = I.at<uchar>(pt.y,pt.x);
            }
            else {
                rotated.at<uchar>(y,x) = (uchar)0;
            }
        
        }
    }

    return rotated;
}

Mat getRotatedWindow(Mat& I, const Point2i& center, int windowSize, float theta, bool degrees=true) {
    int padding = windowSize/2;
    Point2i windowStart(center.x - padding, center.y - padding);
    Point2i windowEnd(center.x + padding, center.y + padding);

    Mat ROI = Mat::zeros(windowSize, windowSize, I.type());
    const Point2f angles = cos_sin_of_angle(theta, degrees);


    //take a window around the center point. 
    //Check if point's window is within image.
    //note! Be careful about this part, we need to account for the rotated window to be within the
    //image, NOT the normal window!
    if (windowStart.x > 0 && windowEnd.x < I.cols && windowStart.y > 0 && windowEnd.y < I.rows) {
        //the point's window won't expand beyond image border. No padding.
    }
    else {
        //if the point's window will expand beyond the image border, it will need padding.
    }


    //iterate over the coordinates which will surround the point.
    Point2i pt_rotated(0,0);
    for (int i = windowStart.y; i <= windowEnd.y; ++i) {
        for (int j = windowStart.x; j <= windowEnd.x; ++j) {
            pt_rotated = rotate_pt_CW(Point2i(j,i), center, angles);
            ROI.at<uchar>(i - windowStart.y, j-windowStart.x) = I.at<uchar>(pt_rotated.y, pt_rotated.x);
        }
    }

    return ROI;
}

Point drawRotated(Point& pt, Mat& src, Mat& roi_rotation_mat, bool draw=true) {
    Mat pos = Mat(Vec3d(pt.x, pt.y, 1));    //convert pt to mat for multiplication
    
    //Now, rotate the point.
    Mat pos_rotated = roi_rotation_mat*pos;
    Point pt_rotated((int)pos_rotated.at<double>(0), (int)pos_rotated.at<double>(1));

    if (draw) {
        circle(src, pt, 3, Scalar(255,255,255), 2);        //draw white circle
        circle(src, pt_rotated, 3, Scalar(255,0,255), 2);
    }

    return pt_rotated;
}

Mat doubleCrop(Mat& src, const Point2i& center, int windowSize, double angle) {
    int padding = windowSize/2;
    double scale = 1.0;      //scaling is optional

    //generate the rotation matrix with cv::getRotationMatrix2D
    Mat rotation_mat = getRotationMatrix2D(center, angle, scale);


    //Make a function out of this
    Point pos1(center.x - padding, center.y - padding);     //top left
    Point pos1_r = drawRotated(pos1, src, rotation_mat, false);

    Point pos2(center.x + padding, center.y - padding);     //top right
    Point pos2_r = drawRotated(pos2, src, rotation_mat, false);

    Point pos3(center.x - padding, center.y + padding);     //bottom left
    Point pos3_r = drawRotated(pos3, src, rotation_mat, false);

    Point pos4(center.x + padding, center.y + padding);     //bottom right
    Point pos4_r = drawRotated(pos4, src, rotation_mat, false);

    //note, this actually needs to be performed for every single pixel..
    std::vector<int> x_positions = {pos1_r.x, pos2_r.x, pos3_r.x, pos4_r.x};
    std::vector<int> y_positions = {pos1_r.y, pos2_r.y, pos3_r.y, pos4_r.y};
    
    int min_x = *min_element(x_positions.begin(), x_positions.end());
    int max_x = *max_element(x_positions.begin(), x_positions.end());
    int min_y = *min_element(y_positions.begin(), y_positions.end());
    int max_y = *max_element(y_positions.begin(), y_positions.end());

    //what about constructing these into points, then applying the rotation.
    //Will that give the cropping boundaries?

    //Now, construct a rect with these new points
    Mat ROI(src, Rect(min_x, min_y, max_x-min_x, max_y-min_y));

    //now, rotate the ROI.
    Point roi_center = Point(ROI.cols/2, ROI.rows/2);
    Mat roi_rotation_mat = getRotationMatrix2D(roi_center, angle, scale);

    Point roi_pos1(ROI.cols/2, 0);           //top
    Point roi_pos2(0,ROI.rows/2);           //left
    Point roi_pos3(ROI.cols, ROI.rows/2);    //right
    Point roi_pos4(ROI.cols/2, ROI.rows);    //bottom
    Point roi_rot_pos1 = drawRotated(roi_pos1, ROI, roi_rotation_mat, false);      //use this
    Point roi_rot_pos2 = drawRotated(roi_pos2, ROI, roi_rotation_mat, false);
    Point roi_rot_pos3 = drawRotated(roi_pos3, ROI, roi_rotation_mat, false);
    Point roi_rot_pos4 = drawRotated(roi_pos4, ROI, roi_rotation_mat, false);      //use this

    Mat rotate_dst;
    warpAffine(ROI, rotate_dst, roi_rotation_mat, ROI.size());

    //finally, take a new ROI from the rotate_dst
    Mat cropped(rotate_dst, Rect(roi_rot_pos1, roi_rot_pos4));
    return cropped;
}

const char* window_name = "Rotation Demo";
const char* trackbar_rotation = "Rotation";

//const char* trackbar_rotation2 = "Rotation 2";
//const char* window_name2 = "Rotate Point";
std::string img_string = samples::findFile("blox.jpg");
Mat img = imread(img_string, IMREAD_GRAYSCALE);
int rotation = 0;
int maxRotation = 360;

//float angle_deg = 180.0f;       //rotating 180 degrees looks fine, but 90 absolutely destroys the image.
int rows = img.rows;
int cols = img.cols;
const Point2i origin(0, 0);
const Point2i center(cols/2, rows/2);
Point2f angles = cos_sin_of_angle(rotation);
Point2i pt(cols/2, 0);  //should be mid top.      //x = cols, y = rows
int windowSize = 16;


static void Rotation_Demo(int, void*) {
    angles = cos_sin_of_angle(rotation);
    Mat rotated = rotate_mat_CCW(img, center, angles);
    imshow(window_name, rotated);
}


/*
static void Rotation_Point_Demo(int, void*) {
    Point2i rot = rotate_pt_CCW(pt, center, rotation);
    circle(img, rot, 3, Scalar(255, 0, 255), 3);
    imshow(window_name2, img);
}
*/

int main() {
    /*
        Test the following:
        - obtain a 50x50 window rotated at 45 degrees of the original image
        1. my custom method of getting the positions, then rotating those and accessing them
        2. the OpenCV method of cropping the minimum values before rotation, then rotating and cropping again.
    
    */


    //will break if all 3 are run at the same time (too much memory allocated on stack)
    const int times = 100;
    double t = (double)getTickCount();
    /*
    for (int i = 0; i < times; i++) {
        //rotate matrix here
        rotate_mat_CCW(img, center, angles);
    }
    t = 1000 * ((double)getTickCount() - t) / getTickFrequency();
    t /= times;
    cout << "Time rotating image (averaged for " << times << " runs): " << t << " milliseconds." << endl;
    */


    //my custom implementation (working)
    Mat ROI = getRotatedWindow(img, center, 100, 45.0f);
    t = (double)getTickCount();
    for (int i = 0; i < times; i++) {
        //rotate matrix here
        getRotatedWindow(img, center, windowSize, 45.0f);
    }
    t = 1000 * ((double)getTickCount() - t) / getTickFrequency();
    t /= times;
    cout << "Time rotating 16x16 ROI (averaged for " << times << " runs): " << t << " milliseconds." << endl;

   

   //compare w/ previous double crop method
   Mat ROI2 = doubleCrop(img, center, 100, 45.0);
   t = (double)getTickCount();
   for (int i = 0; i < times; i++) {
        //rotate matrix here
        doubleCrop(img, center, windowSize, 45.0);
    }
    t = 1000 * ((double)getTickCount() - t) / getTickFrequency();
    t /= times;
    cout << "Time rotating 16x16 ROI with the double crop method (averaged for " << times << " runs): " << t << " milliseconds." << endl;

    namedWindow(window_name, WINDOW_AUTOSIZE);
    //namedWindow(window_name2, WINDOW_AUTOSIZE);

    //create a trackbar PER variable you want to input into the function.
    createTrackbar(trackbar_rotation, window_name, &rotation, maxRotation, Rotation_Demo);
    Rotation_Demo(0,0); //call function to initialize


    //createTrackbar(trackbar_rotation2, window_name2, &rotation, maxRotation, Rotation_Point_Demo);
    //Rotation_Point_Demo(0,0); //call function to initialize

    imshow("ROI", ROI);
    imshow("ROI2", ROI2);
    
    waitKey();
    return 0;
}