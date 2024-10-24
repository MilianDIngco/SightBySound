#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <opencv4/opencv2/calib3d.hpp>
#include <iostream>

int main() {
	cv::VideoCapture cap(0);
	cv::VideoCapture cap1(1);

	if (!cap.isOpened()) {
		std::cerr << "Error: Could not open webcam" << std::endl;
		return -1;
	}

	if (!cap1.isOpened()) {
		std::cerr << "Error: Could not open second webcam" << std::endl;
		return -1;
	}

	cv::Mat frame;
	cv::Mat frame2;
	cv::Mat output;
	while (true) {
	cap >> frame;
	cap1 >> frame2;
	
	cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
	cv::cvtColor(frame2, frame2, cv::COLOR_BGR2GRAY);

	auto stereo = cv::StereoBM::create(16, 15);
	stereo->compute(frame2, frame, output);
	
	cv::imshow("Webcam Live Feed2", output);

	if (cv::waitKey(5000) == 'q') {
		break;
	} 

	}

	/*
	while (true) {
		cap >> frame;
		cap1 >> frame2;

		if (frame.empty()) {
			std::cerr << "Error: Could not capture a frame." << std::endl;
			break;
		}
		if (frame2.empty()) {
			std::cerr << "Error: Could not capture a frame." << std::endl;
			break;
		}

		cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(frame, frame, cv::Size(5, 5), 1.5);
		double low_threshold = 100;
		double high_threshold = 200;
		cv::Canny(frame, frame, low_threshold, high_threshold);

		// Display frame in a window
		cv::imshow("Webcam Live Feed", frame);
		cv::imshow("Webcam Live Feed2", frame2);

		// Wiat 1 ms and break if 'q' is pressed
		if (cv::waitKey(1) == 'q') {
			break;
		}
	}*/



	// init ORB detector 
	/*
	int nfeatures = 500;
	cv::Ptr<cv::ORB> orb = cv::ORB::create(nfeatures);


	cv::Mat frame_grad_x, frame_grad_y, frame_abs_x, frame_abs_y, frame_edge;
	cv::Mat frame2_grad_x, frame2_grad_y, frame2_abs_x, frame2_abs_y, frame2_edge;

	
	std::vector<cv::KeyPoint> keypoints1, keypoints2;
	cv::Mat descriptors1, descriptors2;

	cv::BFMatcher matcher(cv::NORM_HAMMING, true);
	std::vector<cv::DMatch> matches;

	cv::Mat img_matches;

	while(true) {
		cap >> frame;
		cap1 >> frame2;
	
		if (frame.empty()) {
			std::cerr << "Error: Could not capture a frame." << std::endl;
			return -1;	
		}
		if (frame2.empty()) {
			std::cerr << "Error: Could not capture a frame." << std::endl;
			return -1;
		}

		cv::resize(frame, frame, cv::Size(), .3, .3);
		cv::resize(frame2, frame2, cv::Size(), .3, .3);

		//make grayscale
		cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
		cv::cvtColor(frame2, frame2, cv::COLOR_BGR2GRAY);
		
		//Apply Sobel in the X and Y direction
		cv::Sobel(frame, frame_grad_x, CV_16S, 1, 0, 3); // X-direction
		cv::Sobel(frame, frame_grad_y, CV_16S, 0, 1, 3); // Y-direction
		cv::Sobel(frame2, frame2_grad_x, CV_16S, 1, 0, 3); // X-direction
		cv::Sobel(frame2, frame2_grad_y, CV_16S, 0, 1, 3); // Y-direction

		//Convert gradients to absolute values
		cv::convertScaleAbs(frame_grad_x, frame_abs_x);
		cv::convertScaleAbs(frame_grad_y, frame_abs_y);
		cv::convertScaleAbs(frame2_grad_x, frame2_abs_x);
		cv::convertScaleAbs(frame2_grad_y, frame2_abs_y);

		//Combine gradients to get final edge detection result
		cv::addWeighted(frame_abs_x, 0.5, frame_abs_y, 0.5, 0, frame_edge);
		cv::addWeighted(frame2_abs_x, 0.5, frame2_abs_y, 0.5, 0, frame2_edge);

		//Detect key points and extract descriptors
		orb->detectAndCompute(frame_edge, cv::noArray(), keypoints1, descriptors1);
		orb->detectAndCompute(frame2_edge, cv::noArray(), keypoints2, descriptors2);
	
		// Use brute force matcher with Hamming distance
		matcher.match(descriptors1, descriptors2, matches);
	
		// Draw matches
		cv::drawMatches(frame_edge, keypoints1, frame2_edge, keypoints2, matches, img_matches);
	
		//show matching result
		cv::imshow("Matches", img_matches);

		if (cv::waitKey(1) == 'q') {
			break;
		}
	}
	cap.release();
	cap1.release();

	cv::destroyAllWindows();
	*/
	return 0;
}
