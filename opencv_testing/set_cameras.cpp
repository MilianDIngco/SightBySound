#include <opencv4/opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv) {
    cv::VideoCapture cap(std::stoi(argv[1]));  // Open camera n (you can change this)

    if (!cap.isOpened()) {
        std::cerr << "ERROR: Cannot open camera!" << std::endl;
        return -1;
    }

    // Check if serial number can be accessed
    double serial_number = cap.get(cv::CAP_PROP_SERIAL_NUMBER);

    if (serial_number != 0) {
        std::cout << "Camera Serial Number: " << serial_number << std::endl;
    } else {
        std::cout << "Serial number not available for this camera!" << std::endl;
    }

    return 0;
}