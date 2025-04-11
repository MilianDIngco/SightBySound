#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <thread>
#include <chrono>
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

  if (argc < 5) {
    std::cerr << "Open which camera data .yml file you want to open, numDisparities, and blockSize, and numRuns" << std::endl;
    return 1;
  }

  // Open cameras
  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::string right_path = "/dev/v4l/by-path/platform-3f980000.usb-usb-0:1.5:1.0-video-index0";
  std::string left_path = "/dev/v4l/by-path/platform-3f980000.usb-usb-0:1.3:1.0-video-index0";
  
  int left_camera_index = get_index(left_path);
  cv::VideoCapture left(left_camera_index);
  if (!left.isOpened()) {
      std::cerr << "ERROR: Failed to open left camera at path " << left_path << std::endl;
      return 1; 
  }
  std::cout << "Opening camera at index " << left_camera_index << std::endl;

  int right_camera_index = get_index(right_path);
  cv::VideoCapture right(right_camera_index);
  if (!right.isOpened()) {
      std::cerr << "ERROR: Failed to open right camera at path " << right_path << std::endl;
      return 1;
  }
  std::cout << "Opening camera at index " << right_camera_index << std::endl;

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

  // cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create(numDisparities, blockSize);
  cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create();
  stereo->setBlockSize(15);            // 9 to 21 (must be odd)
  stereo->setNumDisparities(96);       // must be divisible by 16
  stereo->setPreFilterCap(31);
  stereo->setMinDisparity(0);
  stereo->setTextureThreshold(5);
  stereo->setUniquenessRatio(10);
  stereo->setSpeckleWindowSize(100);
  stereo->setSpeckleRange(32);
  stereo->setDisp12MaxDiff(1);

  for(int i = 0; i < std::stoi(argv[4]); i++) {
    left >> left_frame;
    right >> right_frame;

    std::cout << "Rectifying images" << std::endl;
    cv::remap(left_frame, left_rectified, left_map1, left_map2, cv::INTER_LINEAR);
    cv::remap(right_frame, right_rectified, right_map1, right_map2, cv::INTER_LINEAR);

    // See images in /test/view/rectified/ ___.jpg
    std::cout << "Storing images" << std::endl;
    // cv::imwrite("view/rectified/left_og.jpg", left_frame);
    cv::imwrite("view/rectified/left_rect.jpg", left_rectified);
    // cv::imwrite("view/rectified/right_og.jpg", right_frame);
    // cv::imwrite("view/rectified/right_rect.jpg", right_rectified);

    std::cout << "Converting images to grayscale" << std::endl;
    cv::cvtColor(left_rectified, left_rectified, cv::COLOR_BGR2GRAY);
    cv::cvtColor(right_rectified, right_rectified, cv::COLOR_BGR2GRAY);

    std::cout << "Stereo Block Matching" << std::endl;
    cv::Mat disparity;
    int numDisparities = std::stoi(argv[2]);
    int blockSize = std::stoi(argv[3]);
    stereo->compute(left_rectified, right_rectified, disparity);

    std::cout << "Storing disparity" << std::endl;
    // cv::imwrite("view/stereoBM/disparity.jpg", disparity);

    std::cout << "Normalize to depth map" << std::endl;
    cv::Mat depth;
    disparity.convertTo(depth, CV_8U, 255.0 / (numDisparities * 16));

    std::cout << "Saving depth map" << std::endl;
    cv::imwrite("view/stereoBM/depth.jpg", depth);
  }

  

  left.release();
  right.release();

  std::cout << "Program done" << std::endl;
	return 0;
}
	// ============== OPEN CAMERAS =================
  // const int MAX_CAMERAS = 20;
  // cv::VideoCapture cap;
	// int index = 0;
  //   while (index < MAX_CAMERAS && !cap.isOpened()) {
  //       cap.open(index++);
  //   }

	// if (!cap.isOpened()) {
	// 	std::cerr << "Error: Could not open webcam" << std::endl;
	// 	return -1;
	// } else {
	// 	std::cout << "Opened camera index " << index << std::endl;
	// }

	// cv::VideoCapture cap2;
	// while(index < MAX_CAMERAS && !cap2.isOpened()) {
	// 	cap2.open(index++);
	// }

	// if (!cap2.isOpened()) {
	// 	std::cerr << "Error: Could not open second webcam" << std::endl;
	// 	return -1;
	// } else {
	// 	std::cout << "Opened camera index " << index << std::endl;
	// }


  // if (argc < 3) {
  //   std::cerr << "ERROR: Enter camera indices for the left and right cameras." << std::endl;
  //   return 1;
  // }
  
  // int left_index = std::stoi(argv[1]);
  // cv::VideoCapture left(left_index);  

  // if (!left.isOpened()) {
  //   std::cerr << "ERROR: Left camera failed to open at index " << left_index << std::endl;
  //   return 1;
  // }

  // int right_index = std::stoi(argv[2]);
	// cv::VideoCapture right(right_index);

  // if (!right.isOpened()) {
  //   std::cerr << "ERROR: Right camera failed to open at index " << right_index << std::endl;
  //   return 1;
  // }

	// cv::Mat frame_l;
	// cv::Mat frame_r;

	// left >> frame_l;
	// right >> frame_r;

  // // bool left_found = findChessboardCorners( frame_l, boardSize, ptvec, CALIB_CB_ADAPTIVE_THRESH );
	
	// cv::cvtColor(frame_l, frame_l, cv::COLOR_BGR2GRAY);
	// cv::cvtColor(frame_r, frame_r, cv::COLOR_BGR2GRAY);
	
	// cv::imwrite("out.png", frame_r);

	// cap.release();

	// cv::destroyAllWindows();



/*
    cv::VideoCapture cap(CAM_1_INDEX);
    cv::VideoCapture cap1(CAM_2_INDEX);
  
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

    cap >> frame;
    cap1 >> frame2;
    
    cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frame2, frame2, cv::COLOR_BGR2GRAY);

    cv::imwrite("left.png", frame);
    cv::imwrite("right.png", frame2);
  
    cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create();
    cv::Mat disp, disparity; 
    stereo->compute(frame, frame2, disp);
  
    disp.convertTo(disparity, CV_32F, 1.0);
    disparity = (disparity / 16.0f - (float) minDisparity) / (float) numDisparity;
    // std::cout << disparity << std::endl;
    
    double min, max;
    cv::minMaxLoc(disparity, &min, &max);
    std::cout << "Min: " << min << " Max: " << max << std::endl;

    disparity = 256 * (disparity - min) / (max - min);

    cv::minMaxLoc(disparity, &min, &max);
    std::cout << "Min: " << min << " Max: " << max << std::endl;

    cv::imwrite("out.png", disparity);
  
    cap.release();
    cap1.release();

    return 0;*/



  