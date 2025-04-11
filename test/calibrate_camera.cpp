#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 7) {
        std::cerr << "ERROR: Not enough parameters.\n Folder Filename StartingIndex Amount NewFolderName NewFileName" << std::endl;
        return 1;
    }
    // Get file name and starting index
    std::string folder = argv[1];
    std::string filename = argv[2];
    int index = std::stoi(argv[3]);
    int file_count = std::stoi(argv[4]);

    std::string filetype = ".jpg";

    // New filepath
    std::string new_folder = argv[5];
    std::string new_filename = argv[6];
    int new_count = 1;

    // Specifies the number of inner corners on the checkerboard
    cv::Size board_size(8, 5); // Width: 9, Height: 6

    // Specify the 2D points in the image where the corners are
    std::vector<std::vector<cv::Point2f>> image_points;

    // Specifies the 3D points in space where the corners are
    std::vector<std::vector<cv::Point3f>> object_points;
    
    std::vector<cv::Point3f> board_obj_pts;
    for (int i = 0; i < board_size.height; i++) {
        for (int n = 0; n < board_size.width; n++) {
            board_obj_pts.push_back(cv::Point3f(n, i, 0));
        }
    }

    for (int i = 0; i < file_count; i++) {
        std::string full_path = "./" + folder + "/" + filename + std::to_string(index + i) + filetype;
        std::cout << "Checking file at " << full_path << std::endl;

        cv::Mat image = cv::imread(full_path);
        if (image.empty()) {
            std::cerr << "ERROR: Failed to read image at " << full_path << std::endl;
            continue; 
        }

        std::vector<cv::Point2f> corner_points;
        // cv::CALIB_CB_ADAPTIVE_THRESH just makes it so the black and white image generated is based on an adaptive level
        // calculated by the average image brightness. 
        bool found_corners = cv::findChessboardCorners(image, board_size, corner_points, cv::CALIB_CB_ADAPTIVE_THRESH);

        if (found_corners) {
            std::cout << "Found corners" << std::endl;
            // Draw corners onto image
            cv::drawChessboardCorners(image, board_size, corner_points, found_corners);

            // Save 2D points
            image_points.push_back(corner_points);
            // Save 3D points
            object_points.push_back(board_obj_pts);

            // Save drawn image to new path
            std::string new_filepath = "./" + new_folder + "/" + new_filename + std::to_string(new_count++) + filetype;
            cv::imwrite(new_filepath, image);
            std::cout << "Saving image at " << new_filepath << std::endl;
        } else {
            std::cout << "Failed to find corners" << std::endl;
        }
    }   
    
    cv::Mat camera_matrix, dist_coeffs, rvecs, tvecs;
    cv::calibrateCamera(object_points, image_points, board_size, camera_matrix, dist_coeffs, rvecs, tvecs);

    std::cout << "Camera Matrix:\n" << camera_matrix << "\n";
    std::cout << "Distortion Coefficients:\n" << dist_coeffs << "\n";

    return 0;
}