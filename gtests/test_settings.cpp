#include <gtest/gtest.h>
#include <memory>
#include "audiomanager.hpp"
#include "cameradepth.hpp"
#include "hilbert.hpp"
#include "settings.hpp"

class SettingsTest : public testing::Test {
  protected:
    std::unique_ptr<Settings> settings;
    void SetUp() override {
      std::string settings_path = "/home/milianingco/Research/SBS/gtests/test_settings.txt";
      this->settings = std::make_unique<Settings>(settings_path);
    }

    void TearDown() override {
      settings.reset();
    }
};

TEST_F(SettingsTest, audiomanager_setup) {
  AudioManager am(*settings);
  ASSERT_EQ(am.get_sample_rate(), 22000);
  ASSERT_EQ(am.get_duration(), 0.1f);
  // period
  ASSERT_EQ(am.get_min_freq(), 30);
  ASSERT_EQ(am.get_max_freq(), 200);
  ASSERT_EQ(am.get_fade_percent(), 0.1);
  ASSERT_EQ(am.get_volume(), 1);
  ASSERT_EQ(am.get_frequencies().size(), 64);
  ASSERT_EQ(am.get_audio_threshold(), 0.8);
  ASSERT_EQ(am.get_audio_soften(), 5);
  ASSERT_EQ(am.get_audio_max(), 0.9);
}

TEST_F(SettingsTest, cameradepth_setup) {
  CameraDepth cd(*this->settings);
  
  ASSERT_EQ(cd.get_right_path(), "test_settings");
  ASSERT_EQ(cd.get_left_path(), "test_settings");
  ASSERT_EQ(cd.get_n_cam_resets(), 5);
  ASSERT_EQ(cd.get_stereo()->getNumDisparities(), 64);
  ASSERT_EQ(cd.get_num_disparities(), 64);
  ASSERT_EQ(cd.get_use_internearest(), true);
  ASSERT_EQ(cd.get_use_interarea(), false);
}

TEST_F(SettingsTest, hilbert_setup) {
  Hilbert hilbert(*settings);
  
  ASSERT_EQ(hilbert.get_order(), 3);
  ASSERT_EQ(hilbert.get_n_points(), 64);
}
