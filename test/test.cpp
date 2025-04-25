#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <regex>
#include "function_timer.h"

int get_camera_index(const std::string& by_path) {
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

int get_left_bound(cv::Mat img) {
  // scan from left to right
  int left_bounds = 0;
  bool found = false;
  for (int i = 0; i < img.size().width && !found; i++) {
    for (int n = 0; n < img.size().height; n++) {
      if (img.at<uchar>(n, i) != 0) {
        left_bounds = i;
        found = true;
        break;
      }
    }
  }

  return left_bounds;
}

int get_right_bound(cv::Mat img) {
  // scan from left to right
  int right_bounds = 0;
  bool found = false;
  for (int i = img.size().width - 1; i > 0 && !found; i--) {
    for (int n = img.size().height - 1; n > 0; n--) {
      if (img.at<uchar>(n, i) != 0) {
        right_bounds = i;
        found = true;
        break;
      }
    }
  }

  return right_bounds;
}

int get_top_bound(cv::Mat img) {
  // scan from left to right
  int top_bounds = 0;
  bool found = false;
  for (int i = 0; i < img.size().height && !found; i++) {
    for (int n = 0; n < img.size().width; n++) {
      if (img.at<uchar>(i, n) != 0) {
        top_bounds = i;
        found = true;
        break;
      }
    }
  }

  return top_bounds;
}

int get_bot_bound(cv::Mat img) {
  // scan from left to right
  int bot_bounds = 0;
  bool found = false;
  for (int i = img.size().height - 1; i > 0 && !found; i--) {
    for (int n = img.size().width - 1; n > 0; n--) {
      if (img.at<uchar>(i, n) != 0) {
        bot_bounds = i;
        found = true;
        break;
      }
    }
  }

  return bot_bounds;
}

bool capture_image(cv::VideoCapture& cap, cv::Mat& frame, int max_attempts = 3) {
  for (int i = 0; i < max_attempts; i++) {
    if (cap.read(frame)) {
      return true;
    }
  }

  return false;
}

bool open_camera(cv::VideoCapture& cap, std::string path) {
  int camera_index = get_camera_index(path);
  if (!cap.open(camera_index, cv::CAP_V4L2)) {
    std::cerr << "ERROR: Failed to open camera at path " << path << std::endl;
    return false;
  }
  cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
  return true;
}

int main(int argc, char** argv) {
  if (argc < 5) {
    std::cerr << "Open which camera data .yml file you want to open, numDisparities, and blockSize, and numRuns, and image_width" << std::endl;
    return 1;
  }

  std::cout << "Scaling down to image size of " << std::stoi(argv[5]) << std::endl;
  int image_width = std::stoi(argv[5]);

  // Open cameras
  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::string right_path = " /dev/v4l/by-path/platform-xhci-hcd.1-usb-0:1:1.0-video-index0";
  std::string left_path = "/dev/v4l/by-path/platform-xhci-hcd.0-usb-0:1:1.0-video-index0";
  
  cv::VideoCapture left;
  std::cout << "Opening left camera" << std::endl;
  if (!open_camera(left, left_path))
    return 1;

  cv::VideoCapture right;
  std::cout << "Opening right camera" << std::endl;
  if (!open_camera(right, right_path))
    return 1;

  // Open filestorage 
  std::string filename = argv[1];
  std::cout << "Opening filestorage at " << filename << std::endl;
  cv::FileStorage fs(filename, cv::FileStorage::READ);

  // Get variablles needed for remap()
  // source img, destination img, map1, map2
  std::cout << "Grabbing variables" << std::endl;
  cv::Mat left_map1, left_map2, right_map1, right_map2;
  fs["left_map1"] >> left_map1;
  fs["left_map2"] >> left_map2;
  fs["right_map1"] >> right_map1;
  fs["right_map2"] >> right_map2;

  // Capture images
  std::cout << "Capturing images" << std::endl;
  cv::Mat right_frame, right_rectified;
  cv::Mat left_frame, left_rectified;

  // SET STEREOBM PARAMS
  int numDisparities = std::stoi(argv[2]);
  int blockSize = std::stoi(argv[3]);
  cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create();
  stereo->setBlockSize(blockSize);         // 9 to 21 (must be odd)
  stereo->setNumDisparities(numDisparities);       // must be divisible by 16
  stereo->setPreFilterCap(31);
  stereo->setMinDisparity(0);
  stereo->setTextureThreshold(5);
  stereo->setUniquenessRatio(0);
  stereo->setSpeckleWindowSize(50);
  stereo->setSpeckleRange(32);
  stereo->setDisp12MaxDiff(1);

  // int min_width = 640;
  // int max_width = 0;
  // int min_height = 480;
  // int max_height = 0;

  for(int i = 0; i < std::stoi(argv[4]); i++) {
    std::cout << "Capturing frame" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // TRY TO CAPTURE IMAGE
    if (!capture_image(left, left_frame) || !capture_image(right, right_frame)) {
        std::cerr << "Frame read timed out or failed!" << std::endl;

        std::cerr << "Trying to reopen cameras" << std::endl;
        left.release();
        if (!open_camera(left, left_path)) {
          std::cerr << "Failed to reopen left camera" << std::endl;
          return -1;
        }

        right.release();
        if (!open_camera(right, right_path)) {
          std::cerr << "Failed to reopen right camera" << std::endl;
          return -1;
        }

        continue;
    }

    if (left_frame.empty() || right_frame.empty()) {
      std::cout << "Empty frame, skipping" << std::endl;
      continue;
    }

    if (left_frame.size() != right_frame.size()) {
      std::cout << "Frame size mismatch" << std::endl;
      continue;
    }

    std::cout << "Rectifying images" << std::endl;
    cv::remap(left_frame, left_rectified, left_map1, left_map2, cv::INTER_LINEAR);
    cv::remap(right_frame, right_rectified, right_map1, right_map2, cv::INTER_LINEAR);

    // See images in /test/view/rectified/ ___.jpg
    std::cout << "Storing images" << std::endl;
    //cv::imwrite("view/rectified/left_og.jpg", left_frame);
    cv::imwrite("view/rectified/left_rect.jpg", left_rectified);

    std::cout << "Converting images to grayscale" << std::endl;
    cv::cvtColor(left_rectified, left_rectified, cv::COLOR_BGR2GRAY);
    cv::cvtColor(right_rectified, right_rectified, cv::COLOR_BGR2GRAY);

    std::cout << "Stereo Block Matching" << std::endl;
    cv::Mat disparity;
    stereo->compute(left_rectified, right_rectified, disparity);

    std::cout << "Normalize to depth map" << std::endl;
    cv::Mat depth;
    disparity.convertTo(depth, CV_8U, 255.0 / (numDisparities * 16));

    // std::cout << "Cropping depth map" << std::endl;
    // std::cout << "Height: " << depth.size().height << ", Width: " << depth.size().width << std::endl;
    // int left = get_left_bound(depth);
    // int right = get_right_bound(depth);
    // int top = get_top_bound(depth);
    // int bot = get_bot_bound(depth);

    // min_width = (left < min_width) ? left : min_width;
    // max_width = (right > max_width) ? right : max_width;
    // min_height = (top < min_height) ? top : min_height;
    // max_height = (bot > max_height) ? bot : max_height;

    // std::cout << "Left: " << left << ", Right: " << right << std::endl;
    // std::cout << "Top: " << top << ", Bottom: " << bot << std::endl;
    depth = depth(cv::Range(7, 472), cv::Range(102, 632));

    std::cout << "Saving depth map" << std::endl;
    cv::imwrite("view/stereoBM/depth.jpg", depth);

    std::cout << "Scaling down depth map" << std::endl;
    cv::Size scaled_size(image_width, image_width);
    cv::resize(depth, depth, scaled_size, 0, 0, cv::INTER_NEAREST);

    std::cout << "Saving scaled depth map" << std::endl;
    //cv::imwrite("view/stereoBM/scaled_depth.jpg", depth);

    std::cout << "Finished " << i << std::endl;
  }

  // std::cout << "Width bounds: (" << min_width << ", " << max_width << ")" << std::endl;
  // std::cout << "Height bounds: (" << min_height << ", " << max_height << ")" << std::endl;

  left.release();
  right.release();

  std::cout << "Program done" << std::endl;
	return 0;
}
