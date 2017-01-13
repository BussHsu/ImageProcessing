//#include <opencv2\opencv.hpp>
//
//using namespace cv;
//using namespace std;
//
////Implementation of Canny Edge Detector, thresholds must 0< t < 1
//Mat MyCanny(const Mat& src, float upperThreshold, float lowerThreshold, int size = 3)
//{
//	Mat workImg = Mat(src);
//	workImg = src.clone();
//
//	//blur
//	GaussianBlur(src, workImg, cv::Size(7, 7), 1.4f);
//	
//	//compute derivitive
//	Mat magX = Mat(src.rows, src.cols, CV_32F);
//	Mat magY = Mat(src.rows, src.cols, CV_32F);
//	Sobel(workImg, magX, CV_32F, 1, 0, size);
//	Sobel(workImg, magY, CV_32F, 0, 1, size);
//	Mat direction(src.rows, src.cols, CV_32F);
//	divide(magY, magX, direction);
//	Mat sum = Mat(workImg.rows, workImg.cols, CV_64F); 
//	Mat prodX = Mat(workImg.rows, workImg.cols, CV_64F); 
//	Mat prodY = Mat(workImg.rows, workImg.cols, CV_64F); 
//	cv::multiply(magX, magX, prodX); 
//	cv::multiply(magY, magY, prodY); 
//	sum = prodX + prodY; 
//	cv::sqrt(sum, sum);
//
//	//initialize the return image as all black
//	Mat res = Mat(workImg.rows, workImg.cols, CV_8U);
//	res.setTo(Scalar(0));
//
//	MatIterator_<float> itMag = sum.begin<float>();
//	MatIterator_<float> itDir = direction.begin<float>();
//	MatIterator_<unsigned char> itRes = res.begin<unsigned char>();
//	MatIterator_<float> endMag = sum.end<float>();
//
//
//	//scaling the threshold
//	double min, max;
//	cv::minMaxLoc(sum, &min, &max);
//	lowerThreshold = min + (max - min)*lowerThreshold;
//	upperThreshold = min + (max - min)*upperThreshold;
//
//	//non maxima suppression: check along the direction, if center is maxima among neighbors => pick it 
//	for (; itMag != endMag; itMag++, itDir++, itRes++)
//	{
//		//direction = atan(dy/dx)
//		float currDir = atan(*itDir)*180/3.142;
//		while (currDir < 0)
//			currDir += 180;
//		*itDir = currDir;
//
//		//magnitude<upperthreshold : skip
//		if (*itMag < upperThreshold)
//			continue;
//
//		//current position
//		const cv::Point pos = itRes.pos();
//		//flag indicate is edge point
//		bool isEdge = true;
//
//		int dirGroup = (currDir-22.5f) / 45;
//		switch (dirGroup) 
//		{
//		case 1:		// 22.5 <currDir <= 67.5
//		{
//			//check upper left neighbor
//			if (pos.x > 0 && pos.y > 0 && *itMag < sum.at<float>(pos.x - 1, pos.y - 1)) isEdge = false;
//			//check lower right neighbor
//			if (pos.x< sum.cols-1 && pos.y > sum.cols-1 && *itMag < sum.at<float>(pos.x + 1, pos.y + 1)) isEdge = false;
//		}
//			break;
//		case 2:		//67.5< currDir <= 112.5	(horizontal)
//		{
//			if (pos.y > 0 && *itMag<sum.at<float>(pos.x, pos.y-1)) isEdge = false;
//			if (pos.y < sum.cols - 1 && *itMag < sum.at<float>(pos.x, pos.y + 1)) isEdge = false;
//		}
//			break;
//		case 3:		//112.5<currDir < 157.5
//		{
//			//upper right neighbor
//			if (pos.x > 0 && pos.y < sum.cols - 1 && *itMag < sum.at<float>(pos.x-1, pos.y+1)) isEdge = false;
//			//lower left neighbro
//			if (pos.x< sum.cols - 1 && pos.y>0 && *itMag < sum.at<float>(pos.x + 1, pos.y - 1)) isEdge = false;
//		}
//			break;
//		default:	//currDir < 22.5 or currDir>157.5 (vertical)
//		{
//			if (pos.x > 0 && *itMag < sum.at<float>(pos.x - 1, pos.y)) isEdge = false;
//			if (pos.x< sum.cols - 1 && *itMag < sum.at<float>(pos.x + 1, pos.y)) isEdge = false;
//		}
//		}
//		
//		if (isEdge)
//			*itRes = 64;
//	}
//
//	//connecting weak edges
//	bool imgChanged = true;
//	while (imgChanged)
//	{
//		imgChanged = false;
//		for (itMag = sum.begin<float>(), itDir = direction.begin<float>(), itRes = res.begin<unsigned char>(); itMag != endMag; itMag++, itDir++, itRes++)
//		{
//			//skip if already checked or not edge
//			if (*itRes != 64)
//				continue;
//			
//			//mark as checked
//			*itRes = 255;
//			
//			cv::Point pos = itMag.pos();
//			int dirGroup = (*itDir - 22.5f) / 45;
//			switch (dirGroup)
//			{
//			case 1:		// 22.5 <currDir <= 67.5
//			{
//				//check upper right neighbor
//				if (pos.x > 0 && pos.y< sum.cols - 1 &&res.at<unsigned char>(pos.x - 1, pos.y + 1)!=255 && sum.at<float>(pos.x - 1, pos.y + 1) > lowerThreshold)
//				{
//					res.ptr<unsigned char>(pos.x - 1, pos.y + 1)[0] = 64;
//					imgChanged = true;
//				}
//				//check lower left neighbor
//				if (pos.x< sum.cols - 1 && pos.y>0 && res.at<unsigned char>(pos.x + 1, pos.y - 1) != 255 && sum.at<float>(pos.x + 1, pos.y - 1) > lowerThreshold)
//				{
//					res.ptr<unsigned char>(pos.x + 1, pos.y - 1)[0] = 64;
//					imgChanged = true;
//				}
//			}
//			break;
//			case 2:		//67.5< currDir <= 112.5	(gradient is horizontal)
//			{
//				//check upper
//				if (pos.x > 0 && res.at<unsigned char>(pos.x - 1, pos.y) != 255 && sum.at<float>(pos.x - 1, pos.y) > lowerThreshold)
//				{
//					res.ptr<unsigned char>(pos.x - 1, pos.y)[0] = 64;
//					imgChanged = true;
//				}
//				//check lower
//				if (pos.x< sum.rows - 1 && res.at<unsigned char>(pos.x + 1, pos.y) != 255 && sum.at<float>(pos.x + 1, pos.y)>lowerThreshold)
//				{
//					res.ptr<unsigned char>(pos.x + 1, pos.y)[0] = 64;
//					imgChanged = true;
//				}
//			}
//			break;
//			case 3:		//112.5<currDir < 157.5
//			{
//				//check upper left neighbor
//				if (pos.x > 0 && pos.y > 0 && res.at<unsigned char>(pos.x - 1, pos.y - 1) != 255 && sum.at<float>(pos.x - 1, pos.y - 1) > lowerThreshold)
//				{
//					res.ptr<unsigned char>(pos.x - 1, pos.y - 1)[0] = 64;
//					imgChanged = true;
//				}
//				//check lower right neighbor
//				if (pos.x< sum.cols - 1 && pos.y< sum.rows - 1 && res.at<unsigned char>(pos.x + 1, pos.y + 1) != 255 && sum.at<float>(pos.x + 1, pos.y + 1) > lowerThreshold)
//				{
//					res.ptr<unsigned char>(pos.x + 1, pos.y + 1)[0] = 64;
//					imgChanged = true;
//				}
//
//			}
//			break;
//			default:	//currDir < 22.5 or currDir>157.5 (gradient is vertical)
//			{
//				//check left
//				if (pos.y > 0 && res.at<unsigned char>( pos.y - 1, pos.x) != 255 && sum.at<float>( pos.y - 1, pos.x) > lowerThreshold)
//				{
//					res.ptr<unsigned char>( pos.y - 1, pos.x)[0] = 64;
//					imgChanged = true;
//				}
//				//check right
//				if (pos.y< sum.rows - 1 && res.at<unsigned char>(pos.x, pos.y + 1) != 255 && sum.at<float>(pos.x, pos.y + 1)>lowerThreshold)
//				{
//					res.ptr<unsigned char>(pos.x, pos.y + 1)[0] = 64;
//					imgChanged = true;
//				}
//			}
//			}
//
//		}
//	}
//	return res;
//}
//
//int main()
//{
//
//	int a;
//	Mat img1 = imread("lena.png", IMREAD_GRAYSCALE);
//	imshow("Canny Edge", MyCanny(img1, 0.3f, 0.15f));
//	//create a working copy
//	
//  //  if (img1.empty())
//  //  {
//  //      cout << "Cannot load image!" << endl;
//		//cin >> a;
//  //      return -1;
//  //  }
//
//  //  imshow("Original", img1);  // Original
//
//  //  // Create mask image
//  //  Mat mask(img1.rows, img1.cols, CV_8UC1, Scalar(255,255,255));
//  //  circle(mask, Point(img1.rows/2,img1.cols/3), 200, 0, -1);
//  //  imshow("Mask",mask);
//
//  //  // perform AND
//  //  Mat r;
//  //  bitwise_and(img1,mask,r);
//
//  //  // fill outside with white
//  //  const uchar white = 255;
//  //  for(int i = 0; i < r.rows; i++)
//  //      for(int j = 0; j < r.cols; j++)
//  //          if (!mask.at<uchar>(i,j))
//  //              r.at<uchar>(i,j)=white;
//
//  //  imshow("Result",r);
//
//    waitKey(0);
//    return 0;
//}

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

#include "MyImpl.h"
using namespace cv;

/// Global variables

Mat src, src_gray;
Mat dst, detected_edges;

int edgeThresh = 1;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";

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

	/// Using Canny's output as a mask, we display our result
	dst = Scalar::all(0);

	src.copyTo(dst, detected_edges);
	imshow(window_name, dst);
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
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);

	///// Create a Trackbar for user to enter threshold
	//createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);

	///// Show the image
	lowThreshold = 60;
	blur(src_gray, detected_edges, Size(3, 3));
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);
	imshow(window_name, detected_edges);

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