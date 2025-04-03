#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    cv::VideoCapture cam("/dev/video1");  

    if (!cam.isOpened()) {
        std::cerr << "ERROR: Camera failed to open" << std::endl;
        return 1;
    }
    
    int img_count = 0;
    while(true) {
        cv::Mat frame;

        std::cout << "Waiting..." << std::endl;

        std::string key = "";
        std::cin >> key;
        if (key[0] == 'q') {
            std::cout << "quitting program" << std::endl;
            break;
        }

        cam >> frame;
        std::string filename = "./checkerboards/checker_" + std::to_string(img_count++) + ".jpg";
        cv::imwrite("view/left.png", frame);
        std::cout << "IMG CAPTURED @ " << filename << std::endl;
    
    }

    cam.release();
      
}