#pragma once
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>

//Memo: src.at(i,j) is using (i,j) as (row,column) but Point(x,y) is using (x,y) as (column,row)
//Mat: row first (matrix index), Point: column first (cartesian inverse-y)

using namespace cv;
void DisplayHough(Mat& matHough);
//Impl1: Canny edge detector
Mat MyCanny(const Mat& src, float upperThreshold, float lowerThreshold, int size = 3)
{
	Mat workImg = Mat(src);
	workImg = src.clone();

	//blur
	cv::GaussianBlur(src, workImg, cv::Size(5, 5), 1.4);

	//compute derivitive
	Mat magX = Mat(src.rows, src.cols, CV_32F);
	Mat magY = Mat(src.rows, src.cols, CV_32F);
	Sobel(workImg, magX, CV_32F, 1, 0, size);
	Sobel(workImg, magY, CV_32F, 0, 1, size);
	Mat direction(src.rows, src.cols, CV_32F);
	divide(magY, magX, direction);
	Mat sum = Mat(workImg.rows, workImg.cols, CV_64F);
	Mat prodX = Mat(workImg.rows, workImg.cols, CV_64F);
	Mat prodY = Mat(workImg.rows, workImg.cols, CV_64F);
	cv::multiply(magX, magX, prodX);
	cv::multiply(magY, magY, prodY);
	sum = prodX + prodY;
	cv::sqrt(sum, sum);

	//initialize the return image as all black
	Mat res = Mat(workImg.rows, workImg.cols, CV_8U);
	res.setTo(Scalar(0));

	MatIterator_<float> itMag = sum.begin<float>();
	MatIterator_<float> itDir = direction.begin<float>();
	MatIterator_<unsigned char> itRes = res.begin<unsigned char>();
	MatIterator_<float> endMag = sum.end<float>();


	//scaling the threshold
	double min, max;
	cv::minMaxLoc(sum, &min, &max);

	//non maxima suppression: check along the direction, if center is maxima among neighbors => pick it 
	for (; itMag != endMag; itMag++, itDir++, itRes++)
	{
		//direction = atan(dy/dx)
		float currDir = atan(*itDir) * 180 / 3.142;
		while (currDir < 0)
			currDir += 180;
		*itDir = currDir;

		//magnitude<upperthreshold : skip
		if (*itMag < upperThreshold)
			continue;

		//current position
		const cv::Point pos = itRes.pos();
		//flag indicate is edge point
		bool isEdge = true;

		int dirGroup = (currDir - 22.5f) / 45;	//TODO:this is slower then direct conditioning
		switch (dirGroup)
		{
		case 1:		// 22.5 <currDir <= 67.5
		{
			//check upper left neighbor
			if (pos.x > 0 && pos.y > 0 && *itMag <= sum.at<float>(pos.y - 1, pos.x - 1)) isEdge = false;
			//check lower right neighbor
			if (pos.x < sum.cols - 1 && pos.y < sum.rows - 1 && *itMag <= sum.at<float>(pos.y + 1, pos.x + 1)) isEdge = false;
		}
		break;
		case 2:		//67.5< currDir <= 112.5	(horizontal)
		{
			if (pos.y > 0 && *itMag < sum.at<float>(pos.y - 1, pos.x)) isEdge = false;
			if (pos.y < sum.rows - 1 && *itMag < sum.at<float>(pos.y + 1, pos.x)) isEdge = false;

		}
		break;
		case 3:		//112.5<currDir < 157.5
		{
			//upper right neighbor
			if (pos.x > 0 && pos.y < sum.rows - 1 && *itMag < sum.at<float>(pos.y + 1, pos.x - 1)) isEdge = false;
			//lower left neighbro
			if (pos.x < sum.cols - 1 && pos.y>0 && *itMag < sum.at<float>(pos.y - 1, pos.x + 1)) isEdge = false;
		}
		break;
		default:	//currDir < 22.5 or currDir>157.5 (vertical)
		{
			//if (pos.x > 0 && *itMag < sum.at<float>(pos.x - 1, pos.y)) isEdge = false;
			//if (pos.x < sum.cols - 1 && *itMag < sum.at<float>(pos.x + 1, pos.y)) isEdge = false;
			if (pos.x > 0 && *itMag <= sum.at<float>(pos.y, pos.x - 1)) isEdge = false;
			if (pos.x < workImg.cols - 1 && *itMag <= sum.at<float>(pos.y, pos.x + 1)) isEdge = false;
		}
		}

		if (isEdge)
			*itRes = 255;
	}
	//connecting weak edges
	bool imgChanged = true;
	while (imgChanged)
	{
		imgChanged = false;
		for (itMag = sum.begin<float>(), itDir = direction.begin<float>(), itRes = res.begin<unsigned char>(); itMag != endMag; itMag++, itDir++, itRes++)
		{
			//skip if already checked or not edge
			if (*itRes != 64)
				continue;

			//mark as checked
			*itRes = 255;

			cv::Point pos = itMag.pos();
			int dirGroup = (*itDir - 22.5f) / 45;
			switch (dirGroup)
			{
			case 1:		// 22.5 <currDir <= 67.5
			{
				//check upper right neighbor
				if (pos.x > 0 && pos.y< sum.cols - 1 && res.at<unsigned char>(pos.y + 1, pos.x - 1) != 255 && sum.at<float>(pos.y + 1, pos.x - 1) > lowerThreshold)
				{
					res.ptr<unsigned char>(pos.y + 1, pos.x - 1)[0] = 64;
					imgChanged = true;
				}
				//check lower left neighbor
				if (pos.x < sum.cols - 1 && pos.y>0 && res.at<unsigned char>(pos.y - 1, pos.x + 1) != 255 && sum.at<float>(pos.y - 1, pos.x + 1) > lowerThreshold)
				{
					res.ptr<unsigned char>(pos.y - 1, pos.x + 1)[0] = 64;
					imgChanged = true;
				}
			}
			break;
			case 2:		//67.5< currDir <= 112.5	(gradient is horizontal)
			{
				//check upper
				if (pos.x > 0 && res.at<unsigned char>(pos.y, pos.x - 1) != 255 && sum.at<float>(pos.y, pos.x - 1) > lowerThreshold)
				{
					res.ptr<unsigned char>(pos.y, pos.x - 1)[0] = 64;
					imgChanged = true;
				}
				//check lower
				if (pos.x< sum.rows - 1 && res.at<unsigned char>(pos.y, pos.x + 1) != 255 && sum.at<float>(pos.y, pos.x + 1)>lowerThreshold)
				{
					res.ptr<unsigned char>(pos.y, pos.x + 1)[0] = 64;
					imgChanged = true;
				}
			}
			break;
			case 3:		//112.5<currDir < 157.5
			{
				//check upper left neighbor
				if (pos.x > 0 && pos.y > 0 && res.at<unsigned char>(pos.y - 1, pos.x - 1) != 255 && sum.at<float>(pos.y - 1, pos.x - 1) > lowerThreshold)
				{
					res.ptr<unsigned char>(pos.y - 1, pos.x - 1)[0] = 64;
					imgChanged = true;
				}
				//check lower right neighbor
				if (pos.x < sum.cols - 1 && pos.y< sum.rows - 1 && res.at<unsigned char>(pos.y + 1, pos.x + 1) != 255 && sum.at<float>(pos.y + 1, pos.x + 1) > lowerThreshold)
				{
					res.ptr<unsigned char>(pos.y + 1, pos.x + 1)[0] = 64;
					imgChanged = true;
				}

			}
			break;
			default:	//currDir < 22.5 or currDir>157.5 (gradient is vertical)
			{
				//check left
				if (pos.y > 0 && res.at<unsigned char>(pos.y - 1, pos.x) != 255 && sum.at<float>(pos.y - 1, pos.x) > lowerThreshold)
				{
					res.ptr<unsigned char>(pos.y - 1, pos.x)[0] = 64;
					imgChanged = true;
				}
				//check right
				if (pos.y< sum.rows - 1 && res.at<unsigned char>(pos.x, pos.y + 1) != 255 && sum.at<float>(pos.x, pos.y + 1)>lowerThreshold)
				{
					res.ptr<unsigned char>(pos.x, pos.y + 1)[0] = 64;
					imgChanged = true;
				}
			}
			}

		}
	}
	return res;
}

//Impl2: Hough Transform
// input edgeImg should be CV_8UC1
Mat MyHough(const Mat& edgeImg) 
{
	const int srcWidth = edgeImg.cols;
	const int srcHeight = edgeImg.rows;
	const int centerX = srcWidth / 2;
	const int centerY = srcHeight / 2;
	const int outWidth = 180;				//theta increment by 1 degree
	const int outHeight = ceil(sqrt(pow(srcWidth,2)+pow(srcHeight,2)));
	Mat out(outWidth, outHeight, CV_8U);

	for (int i = 0; i < edgeImg.rows; i++)
	{
		const uchar* currRow = edgeImg.ptr<uchar>(i);
		for (int j = 0; j < edgeImg.cols; j++)
		{
			//skip if not an edge point
			if (currRow[j] < 255)
				continue;
			for (int rowIdx = 0; rowIdx < outWidth; rowIdx++)
			{
				int theta = rowIdx - 90;		//theta from -90 to 89
				//rho is the distance (inner product with normal)
				int rho = (j - centerX)*sin(theta) + (i - centerY)*cos(theta);
				out.ptr<uchar>(rowIdx,rho+outHeight/2)[0]++;
			}
		}
	}

	return out;
}

//TODO: from hough matrix to line

void DisplayHough(Mat& matHough)
{
	for (int i = 0; i < matHough.rows; i++)
	{
		const int* currRow = matHough.ptr<int>(i);
		for (int j = 0; j < matHough.cols; j++)
		{
			printf("%u ",currRow[j]);
		}
		printf("\n");
	}
}