#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

class Utils
{
public:
	static std::string type2str(int type)
	{
		std::string r;

		uchar depth = type & CV_MAT_DEPTH_MASK;
		uchar chans = 1 + (type >> CV_CN_SHIFT);

		switch (depth) {
		case CV_8U:  r = "8U"; break;
		case CV_8S:  r = "8S"; break;
		case CV_16U: r = "16U"; break;
		case CV_16S: r = "16S"; break;
		case CV_32S: r = "32S"; break;
		case CV_32F: r = "32F"; break;
		case CV_64F: r = "64F"; break;
		default:     r = "User"; break;
		}

		r += "C";
		r += (chans + '0');

		return r;
	}

	static void imagesc(cv::Mat& img, std::string window_name)
	{
		double min;
		double max;
		cv::minMaxIdx(img, &min, &max);
		cv::Mat adjMap;
		// Histogram Equalization
		float scale = 255 / (max - min);
		img.convertTo(adjMap, CV_8UC1, scale, -min*scale);
		
		cv::Mat falseColorsMap;
		applyColorMap(adjMap, falseColorsMap, cv::COLORMAP_AUTUMN);

		cv::imshow(window_name, falseColorsMap);
	}
};
