#include <opencv2/opencv.hpp> // OpenCV header
#include <iostream>

int main() {
	cv::VideoCapture cap(0);
	cv::VideoCapture cap1(2);

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

		// Display frame in a window
		cv::imshow("Webcam Live Feed", frame);
		cv::imshow("Webcam Live Feed2", frame2);

		// Wiat 1 ms and break if 'q' is pressed
		if (cv::waitKey(1) == 'q') {
			break;
		}
	}

	cap.release();
	cap1.release();

	cv::destroyAllWindows();

	return 0;
}
