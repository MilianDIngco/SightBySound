#include <cmath>
#include <gtest/gtest.h>
#include "hilbert.hpp"
#include <memory>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <vector>

using vec2D = std::vector<std::vector<int>>;
using vec1D = std::vector<int>;
using vec1Duchar = std::vector<uchar>;

class HilbertTest : public testing::Test {
  protected:
    std::unique_ptr<Hilbert> order0;
    std::unique_ptr<Hilbert> order1;
    std::unique_ptr<Hilbert> order2;
    std::unique_ptr<Hilbert> order3;

    vec2D vec0 = {
      {0}
    };
    vec2D vec1 = {
      {0, 3},
      {1, 2}
    };
    vec2D vec2 = {
      {0, 1, 14, 15},
      {3, 2, 13, 12},
      {4, 7, 8, 11},
      {5, 6, 9, 10}
    };
    vec2D vec3 = {
      { 0,  3,  4,  5, 58, 59, 60, 63},
      { 1,  2,  7,  6, 57, 56, 61, 62},
      {14, 13,  8,  9, 54, 55, 50, 49},
      {15, 12, 11, 10, 53, 52, 51, 48},
      {16, 17, 30, 31, 32, 33, 46, 47},
      {19, 18, 29, 28, 35, 34, 45, 44},
      {20, 23, 24, 27, 36, 39, 40, 43},
      {21, 22, 25, 26, 37, 38, 41, 42}
    };
    
    vec1D sol0;
    vec1D sol1;
    vec1D sol2;
    vec1D sol3;
    
    void SetUp() override {
      sol0.resize(std::pow(2, 2 * 0));
      sol1.resize(std::pow(2, 2 * 1));
      sol2.resize(std::pow(2, 2 * 2));
      sol3.resize(std::pow(2, 2 * 3));
      for (int i = 0; i < std::pow(2, 2 * 0); i++) {
        this->sol0.at(i) = i;
      }
      for (int i = 0; i < std::pow(2, 2 * 1); i++) {
        this->sol1.at(i) = i;
      }
      for (int i = 0; i < std::pow(2, 2 * 2); i++) {
        this->sol2.at(i) = i;
      }
      for (int i = 0; i < std::pow(2, 2 * 3); i++) {
        this->sol3.at(i) = i;
      }

      Settings s0 = Settings("");
      s0.order = 0;
      Settings s1 = Settings("");
      s1.order = 1;
      Settings s2 = Settings("");
      s2.order = 2;
      Settings s3 = Settings("");
      s3.order = 3;
      this->order0 = std::make_unique<Hilbert>(s0);
      this->order1 = std::make_unique<Hilbert>(s1);
      this->order2 = std::make_unique<Hilbert>(s2);
      this->order3 = std::make_unique<Hilbert>(s3);

    }

};

TEST_F(HilbertTest, toHilbert2D) {
  vec1D res0 = order0->toHilbert(vec0);
  EXPECT_EQ(res0.size(), std::pow(2, 2 * 0));
  vec1D res1 = order1->toHilbert(vec1);
  EXPECT_EQ(res1.size(), std::pow(2, 2 * 1));
  vec1D res2 = order2->toHilbert(vec2);
  EXPECT_EQ(res2.size(), std::pow(2, 2 * 2));
  vec1D res3 = order3->toHilbert(vec3);
  EXPECT_EQ(res3.size(), std::pow(2, 2 * 3));

  for (int i = 0; i < res0.size(); i++) {
    EXPECT_EQ(res0[i], sol0[i]);
  }
  for (int i = 0; i < res1.size(); i++) {
    EXPECT_EQ(res1[i], sol1[i]);
  }
  for (int i = 0; i < res2.size(); i++) {
    EXPECT_EQ(res2[i], sol2[i]);
  }
  for (int i = 0; i < res3.size(); i++) {
    EXPECT_EQ(res3[i], sol3[i]);
  }
  
}

TEST_F(HilbertTest, toHilbert1D) {
  // Turn arrays into 1D arrays to test
  vec1D vec01D(std::pow(2, 2 * 0));
  vec1D vec11D(std::pow(2, 2 * 1));
  vec1D vec21D(std::pow(2, 2 * 2));
  vec1D vec31D(std::pow(2, 2 * 3));

  for (int i = 0; i < vec0.size(); i++) {
    for (int n = 0; n < vec0.at(i).size(); n++) {
      vec01D.at(i * vec0.size() + n) = vec0.at(i).at(n);
    }
  }
  for (int i = 0; i < vec1.size(); i++) {
    for (int n = 0; n < vec1.at(i).size(); n++) {
      vec11D.at(i * vec1.size() + n) = vec1.at(i).at(n);
    }
  }
  for (int i = 0; i < vec2.size(); i++) {
    for (int n = 0; n < vec2.at(i).size(); n++) {
      vec21D.at(i * vec2.size() + n) = vec2.at(i).at(n);
    }
  }
  for (int i = 0; i < vec3.size(); i++) {
    for (int n = 0; n < vec3.at(i).size(); n++) {
      vec31D.at(i * vec3.size() + n) = vec3.at(i).at(n);
    }
  }

  // Test results
  vec1D res0 = order0->toHilbert(vec01D);
  EXPECT_EQ(res0.size(), std::pow(2, 2 * 0));
  vec1D res1 = order1->toHilbert(vec11D);
  EXPECT_EQ(res1.size(), std::pow(2, 2 * 1));
  vec1D res2 = order2->toHilbert(vec21D);
  EXPECT_EQ(res2.size(), std::pow(2, 2 * 2));
  vec1D res3 = order3->toHilbert(vec31D);
  EXPECT_EQ(res3.size(), std::pow(2, 2 * 3));

  for (int i = 0; i < res0.size(); i++) {
    EXPECT_EQ(res0[i], sol0[i]);
  }
  for (int i = 0; i < res1.size(); i++) {
    EXPECT_EQ(res1[i], sol1[i]);
  }
  for (int i = 0; i < res2.size(); i++) {
    EXPECT_EQ(res2[i], sol2[i]);
  }
  for (int i = 0; i < res3.size(); i++) {
    EXPECT_EQ(res3[i], sol3[i]);
  }
}

TEST_F(HilbertTest, toHilbertMat) {
  auto toMat = [](vec2D vec) {
    int rows = vec.size();
    int cols = vec.at(0).size();
    cv::Mat res(rows, cols, CV_8UC1);
    for (int i = 0; i < res.rows; i++) 
      for (int n = 0; n < res.cols; n++)
        res.at<uchar>(i, n) = static_cast<uchar>(vec.at(i).at(n));

    return res;
  };

  cv::Mat vec0mat = toMat(vec0);
  cv::Mat vec1mat = toMat(vec1);
  cv::Mat vec2mat = toMat(vec2);
  cv::Mat vec3mat = toMat(vec3);

  // Go from mat to 1d vector
  vec1Duchar vec01D;
  vec1Duchar vec11D;
  vec1Duchar vec21D;
  vec1Duchar vec31D;
  vec01D.assign(vec0mat.begin<uchar>(), vec0mat.end<uchar>());
  vec11D.assign(vec1mat.begin<uchar>(), vec1mat.end<uchar>());
  vec21D.assign(vec2mat.begin<uchar>(), vec2mat.end<uchar>());
  vec31D.assign(vec3mat.begin<uchar>(), vec3mat.end<uchar>());

  // Test results
  vec1Duchar res0 = order0->toHilbert(vec01D);
  EXPECT_EQ(res0.size(), std::pow(2, 2 * 0));
  vec1Duchar res1 = order1->toHilbert(vec11D);
  EXPECT_EQ(res1.size(), std::pow(2, 2 * 1));
  vec1Duchar res2 = order2->toHilbert(vec21D);
  EXPECT_EQ(res2.size(), std::pow(2, 2 * 2));
  vec1Duchar res3 = order3->toHilbert(vec31D);
  EXPECT_EQ(res3.size(), std::pow(2, 2 * 3));

  for (int i = 0; i < res0.size(); i++) {
    EXPECT_EQ(static_cast<int>(res0[i]), sol0[i]);
  }
  for (int i = 0; i < res1.size(); i++) {
    EXPECT_EQ(static_cast<int>(res1[i]), sol1[i]);
  }
  for (int i = 0; i < res2.size(); i++) {
    EXPECT_EQ(static_cast<int>(res2[i]), sol2[i]);
  }
  for (int i = 0; i < res3.size(); i++) {
    EXPECT_EQ(static_cast<int>(res3[i]), sol3[i]);
  }

}
