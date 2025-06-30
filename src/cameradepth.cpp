#include "cameradepth.hpp"
#include "opencv2/core/persistence.hpp"
#include "opencv2/core/types.hpp"
#include <chrono>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <iostream>
#include <regex>
#include <thread>

CameraDepth::CameraDepth(Settings settings) {
  // Initialize Calibration maps
  cv::FileStorage fs(settings.camera_calibration_path, cv::FileStorage::READ);
  if (!fs.isOpened()) {
    std::cerr << "ERROR: Failed to open calibration file at path " << settings.camera_calibration_path << std::endl;
    return;
  }

  cv::Mat left_map1, left_map2, right_map1, right_map2;
  if (fs["left_map1"].empty()) {
    std::cerr << "ERROR: Failed to open left map 1 from calibration file" << std::endl;
    return;
  } else 
    fs["left_map1"] >> left_map1;

  if (fs["left_map2"].empty()) {
    std::cerr << "ERROR: Failed to open left map 2 from calibration file" << std::endl;
    return;
  } else 
    fs["left_map2"] >> left_map2;
  if (fs["right_map1"].empty()) {
    std::cerr << "ERROR: Failed to open right map 1 from calibration file" << std::endl;
    return;
  } else 
    fs["right_map1"] >> right_map1;
  if (fs["right_map1"].empty()) {
    std::cerr << "ERROR: Failed to open right map 2 from calibration file" << std::endl;
    return;
  } else 
    fs["right_map2"] >> right_map2;

  this->maps = CalibrationMaps(left_map1, left_map2, right_map1, right_map2);

  // Initialize StereoBM ptr
  this->stereo = cv::StereoBM::create();
  stereo->setBlockSize(settings.block_size);
  stereo->setNumDisparities(settings.num_disparities); // must be divisible by 16
  stereo->setPreFilterCap(settings.prefilter_cap);
  stereo->setMinDisparity(settings.min_disparity);
  stereo->setTextureThreshold(settings.texture_threshold);
  stereo->setUniquenessRatio(settings.uniqueness_ratio);
  stereo->setSpeckleWindowSize(settings.speckle_window_size);
  stereo->setSpeckleRange(settings.speckle_range);
  stereo->setDisp12MaxDiff(settings.disp12maxdiff);

  this->num_disparities = settings.num_disparities;
  this->use_internearest = settings.use_internearest;
  this->use_interarea = settings.use_interarea;
  this->n_cam_resets = settings.n_cam_resets;

  // Initialize Cameras
  this->left_path = settings.left_camera_path;
  this->right_path = settings.right_camera_path;
  
  if (!this->setCameras() && (settings.left_camera_path != "test_settings" || settings.right_camera_path != "test_settings")) {
    std::cerr << "ERROR: Failed to set cameras at path " << settings.left_camera_path << " & " << settings.right_camera_path << std::endl;
    return;
  }

  // Set prestereo scale size
  cv::Mat test;
  this->left_cam >> test;
  int image_width = test.cols * settings.prestereo_scale;
  int image_height = test.rows * settings.prestereo_scale;
  this->prestereo_scale = cv::Size(image_width, image_height);
  int hilbert_width = std::pow(2, settings.order);
  this->hilbert_scale = cv::Size(hilbert_width, hilbert_width);

  // Set cropping bounds
  this->left_bound = (settings.left_bound > 0) ? settings.left_bound : 0;
  this->right_bound = (settings.right_bound < image_width) ? settings.right_bound : image_width;
  this->upper_bound = (settings.upper_bound > 0) ? settings.upper_bound : 0;
  this->lower_bound = (settings.lower_bound < image_height) ? settings.lower_bound : image_height;
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
  depth = depth(cv::Range(this->upper_bound, this->lower_bound), cv::Range(this->left_bound, this->right_bound)); 

  return depth;
}

bool CameraDepth::resetCamera(cv::VideoCapture cam, const std::string path) {
  for (int i = 0; i < this->n_cam_resets; i++) {
    std::cout << "Trying to reset camera" << std::endl;
    cam.release();

    // Try to reopen at the path
    if (this->openCamera(cam, path)) {
      return true;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  std::cerr << "ERROR: Failed to reset camera" << std::endl;
  return false;
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
  cap.set(cv::CAP_PROP_BUFFERSIZE, 1);
  cap.set(cv::CAP_PROP_OPEN_TIMEOUT_MSEC, 3000);
  cap.set(cv::CAP_PROP_READ_TIMEOUT_MSEC, 3000);

  int camera_index = getCameraIndex(path);
  cap.open(camera_index, cv::CAP_V4L2);
  if (!cap.isOpened()) {
    cap.open(camera_index, cv::CAP_GSTREAMER); // Fallback to GStreamer
  }
  if (!cap.isOpened()) {
    std::cerr << "ERROR: Failed to open camera" << std::endl;
  }

  cv::Mat mat;
  // Release camera if failed to capture an image
  if(!cap.read(mat)) {
    cap.release();
    return false;
  }

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
