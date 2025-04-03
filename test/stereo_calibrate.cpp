#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <regex>

int get_index(const std::string& by_path) {
    std::string readlink = "readlink -f " + by_path;
    char buffer[128];
    std::string result;

    // Run readlink command in subprocess
    FILE* pipe = popen(readlink.c_str(), "r");
    if (!pipe) return -1;
    while(fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);

    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    // Get the last number from the string
    std::regex numRegex(R"(\d+$)");  // Matches digits at the end of the string
    std::smatch match;
    if (std::regex_search(result, match, numRegex)) {
        return std::stoi(match.str());  // Convert matched string to int
    }
    return -1;  // Return -1 if no number is found
}

int main(int argc, char** argv) {
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::string right_path = "/dev/v4l/by-path/platform-3f980000.usb-usb-0:1.5:1.0-video-index0";
    std::string left_path = "/dev/v4l/by-path/platform-3f980000.usb-usb-0:1.3:1.0-video-index0";
    
    cv::VideoCapture left(get_index(left_path));
    if (!left.isOpened()) {
        std::cerr << "ERROR: Failed to open left camera at path " << left_path << std::endl;
        return 1; 
    }
    cv::VideoCapture right(get_index(right_path));
    if (!right.isOpened()) {
        std::cerr << "ERROR: Failed to open right camera at path " << right_path << std::endl;
        return 1;
    }
    
    int img_count = 0;

    std::vector<cv::Mat> left_frames;
    std::vector<cv::Mat> right_frames;

    while(true) {
        cv::Mat left_frame;
        cv::Mat right_frame;
        std::cout << "Waiting..." << std::endl;

        std::string key = "";
        std::cin >> key;
        if (key[0] == 'q') {
            std::cout << "quitting program" << std::endl;
            break;
        }

        left >> left_frame;
        right >> right_frame;
        if(left_frame.empty() || right_frame.empty()) {
            std::cerr << "ERROR: Failed to capture image" << std::endl;
            continue;
        }

        // See images in /test/view/left.jpg
        std::string left_filename = "view/left" + std::to_string(img_count) + ".png";
        std::string right_filename = "view/right" + std::to_string(img_count++) + ".png";
        cv::imwrite(left_filename, left_frame);
        cv::imwrite(right_filename, right_frame);

        // Save left image
        // left_frames.push_back(left_frame);
        // right_frames.push_back(right_frame);

        left_frame.release();
        right_frame.release();
        
        std::cout << "Image caputred" << std::endl;
    }

    // Once enough images were captured, we can then perform the stereocalibration



    left.release();
    right.release();
    
    return 0;
}