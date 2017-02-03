#pragma once

#include <iostream>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "Utils.h"

// Pre-processor directives
#define SIGMA_SQUARE 50.0
#define NUMBER_OF_BINS 32
#define NUMBER_OF_BINS_SQ 1024
#define BIN_DIVISOR 8 // 256/32
#define IMAGE_HEIGHT 480
#define IMAGE_WIDTH 640
#define BAND_WIDTH 3 // b in smooth heaviside and dirac delta step functions
#define KERNEL_SIZE 3
#define SIGMA_SQ 50 // sigma = sqrt(50)

using namespace std;

class PWPTracker
{
private:
	cv::VideoCapture my_camera;

	cv::Mat frame;
	cv::Mat model_template;

	cv::Mat phi;
	cv::Mat heavisidePhi;
	cv::Mat diracPhi;
	cv::Mat laplacianPhi;
	cv::Mat gradientPhiX, gradientPhiY;
	cv::Mat gradientMagPhi, gradientDirPhi;
	cv::Mat kernelx, kernely;
	cv::Mat shapeKernelGradient;
	
	// Pixelwise posterior
	const int hist_size[3] = { NUMBER_OF_BINS, NUMBER_OF_BINS, NUMBER_OF_BINS };
	cv::Mat foreground_histogram;
	cv::Mat background_histogram;
	cv::Mat Pf, Pb;
	float nB, nF; // Num pixels (sum of heaviside step function)
	float Mf, Mb;
	
	// Private functions
	void calculateRequiredMatrices();
	void calculatePixelwisePosteriors();
	void updateShapeKernel();

public:
	// Public functions
	PWPTracker(string template_path);
	void startTracking();
	
};