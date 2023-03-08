#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <string>

using namespace cv;
using std::cout, std::endl;

//check out cv::buildPyramid() and pyrUp() / pyrDown()

//SIFT paper considers the gaussian function G(x,y,sigma)
//it uses scales seperated by a constant k
//D (x,y,sigma) = ( G(x,y,k*sigma) - G(x,y,sigma) ) conv* I(x,y)

//after each octave, the guassian image is downsampled to half the size.
//i.e. octave 0 --> G(x,y,sigma), octave 1 --> G(x/2, y/2, sigma)

//they set k = 2^(1/s), where is s is an integer number of times we divide the image in scale space.
//ex 4 octaves, s = 4, k = 2^(1/4).

//produce s+3 blurred images for each octave.. This part is the least clear.
//Do we incrementally or multiplicatively increase k on each iteration?
//


//per octave change, the sigma value is doubled while the image size is halved.
//so if we double sigma 4 times (i.e. 4 octave changes), we need 7 images in each octave?

//so basically, when computing the images of an octave, sigma stays the same
//what changes is the k factor

//does the s variable in k change depending on what octave you're currently in?
//ex: Octave 1:
//k = 2^(1/1) = 2

//Figure 1 says, for EACH octave, the intial image is repeatedly convolved w/ gaussians
//so it seems s DOES NOT change

//So what is the initial sigma value? 1?

//note, mathematically, due to the associative property of convolution, is it faster to
//convolve the gaussian kernels with each other, before convolving with the image.

//equally, you can blur an image with the same blur as two kernels by using
//new_sigma = sqrt( sig1**2 + sig2**2 )
//this will probably be the fastest method.
//https://math.stackexchange.com/questions/3159846/what-is-the-resulting-sigma-after-applying-successive-gaussian-blur
//for development sake though, it will just be fastest to blur each previous image.
class GaussPyramid
{
    public:
        GaussPyramid(Mat& img, int numOctaves, float sigma) : numOctaves_{numOctaves}, sigma_{sigma} { createPyramid(img); } 
        const std::map<int, std::vector<Mat>>& gaussPyramid() { return this->gauss_pyramid; }
        const std::map<int, std::vector<Mat>>& diffPyramid() { return this->diff_pyramid; }
        const std::vector<Mat>& getBlurOctave(int key) { return this->gauss_pyramid.at(key); }
        const std::vector<Mat>& getDiffOctave(int key) { return this->diff_pyramid.at(key); }
        static void displayPyramid(const std::map<int, std::vector<Mat>> pyramid);
        static void showOctave(const std::vector<Mat> images, const std::string window_name, const Point pos = Point(0,0));
    private:
        void createPyramid(Mat& img);
        std::vector<Mat> GaussVector(Mat& img);
        std::vector<Mat> Diff_of_Gauss(std::vector<Mat> gaussians);
        std::map<int, std::vector<Mat>> gauss_pyramid;
        std::map<int, std::vector<Mat>> diff_pyramid;
        int numOctaves_ = 0;
        int numImages_ = numOctaves_ + 3;
        float sigma_ = 0.0f;
        float k_ = pow(2.0f, 1.0f/float(numOctaves_));
};

void GaussPyramid::displayPyramid(const std::map<int, std::vector<Mat>> pyramid) {
    std::string window_name;
    for (const auto& kv: pyramid) {
        window_name = "Octave " + std::to_string(kv.first) + " blur";
        GaussPyramid::showOctave(kv.second, window_name, Point(kv.second.at(0).cols, 0));
    }
}

void GaussPyramid::showOctave(const std::vector<Mat> images, const std::string window_name, const Point pos) {
    Mat display;
    display = images.at(0);

    for (int i = 1; i < images.size(); ++i) {
        vconcat(display, images[i], display);
    }

    imshow(window_name, display);
    moveWindow(window_name, pos.x, pos.y);
}

void GaussPyramid::createPyramid(Mat& img) {
    //the sift paper states they double the size of the original image for the first level of the pyramid.
    //'double the size of the input image using linear interpolation prior to building the first level of the pyramid'
    Mat pyrBase;
    resize(img, pyrBase, Size(), 2, 2, INTER_LINEAR);

    for (int i = 0; i < this->numOctaves_; ++i) {
        //allocate 7 empty images to move the data over.
        std::vector<Mat> gaussians = GaussVector(pyrBase);
        std::vector<Mat> diffs = Diff_of_Gauss(gaussians);

        //The SIFT paper states that, they take a gaussian image w/ twice the initial value of sigma, this corresponds to
        //the 3rd image from the top (in our case, the 5th image)
        Mat octaveBase = gaussians.at(this->numOctaves_); //only copies the header

        //now with the resized image, repeat the two for the other levels of the pyramid.
        resize(octaveBase, pyrBase, Size(), 0.5, 0.5, INTER_NEAREST);
        
        this->gauss_pyramid.emplace(i, gaussians);
        this->diff_pyramid.emplace(i, diffs);
    }
}

std::vector<Mat> GaussPyramid::GaussVector(Mat& img) {
    std::vector<Mat> gaussians{this->numImages_, Mat::zeros(img.size(), CV_32FC1)};
    Mat blurred;
    for (int i = 0; i < this->numImages_; ++i) {
        
        if (i == 0 ) {
            GaussianBlur(img, blurred, Size(0,0), this->sigma_*this->k_, 0, BORDER_DEFAULT);
        }
        else if(i > 0) {
            GaussianBlur(gaussians[i-1], blurred, Size(0,0), this->sigma_*this->k_, 0, BORDER_DEFAULT);
        }
        
        //cv::swap(blurred, gaussians[i]);     //note that swap is just slightly slower (by .1~1 miliseconds)
        gaussians[i] = std::move(blurred);
        //each iteration, take the previous blurred image & blur it again.
    }
    return gaussians;
}

std::vector<Mat> GaussPyramid::Diff_of_Gauss(std::vector<Mat> gaussians) {
    //now, get a vector of difference of gaussians.
    std::vector<Mat> diffs{gaussians.size()-1, Mat::zeros(gaussians[0].size(), CV_32FC1)};
    for (int i = 1; i < gaussians.size(); i++) {
        //start i =1, therefore we can always grab the previous gaussian.
         //moving it mighttt be the problem? Since it's an rvalue in the first place and we're moving to an lvalue?
        diffs[i-1] = std::move(gaussians[i] - gaussians[i-1]);      
    }
    return diffs;
}




int main() {
    /*
        Goal: Create and display a gaussian pyramid (map)
        
    */

    std::string img_path = samples::findFile("building.jpg");
    Mat img_color = imread(img_path, IMREAD_COLOR);
    Mat img;
    cvtColor(img_color, img, COLOR_BGR2GRAY);

    int numOctaves = 4; //this represents the s variable in SIFT.
    float sigma = 1.6f;                        //paper states a sigma of 1.6
    GaussPyramid pyramid{img, numOctaves, sigma};


    for (const auto& kv: pyramid.gaussPyramid()) {
        cout << "Pyramid level: " << kv.first << endl;
        //cout << "size of data: " << kv.second.size() << endl;
        cout << "image size: width.height (" << kv.second.at(0).cols << ", " << kv.second.at(0).rows << ")" << endl;
    }
    
    //https://stackoverflow.com/questions/9905093/how-to-check-whether-two-matrices-are-identical-in-opencv
    //bool areIdentical = !cv::norm(downsized,downsized2,NORM_L1);
    //cout << "Downsized images equal?: " << areIdentical << endl;

    imshow("original image: ", img);        //erroring here for some reason..
    moveWindow("original image: ", 0,0);

    //imshow("downsampled image: ", downsized);
    //moveWindow("downsampled image: ", 0,img.rows + pyrBase.rows);

    //int octave = 3;

    //imshow("pyramid test", pyramid.getBlurOctave(octave).at(0));

    //std::string octave_window = "Octave 1 Blurs";
    //GaussPyramid::showOctave(pyramid.getBlurOctave(1), octave_window);

    std::string octave_window2 = "Octave 2 Blurs";
    GaussPyramid::showOctave(pyramid.getBlurOctave(2), octave_window2);

    std::string octave_window3 = "Octave 3 Blurs";
    GaussPyramid::showOctave(pyramid.getBlurOctave(3), octave_window3);
    //GaussPyramid::displayPyramid(pyramid.gaussPyramid());

    /*
    imshow("blurred1 image: ", gaussians.at(0));
    moveWindow("blurred1 image: ", 0,img.rows);

    imshow("blurred2 image: ", gaussians.at(1));
    moveWindow("blurred2 image: ", 0,img.rows*2);
    

    //show the Difference of Gaussian images.
    imshow("dog1 image: ", diffs.at(0));
    moveWindow("dog1 image: ", img.cols,img.rows);

    imshow("dog2 image: ", diffs.at(1));
    moveWindow("dog2 image: ", img.cols,img.rows*2);
    */
    

    waitKey(0);
    return 0;
}