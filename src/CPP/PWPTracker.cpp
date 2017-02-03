#include "PWPTracker.h"

// Default constructor
PWPTracker::PWPTracker(string template_path)
{
	// Initialize mats
	heavisidePhi = cv::Mat::zeros(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_32F);
	diracPhi = cv::Mat::zeros(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_32F);
	laplacianPhi = cv::Mat::zeros(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_32F);

	kernelx = (cv::Mat_<float>(1, 3) << -1, 0, 1);
	kernely = (cv::Mat_<float>(3, 1) << -1, 0, 1);

	// Read the template from the specified path
	model_template = cv::imread(template_path, CV_LOAD_IMAGE_GRAYSCALE);
	if (model_template.empty())
	{
		cout << "Error loading template" << endl;
		exit(EXIT_FAILURE);
	}

	// Convert template to binary image (0 or 1)
	cv::threshold(model_template, model_template, 1, 1, CV_THRESH_BINARY);
	
	// Calculate distance transform (-ve inside silhouette, +ve outside silhouette)
	cv::Mat temp;
	cv::distanceTransform(model_template, phi, CV_DIST_L2, KERNEL_SIZE);
	cv::distanceTransform(1 - model_template, temp, CV_DIST_L2, KERNEL_SIZE);
	//phi = phi - temp; // Inside contour is positive, outside contour is negative
	phi = temp - phi; // Inside contour is negative, outside contour is positive
	
	// Open the camera
	my_camera = cv::VideoCapture(0);
	if (!my_camera.isOpened())
	{
		cout << "Error loading camera" << endl;
		exit(EXIT_FAILURE);
	}
}

void PWPTracker::startTracking()
{
	char key_pressed;
	while (1)
	{
		// Read frame from the camera
		my_camera >> frame;
		if (frame.empty())
		{
			cout << "No frame found" << endl;
			break;
		}

		// Display the frame
		cv::imshow("Frame", frame);

		// Perform update
		calculateRequiredMatrices();
		calculatePixelwisePosteriors();
		updateShapeKernel();
		
		// Display images
		Utils::imagesc(phi, "Phi");
		Utils::imagesc(heavisidePhi, "Heaviside Phi");
		Utils::imagesc(diracPhi, "Dirac Delta Phi");
		cv::imshow("Pf", Pf);
		cv::imshow("Pb", Pb);
		
		// End program if q is pressed
		key_pressed = cv::waitKey(1);
		if (key_pressed == 'q')
		{
			cout << "User terminated program execution" << endl;
			break;
		}
		// Register
		else if (key_pressed == 'r')
		{

		}
	}
}


/******************* Private methods *******************/
void PWPTracker::calculateRequiredMatrices()
{
	// Calculate dirac delta and heaviside step functions
	for (int j = 0; j < phi.rows; j++) 
	{
		float* phiVal = phi.ptr<float>(j);
		float* heavisideVal = heavisidePhi.ptr<float>(j);
		float* diracVal = diracPhi.ptr<float>(j);

		for (int i = 0; i < phi.cols; i++) 
		{
			float val = *phiVal++;
			
			// Heaviside step function
			*heavisideVal++ = (1 / CV_PI) * (-atan(BAND_WIDTH * val) + (CV_PI / 2));

			// Dirac Delta step function
			*diracVal++ = (-BAND_WIDTH / CV_PI) * (1 / (1 + ((BAND_WIDTH * val) * (BAND_WIDTH * val))));
		}
	}

	// Calculate laplacian
	cv::Laplacian(phi, laplacianPhi, CV_32F, KERNEL_SIZE);

	// Calculate first order derivative	
	cv::filter2D(phi, gradientPhiX, -1, kernelx);
	cv::filter2D(phi, gradientPhiY, -1, kernely);
	cv::magnitude(gradientPhiX, gradientPhiY, gradientMagPhi); // sqrt(Ix.^2 + Iy.^2)
	cv::phase(gradientPhiX, gradientPhiY, gradientDirPhi); // atan(Iy, Ix)
}

void PWPTracker::calculatePixelwisePosteriors()
{
	// Initialize histogram
	foreground_histogram = cv::Mat::zeros(cv::Size(1, NUMBER_OF_BINS * NUMBER_OF_BINS * NUMBER_OF_BINS), CV_32F);
	background_histogram = cv::Mat::zeros(cv::Size(1, NUMBER_OF_BINS * NUMBER_OF_BINS * NUMBER_OF_BINS), CV_32F);
	//foreground_histogram = cv::Mat(3, hist_size, CV_32F, cv::Scalar(0)); // 3 defines num dims
	//background_histogram = cv::Mat(3, hist_size, CV_32F, cv::Scalar(0)); // 3 defines num dims
	nF = 0; nB = 0;
	int foreground_pixels = 0, background_pixels = 0;

	// Populate foreground and background histograms
	//cout << "Computing histograms" << endl;
	unsigned char *inputFrame = (unsigned char*)(frame.data);
	for (int j = 0; j < frame.rows; j++) 
	{
		int index = frame.step * j;
		float* heavisideVal = heavisidePhi.ptr<float>(j);
		for (int i = 0; i < frame.cols; i++) 
		{
			int b = inputFrame[index++];
			int g = inputFrame[index++];
			int r = inputFrame[index++];
			/*cout << b << "," << g << "," << r << endl;
			cout << (b / BIN_DIVISOR) << "," << (g / BIN_DIVISOR) << "," << (r / BIN_DIVISOR) << endl;*/
			int histIndex = (NUMBER_OF_BINS_SQ) * (b / BIN_DIVISOR) + (NUMBER_OF_BINS) * (g / BIN_DIVISOR) + (r / BIN_DIVISOR);
			float currentHeavisideVal = *heavisideVal++;
			// If foreground
			if (currentHeavisideVal > 0.5)
			{
				foreground_histogram.at<float>(histIndex)++; // Increment the counter
				foreground_pixels++;
				nF += currentHeavisideVal;
			}
			// If background
			else
			{
				background_histogram.at<float>(histIndex)++; // Increment the counter
				background_pixels++;
				nB += (1 - currentHeavisideVal);
			}
		}
	}

	// Normalize histogram
	foreground_histogram = foreground_histogram / foreground_pixels;
	background_histogram = background_histogram / background_pixels;

	//Mf = ((float)nF / (nF + nB));
	//Mb = ((float)nB / (nF + nB));
	//cout << nF << "," << nB << endl;
	//cout << Mf << "," << Mb << endl;

	// Generate posteriors probability maps
	//cout << "Computing posteriors" << endl;
	Pf = cv::Mat::zeros(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_32F);
	Pb = cv::Mat::zeros(cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT), CV_32F);

	inputFrame = (unsigned char*)(frame.data);
	for (int j = 0; j < frame.rows; j++)
	{
		int index = frame.step * j;
		float* pfVal = Pf.ptr<float>(j);
		float* pbVal = Pb.ptr<float>(j);

		for (int i = 0; i < frame.cols; i++)
		{
			int b = inputFrame[index++];
			int g = inputFrame[index++];
			int r = inputFrame[index++];

			int histIndex = (NUMBER_OF_BINS_SQ) * (b / BIN_DIVISOR) + (NUMBER_OF_BINS) * (g / BIN_DIVISOR) + (r / BIN_DIVISOR);
			float foregroundProb = foreground_histogram.at<float>(histIndex);
			float backgroundProb = background_histogram.at<float>(histIndex);
			
			//float denom = ((foregroundProb * Mf) + (backgroundProb * Mb));
			//*pfVal++ = (foregroundProb * Mf) / denom;
			//*pbVal++ = (backgroundProb * Mb) / denom; 
			/*
			*pfVal = (foregroundProb * Mf) / denom;
			*pbVal = (backgroundProb * Mb) / denom;
			cout << "Using m: " << *pfVal << ", " << *pbVal << endl; */
			
			float denom = ((foregroundProb * nF) + (backgroundProb * nB));
			*pfVal++ = (foregroundProb * nF) / denom;
			*pbVal++ = (backgroundProb * nB) / denom;
			//cout << "Using n: " << *pfVal << ", " << *pbVal << endl;

			//pfVal++;
			//pbVal++;

		}
	}
}

void PWPTracker::updateShapeKernel()
{
	shapeKernelGradient = Pf.mul(heavisidePhi) + Pb.mul(1 - heavisidePhi);
	shapeKernelGradient = diracPhi.mul(Pf - Pb) / shapeKernelGradient;
	shapeKernelGradient = shapeKernelGradient - (1.0 / SIGMA_SQ) * (laplacianPhi - gradientDirPhi);

	// Perform steepest ascent update
	phi = phi + shapeKernelGradient;
}