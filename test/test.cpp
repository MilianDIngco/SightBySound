#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <thread>
#include <chrono>
/*
Left Camera
Camera Matrix:
[867.4390361465698, 0, 317.9791697706767;
 0, 864.0893210385751, 210.3069974974848;
 0, 0, 1]
Distortion Coefficients:
[0.2622121114918689, -1.136492077276323, -0.009012060432466136, -0.008457245113531272, 1.700060607263798]

Right Camera
Camera Matrix:
[869.4844623454302, 0, 347.5857681840643;
 0, 869.7203390785392, 210.5008463947516;
 0, 0, 1]
Distortion Coefficients:
[0.2136811148693865, -0.9768909869007987, -0.009122562696865059, 0.005870254419487973, 1.282784942218179]

*/

int main(int argc, char** argv) {

  std::this_thread::sleep_for(std::chrono::seconds(2));

  std::string right_path = "/dev/v4l/by-path/platform-3f980000.usb-usb-0:1.5:1.0-video-index0";
  std::string left_path = "/dev/v4l/by-path/platform-3f980000.usb-usb-0:1.4:1.0-video-index0";
  cv::VideoCapture left(left_path);
  if (!left.isOpened()) {
    std::cerr << "ERROR: Failed to open left camera at path " << left_path << std::endl;
    return 1; 
  }
  cv::VideoCapture right(right_path);
  if (!right.isOpened()) {
    std::cerr << "ERROR: Failed to open right camera at path " << right_path << std::endl;
    return 1;
  }

  // Camera matrices
  cv::Mat left_camera_matrix = (cv::Mat_<double>(3, 3) << 867.4390361465698, 0, 317.9791697706767,
                                                          0, 864.0893210385751, 210.3069974974848,
                                                          0, 0, 1);

  cv::Mat left_distortion_coeffs = (cv::Mat_<double>(1, 5) << 0.2622121114918689, -1.136492077276323, 
                                                              -0.009012060432466136, -0.008457245113531272, 
                                                              1.700060607263798);

  cv::Mat right_camera_matrix = (cv::Mat_<double>(3, 3) << 869.4844623454302, 0, 347.5857681840643,
                                                          0, 869.7203390785392, 210.5008463947516,
                                                          0, 0, 1);

  cv::Mat right_distortion_coeffs = (cv::Mat_<double>(1, 5) << 0.2136811148693865, -0.9768909869007987,
                                                              -0.009122562696865059, 0.005870254419487973,
                                                              1.282784942218179);


  // Capture images
  cv::Mat right_frame;
  cv::Mat left_frame;

  left >> left_frame;
  right >> right_frame;

  std::cout << "saved pictures" << std::endl; 
  cv::imwrite("left_cam.png", left_frame);
  cv::imwrite("right_cam.png", right_frame);

  // Undistort images
  cv::Mat right_undistorted;
  cv::Mat left_undistorted;

  cv::undistort(left_frame, left_undistorted, left_camera_matrix, left_distortion_coeffs);
  cv::undistort(right_frame, right_undistorted, right_camera_matrix, right_distortion_coeffs);

  std::cout << "saved undistorted" << std::endl;
  cv::imwrite("left_undistorted.png", left_undistorted);
  cv::imwrite("right_undistorted.png", right_undistorted);

  left.release();
  right.release();
  
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



  