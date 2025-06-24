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
    int n_cam_resets;
  
    int getCameraIndex(const std::string &cam_path);
    bool openCamera(cv::VideoCapture &cap, std::string path);
    bool resetCamera(cv::VideoCapture cam, const std::string path);

  public: 
    cv::Size prestereo_scale;
    cv::Size hilbert_scale;

    CameraDepth(std::string calibration_path, std::string left_path, std::string right_path, float prestereo_scale, int block_size, int num_disparities, int pre_filter_cap, int min_disparity, int texture_threshold, int uniqueness_ratio, int speckle_window_size, int speckle_range, int disp12maxdiff, int order, bool use_internearest, bool use_interarea, int n_cam_resets); 
    CameraDepth(Settings settings) : CameraDepth(settings.camera_calibration_path, settings.left_camera_path, settings.right_camera_path, settings.prestereo_scale, settings.block_size, settings.num_disparities, settings.prefilter_cap, settings.min_disparity, settings.texture_threshold, settings.uniqueness_ratio, settings.speckle_window_size, settings.speckle_range, settings.disp12maxdiff, settings.order, settings.use_internearest, settings.use_interarea, settings.n_cam_resets) {};

    ~CameraDepth();

    bool setCameras(std::string left_path = "", std::string right_path = "");
    bool captureImages(cv::Mat &left_frame, cv::Mat &right_frame, const bool resetIfFail = false);
    void rectifyImages(cv::Mat &left_frame, cv::Mat &right_frame);
    void scaleImages(cv::Mat &frame, cv::Size size);
    void grayImages(cv::Mat &left_frame, cv::Mat &right_frame);
    cv::Mat getDepthImage(cv::Mat &left_frame, cv::Mat &right_frame);

    std::string get_left_path() { return this->left_path; };
    std::string get_right_path() { return this->right_path; };
    cv::VideoCapture get_left_cam() { return this->left_cam; };
    cv::VideoCapture get_right_cam() { return this->right_cam; };
    CalibrationMaps get_maps() { return this->maps; };
    cv::Ptr<cv::StereoBM> get_stereo() { return this->stereo; };
    int get_left_bound() { return this->left_bound; };
    int get_right_bound() { return this->right_bound; };
    int get_upper_bound() { return this->upper_bound; };
    int get_lower_bound() { return this->lower_bound; };
    int get_num_disparities() { return this->num_disparities; };
    bool get_use_internearest() { return this->use_internearest; };
    bool get_use_interarea() { return this->use_interarea; };
    int get_n_cam_resets() { return this->n_cam_resets; };
};

#endif // !CAMERADEPTH_H
