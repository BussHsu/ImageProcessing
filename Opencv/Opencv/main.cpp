#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

#include "MyImpl.h"
using namespace cv;

/// Global variables

Mat src, src_gray;
Mat dst, detected_edges, my_edges;

int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name_figure1 = "Edge Map";
char* window_name_figure2 = "My Edge Map";

/**
* @function CannyThreshold
* @brief Trackbar callback - Canny thresholds input with a ratio 1:3
*/
void CannyThreshold(int, void*)
{
	/// Reduce noise with a kernel 3x3
	blur(src_gray, detected_edges, Size(3, 3));

	/// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);
	my_edges = MyCanny(detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);
	/// Using Canny's output as a mask, we display our result
	//dst = Scalar::all(0);

	//src.copyTo(dst, detected_edges);
	imshow(window_name_figure1, detected_edges);
	imshow(window_name_figure2, my_edges);
}


/** @function main */

int main(int argc, char** argv)
{
	/// Load an image
	src = imread("sudoku.jpg");
	if (!src.data)
	{
		return -1;
	}

	/// Create a matrix of the same type and size as src (for dst)
	dst.create(src.size(), src.type());

	/// Convert the image to grayscale
	cvtColor(src, src_gray, CV_BGR2GRAY);

	//imshow("MyCanny", MyCanny(src_gray, 60, 20));
	/// Create a window
	namedWindow(window_name_figure1, CV_WINDOW_AUTOSIZE);
	namedWindow(window_name_figure2, CV_WINDOW_AUTOSIZE);
	///// Create a Trackbar for user to enter threshold
	//createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);

	///// Show the image
	lowThreshold = 60;
	/// Reduce noise with a kernel 3x3
	blur(src_gray, detected_edges, Size(3, 3));

	/// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);
	my_edges = MyCanny(detected_edges, 70, lowThreshold*ratio, kernel_size);
	/// Using Canny's output as a mask, we display our result
	//dst = Scalar::all(0);

	//src.copyTo(dst, detected_edges);
	imshow(window_name_figure1, detected_edges);
	imshow(window_name_figure2, my_edges);

	dst = MyHough(detected_edges);
	double min, max;
	minMaxLoc(dst, &min, &max);
	printf("%lf, %lf ", min, max);
	printf("%d, %d ", dst.rows, dst.cols);
	normalize(dst, dst, 0, 255, NORM_MINMAX);
	minMaxLoc(dst, &min, &max);
	printf("%lf, %lf ", min, max);
	Mat normImg;
	dst.convertTo(normImg, CV_8U);
	imshow("hough",normImg);
	/// Wait until user exit program by pressing a key
	waitKey(0);

	return 0;
}