#include "../include/cameradepth.hpp"
#include "opencv2/core/persistence.hpp"
#include "opencv2/core/types.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include <regex>

CameraDepth::CameraDepth(std::string calibration_path, std::string left_path, std::string right_path, float prestereo_scale, int block_size, int num_disparities, int pre_filter_cap, int min_disparity, int texture_threshold, int uniqueness_ratio, int speckle_window_size, int speckle_range, int disp12maxdiff, int order, bool use_internearest, bool use_interarea) {
  // Initialize Calibration maps
  cv::FileStorage fs(calibration_path, cv::FileStorage::READ);
  if (!fs.isOpened()) {
    std::cerr << "ERROR: Failed to open calibration file at path " << calibration_path << std::endl;
    return;
  }
  cv::Mat left_map1, left_map2, right_map1, right_map2;
  fs["left_map1"] >> left_map1;
  fs["left_map2"] >> left_map2;
  fs["right_map1"] >> right_map1;
  fs["right_map2"] >> right_map2;
  this->maps = CalibrationMaps(left_map1, left_map2, right_map1, right_map2);

  // Initialize StereoBM ptr
  this->stereo = cv::StereoBM::create();
  stereo->setBlockSize(block_size);
  stereo->setNumDisparities(num_disparities); // must be divisible by 16
  stereo->setPreFilterCap(pre_filter_cap);
  stereo->setMinDisparity(min_disparity);
  stereo->setTextureThreshold(texture_threshold);
  stereo->setUniquenessRatio(uniqueness_ratio);
  stereo->setSpeckleWindowSize(speckle_window_size);
  stereo->setSpeckleRange(speckle_range);
  stereo->setDisp12MaxDiff(disp12maxdiff);

  this->num_disparities = num_disparities;
  this->use_internearest = use_internearest;
  this->use_interarea = use_interarea;

  // Initialize Cameras
  this->left_path = left_path;
  this->right_path = right_path;
  if (!this->setCameras()) {
    std::cerr << "ERROR: Failed to set cameras at path " << left_path << " & " << right_path << std::endl;
    return;
  }

  // Set prestereo scale size
  cv::Mat test;
  this->left_cam >> test;
  int image_width = test.cols * prestereo_scale;
  int image_height = test.rows * prestereo_scale;
  this->prestereo_scale = cv::Size(image_width, image_height);
  int hilbert_width = std::pow(2, order);
  this->hilbert_scale = cv::Size(hilbert_width, hilbert_width);

  // Set cropping bounds
  this->left_bound = (int) 7 * prestereo_scale;
  this->right_bound = (int) 472 * prestereo_scale;
  this->upper_bound = (int) 102 * prestereo_scale;
  this->lower_bound = (int) 632 * prestereo_scale;
}

CameraDepth::~CameraDepth() {
  this->left_cam.release();
  this->right_cam.release();
}

bool CameraDepth::captureImages(cv::Mat &left_frame, cv::Mat &right_frame, const bool resetIfFail) {
  if (!this->left_cam.read(left_frame)) {
    std::cerr << "ERROR: Failed to capture image from left camera" << std::endl;
    if (resetIfFail)
      return this->resetCamera(this->left_cam, this->left_path);
    return false;
  }
  if (!this->right_cam.read(right_frame)) {
    std::cerr << "ERROR: Failed to capture image from right camera" << std::endl;
    if (resetIfFail)
      return this->resetCamera(this->right_cam, this->right_path);
    return false;
  }

  return true;
}

void CameraDepth::rectifyImages(cv::Mat &left_frame, cv::Mat &right_frame) {
  cv::remap(left_frame, left_frame, maps.left_map1, maps.left_map2,
              cv::INTER_LINEAR);
  cv::remap(right_frame, right_frame, maps.right_map1, maps.right_map2,
              cv::INTER_LINEAR);
}

void CameraDepth::scaleImages(cv::Mat &frame, cv::Size size) {
  if (this->use_internearest)
    cv::resize(frame, frame, size, 0, 0, cv::INTER_NEAREST);
  else
    cv::resize(frame, frame, size, 0, 0, cv::INTER_AREA);
}

void CameraDepth::grayImages(cv::Mat &left_frame, cv::Mat &right_frame) {
  cv::cvtColor(left_frame, left_frame, cv::COLOR_BGR2GRAY);
  cv::cvtColor(right_frame, right_frame, cv::COLOR_BGR2GRAY);
}

cv::Mat CameraDepth::getDepthImage(cv::Mat &left_frame, cv::Mat &right_frame) {
  cv::Mat disparity;
  // Perform stereo block matching
  this->stereo->compute(left_frame, right_frame, disparity);

  cv::Mat depth;
  disparity.convertTo(depth, CV_8U, 255.0 / (this->num_disparities * 16));

  // Crop image
  depth = depth(cv::Range(this->left_bound, this->right_bound), cv::Range(this->upper_bound, this->lower_bound));

  return depth;
}

bool CameraDepth::resetCamera(cv::VideoCapture cam, const std::string path) {
  cam.release();

  // Try to reopen at the path
  if (!this->openCamera(cam, path)) {
    std::cerr << "ERROR: Failed to reset camera" << std::endl;
    return false;
  }
  return true;
}

bool CameraDepth::setCameras(std::string left_path, std::string right_path) {
  if (left_path == "") {
    left_path = this->left_path;
  }
  if (right_path == "") {
    right_path = this->right_path;
  }

  if (!this->openCamera(this->left_cam, left_path)) {
    std::cerr << "ERROR: Failed to open left camera at path " << left_path << std::endl;
    return false;
  }

  if (!this->openCamera(this->right_cam, right_path)) {
    std::cerr << "ERROR: Failed to open right camera at path " << right_path << std::endl;
    return false;
  }

  return true;
}

bool CameraDepth::openCamera(cv::VideoCapture &cap, const std::string path) {
  int camera_index = getCameraIndex(path);
  if (!cap.open(camera_index, cv::CAP_V4L2)) {
    return false;
  }

  cv::Mat mat;
  // Release camera if failed to capture an image
  if(!cap.read(mat)) {
    cap.release();
    return false;
  }

  cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
  return true;
}

int CameraDepth::getCameraIndex(const std::string &cam_path) {
  std::string readlink = "readlink -f " + cam_path;
  char buffer[128];
  std::string result;

  // Run readlink command in subprocess
  FILE *pipe = popen(readlink.c_str(), "r");
  if (!pipe)
    return -1;
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result += buffer;
  }

  pclose(pipe);

  result.erase(result.find_last_not_of(" \n\r\t") + 1);

  // Get the last number from the string
  std::regex numRegex(R"(\d+$)"); // Matches digits at the end of the string
  std::smatch match;
  if (std::regex_search(result, match, numRegex)) {
    return std::stoi(match.str()); // Convert matched string to int
  }

  return -1; // Return -1 if no number is found
}
