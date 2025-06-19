#ifndef CAMERADEPTH_H
#define CAMERADEPTH_H

#include "settings.hpp"
#include <opencv4/opencv2/opencv.hpp>

struct CalibrationMaps {
  cv::Mat left_map1;
  cv::Mat left_map2;
  cv::Mat right_map1;
  cv::Mat right_map2;

  CalibrationMaps() = default;
  CalibrationMaps(cv::Mat l1, cv::Mat l2, cv::Mat r1, cv::Mat r2) : left_map1(l1), left_map2(l2), right_map1(r1), right_map2(r2) {}
};

class CameraDepth {
  private:
    std::string left_path;
    std::string right_path;
    cv::VideoCapture left_cam;
    cv::VideoCapture right_cam;
    CalibrationMaps maps;
    cv::Ptr<cv::StereoBM> stereo;
    int left_bound;
    int right_bound;
    int upper_bound;
    int lower_bound;
    int num_disparities;
    bool use_internearest;
    bool use_interarea;
  
    int getCameraIndex(const std::string &cam_path);
    bool openCamera(cv::VideoCapture &cap, std::string path);
    bool resetCamera(cv::VideoCapture cam, const std::string path);

  public: 
    cv::Size prestereo_scale;
    cv::Size hilbert_scale;

    CameraDepth(std::string calibration_path, std::string left_path, std::string right_path, float prestereo_scale, int block_size, int num_disparities, int pre_filter_cap, int min_disparity, int texture_threshold, int uniqueness_ratio, int speckle_window_size, int speckle_range, int disp12maxdiff, int order, bool use_internearest, bool use_interarea); 
    CameraDepth(Settings settings) : CameraDepth(settings.camera_calibration_path, settings.left_camera_path, settings.right_camera_path, settings.prestereo_scale, settings.block_size, settings.num_disparities, settings.prefilter_cap, settings.min_disparity, settings.texture_threshold, settings.uniqueness_ratio, settings.speckle_window_size, settings.speckle_range, settings.disp12maxdiff, settings.order, settings.use_internearest, settings.use_interarea) {};
    CameraDepth() : CameraDepth("", "", "", -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, true, false) {};

    ~CameraDepth();

    bool setCameras(std::string left_path = "", std::string right_path = "");
    bool captureImages(cv::Mat &left_frame, cv::Mat &right_frame, const bool resetIfFail = false);
    void rectifyImages(cv::Mat &left_frame, cv::Mat &right_frame);
    void scaleImages(cv::Mat &frame, cv::Size size);
    void grayImages(cv::Mat &left_frame, cv::Mat &right_frame);
    cv::Mat getDepthImage(cv::Mat &left_frame, cv::Mat &right_frame);
};

#endif // !CAMERADEPTH_H
