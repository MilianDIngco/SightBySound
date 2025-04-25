#include <opencv4/opencv2/opencv.hpp> // OpenCV header
#include <opencv4/opencv2/features2d.hpp>
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "ERROR: Enter camera indice and which camera it is." << std::endl;
        return 1;
    }
    
    int index = std::stoi(argv[1]);
    cv::VideoCapture cam(index);  

    if (!cam.isOpened()) {
        std::cerr << "ERROR: Camera failed to open at index " << index << std::endl;
        return 1;
    }

    // Which camera it is
    std::string which = argv[2];
    
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
        std::string filename = "./checkerboards/" + which + "_checker_" + std::to_string(img_count++) + ".jpg";
        cv::imwrite(filename, frame);
        std::cout << "IMG CAPTURED @ " << filename << std::endl;
    
    }

    cam.release();
      
}