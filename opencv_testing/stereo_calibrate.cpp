#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <regex>

/*
.yaml -----------------VALUES STORED -------------------------

left_camera_matrix
left_dist_coeffs
left_rvecs
left_tvecs
right_camera_matrix
right_dist_coeffs
right_rvecs
right_tvecs

rotation_matrix
translation_matrix
essential_matrix
fundamental_matrix

left_rotation_matrix
right_rotation_matrix
left_projection_matrix
right_projection_matrix
disparity_2_depth_matrix

left_map1
left_map2
right_map1
right_map2
*/


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
    if (argc < 2) {
        std::cout << "Provide filename to store values to (with .yml ending)" << std::endl;
        return 1;
    }

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
    
    int captured_imgs = 0;
    cv::Size image_size;

    std::vector<cv::Mat> left_frames;
    std::vector<cv::Mat> right_frames;
    // Specifies the number of inner corners on the checkerboard
    cv::Size board_size(8, 5); // Width: 9, Height: 6

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
        cv::imwrite("view/left.png", left_frame);
        cv::imwrite("view/right.png", right_frame);

        std::cout << "Checking corners" << std::endl;

        std::vector<cv::Point2f> left_temp;
        std::vector<cv::Point2f> right_temp;

        bool found_left_corners = cv::findChessboardCorners(left_frame, board_size, left_temp, cv::CALIB_CB_ADAPTIVE_THRESH);
        bool found_right_corners = cv::findChessboardCorners(right_frame, board_size, right_temp, cv::CALIB_CB_ADAPTIVE_THRESH);

        if(!found_left_corners || !found_right_corners) {
            std::cerr << "ERROR: Corners not detected in image" << std::endl;
            continue;
        }

        // Save left image
        left_frames.push_back(left_frame);
        right_frames.push_back(right_frame);

        left_frame.release();
        right_frame.release();
        
        std::cout << "Image caputred" << ++captured_imgs << std::endl;
    }

    left.release();
    right.release();

    // Once enough images were captured, we can then perform the stereocalibration

    // Specify the 2D points in the image where the corners are
    std::vector<std::vector<cv::Point2f>> left_image_points;
    std::vector<std::vector<cv::Point2f>> right_image_points;

    // Specifies the 3D points in space where the corners are
    std::vector<std::vector<cv::Point3f>> object_points;

    std::vector<cv::Point3f> board_obj_pts;
    for (int i = 0; i < board_size.height; i++) {
        for (int n = 0; n < board_size.width; n++) {
            board_obj_pts.push_back(cv::Point3f(n, i, 0));
        }
    }

    int corners_found = 0;

    for (int i = 0; i < left_frames.size(); i++) {
        std::cout << "Image @ index " << i << std::endl;
        cv::Mat left_calib = left_frames[i];
        cv::Mat right_calib = right_frames[i];

        if (left_calib.empty() || right_calib.empty()) {
            std::cerr << "ERROR: Failed to read image at index " << i << std::endl;
            continue; 
        } else {
            image_size = left_calib.size();
        }

        std::cout << "Finding image corners" << std::endl;
        std::vector<cv::Point2f> left_corner_points;
        std::vector<cv::Point2f> right_corner_points;
        // cv::CALIB_CB_ADAPTIVE_THRESH just makes it so the black and white image generated is based on an adaptive level
        // calculated by the average image brightness. 
        bool found_left_corners = cv::findChessboardCorners(left_calib, board_size, left_corner_points, cv::CALIB_CB_ADAPTIVE_THRESH);
        bool found_right_corners = cv::findChessboardCorners(right_calib, board_size, right_corner_points, cv::CALIB_CB_ADAPTIVE_THRESH);

        if (found_left_corners && found_right_corners) {
            corners_found++;

            std::cout << "Found corners " << corners_found << std::endl;
            // Draw corners onto image
            cv::drawChessboardCorners(left_calib, board_size, left_corner_points, found_left_corners);
            cv::drawChessboardCorners(right_calib, board_size, right_corner_points, found_right_corners);

            // See images in /test/view/left.jpg
            cv::imwrite("view/left.png", left_calib);
            cv::imwrite("view/right.png", right_calib);

            // Save 2D points
            left_image_points.push_back(left_corner_points);
            right_image_points.push_back(right_corner_points);
            // Save 3D points
            object_points.push_back(board_obj_pts);

        } else {
            std::cout << "Failed to find corners" << std::endl;
        }
    }   
    
    // Open filestorage member
    std::string filename = argv[1];
    cv::FileStorage fs(filename, cv::FileStorage::WRITE);

    std::cout << "Calibrating left camera" << std::endl;
    cv::Mat left_camera_matrix, left_dist_coeffs, left_rvecs, left_tvecs;
    cv::calibrateCamera(object_points, left_image_points, board_size, left_camera_matrix, left_dist_coeffs, left_rvecs, left_tvecs);

    std::cout << "Storing left camera data" << std::endl;
    fs << "left_camera_matrix" << left_camera_matrix;
    fs << "left_dist_coeffs" << left_dist_coeffs;
    fs << "left_rvecs" << left_rvecs;
    fs << "left_tvecs" << left_tvecs;

    std::cout << "Calibrating right camera" << std::endl;
    cv::Mat right_camera_matrix, right_dist_coeffs, right_rvecs, right_tvecs;
    cv::calibrateCamera(object_points, right_image_points, board_size, right_camera_matrix, right_dist_coeffs, right_rvecs, right_tvecs);

    std::cout << "Storing right camera data" << std::endl;
    fs << "right_camera_matrix" << right_camera_matrix;
    fs << "right_dist_coeffs" << right_dist_coeffs;
    fs << "right_rvecs" << right_rvecs;
    fs << "right_tvecs" << right_tvecs;

    std::cout << "Stereocalibration" << std::endl;
    /*  R: Output rotation matrix
        T: Output translation matrix
        E: Output essential matrix
        F: Output fundamental matrix
    */
    cv::Mat R, T, E, F; 
    cv::stereoCalibrate(object_points, left_image_points, right_image_points, left_camera_matrix, left_dist_coeffs, right_camera_matrix, right_dist_coeffs, image_size, R, T, E, F);
    
    std::cout << "Storing stereo data" << std::endl;

    fs << "rotation_matrix" << R;
    fs << "translation_matrix" << T;
    fs << "essential_matrix" << E;
    fs << "fundamental_matrix" << F;

    // First camera is the left camera, second is the right camera
    // NOTE: Might want to get new image size later but unsure if necessary since its getting rescaled anyways
    std::cout << "Stereorectification" << std::endl;
    cv::Mat R1, R2, P1, P2, Q;
    cv::stereoRectify(left_camera_matrix, left_dist_coeffs, right_camera_matrix, right_dist_coeffs, image_size, R, T, R1, R2, P1, P2, Q);
    
    std::cout << "Storing stereorectification" << std::endl;

    fs << "left_rotation_matrix" << R1;
    fs << "right_rotation_matrix" << R2;
    fs << "left_projection_matrix" << P1;
    fs << "right_projection_matrix" << P2;
    fs << "disparity_2_depth_matrix" << Q;

    // m1type options : CV_32FC1, CV_32FC2 or CV_16SC2
    // CV_32FC1 and CV_32FC2 are more precise but slower
    // CV_16SC2 is faster but is fixed point, will be using CV_16SC2
    std::cout << "Init undistort rectify map for left camera?" << std::endl;
    cv::Mat left_map1, left_map2;
    cv::initUndistortRectifyMap(left_camera_matrix, left_dist_coeffs, R1, P1, image_size, CV_16SC2, left_map1, left_map2);

    std::cout << "Storing left camera maps" << std::endl;

    fs << "left_map1" << left_map1;
    fs << "left_map2" << left_map2;

    std::cout << "Init undistort rectify map for right camera?" << std::endl;
    cv::Mat right_map1, right_map2;
    cv::initUndistortRectifyMap(right_camera_matrix, right_dist_coeffs, R2, P2, image_size, CV_16SC2, right_map1, right_map2);

    std::cout << "Storing right camera maps" << std::endl;

    fs << "right_map1" << right_map1;
    fs << "right_map2" << right_map2;

    fs.release();

    return 0;
}

