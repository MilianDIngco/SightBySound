#include "../include/hilbert.hpp"
#include "opencv2/core/mat.hpp"
#include <cmath>
#include <string>
#include <sstream>
#include <iostream>

struct Pair {
  int x;
  int y;

  Pair() : x(0), y(0) {}
  Pair(int x, int y) : x(x), y(y) {}
  Pair(const Pair &p) : x(p.x), y(p.y) {}

  std::string toString() {
    std::stringstream res;
    res << "(" << x << ", " << y << ")";
    return res.str();
  }
};

Hilbert::Hilbert(int order) {
  this->order = order;
  hilbert.resize(std::pow(2, 2 * order));
  this->generateHilbert();
  this->n_points = this->getNPoints(order);
}

int Hilbert::getNPoints(int order) {
  return std::pow(2, 2 * order);
}

Pair Hilbert::getCoord(int index) {
  // bounds checking; Number of points is ( 2 ^ order ) ^ 2 since it's a square
  // array for now
  if (index >= std::pow(4, order))
    return Pair(-1, -1);

  Pair order1[] = {Pair(0, 0), Pair(0, 1), Pair(1, 1), Pair(1, 0)};

  // Depth of recursion is mapped by (log base 4) + 1
  // Index < 4 : order 1
  // Index < 16 : order 2 ...
  int depth = order;

  int quadrant = index & 3;
  Pair coord(order1[quadrant]);
  // Start at order 1
  for (int i = 1; i < depth; i++) {
    // Gets if in quadrant 0, 1, 2, or 3
    index >>= 2;
    int nextq = index & 3;

    // First quadrant ? Reflect across y = x
    // Multiply by matrix
    // [ 0 1 ]
    // [ 1 0 ]
    // Equivalent to swapping x and y
    if (nextq == 0) {
      int temp = coord.y;
      coord.y = coord.x;
      coord.x = temp;
      // Fourth quadrant ?
    } else if (nextq == 3) {
      // Translate to center the origin
      double dist =
          (std::pow(2, i) - 1) / 2.0; // dist is half the width of a quadrant
      double xTranslated = ((double)coord.x) - dist;
      double yTranslated = ((double)coord.y) - dist;
      double temp = yTranslated;
      // Swap and take the negatives
      yTranslated = -xTranslated;
      xTranslated = -temp;
      // Translate back to regular
      coord.x = (int)(xTranslated + dist);
      coord.y = (int)(yTranslated + dist);
    }

    // Right quadrants ?
    // Add 2^order to x
    // Moves coordinate over by half of the width of the new square
    if (nextq == 2 || nextq == 3) {
      coord.x += std::pow(2, i);
    }

    // Bottom quadrants ?
    // Add 2^order to y
    // Moves coordinate over by half of the width of the new square
    if (nextq == 1 || nextq == 2) {
      coord.y += std::pow(2, i);
    }
  }
  return coord;
}

void Hilbert::generateHilbert() {
  for (int i = 0; i < hilbert.size(); i++) {
    hilbert.at(i) = getCoord(i);
  }
}

template<typename T>
std::vector<T> Hilbert::toHilbert(std::vector<std::vector<T>> arr) {
  int len = arr.size();
  int wid = arr.at(0).size();
  if (len != wid) {
    std::cerr << "ERROR (toHilbert): Non-square array passed to toHilbert" << std::endl;
    return;
  }

  int size = len * wid;

  if (size != this->n_points) {
    std::cerr << "ERROR (toHilbert): 2D array passed is not same size as order " << this->order << " Hilbert curve" << std::endl << "Array passed is of size: " << len << " by " << wid;
    return;
  }

  std::vector<T> res(this->n_points);
  for (int i = 0; i < this->n_points; i++) {
    Pair coord = this->hilbert.at(i);
    res.at(i) = arr.at(coord.y).at(coord.x);
  }

  return res;
}

template<typename T>
std::vector<T> Hilbert::toHilbert(std::vector<T> arr) {
  if (arr.size() != this->n_points) {
    std::cerr << "ERROR (toHilbert): 1D array passed is not same size as order " << this->order << " Hilbert curve" << std::endl;
    return;
  }

  std::vector<T> res(this->n_points);
  for (int i = 0; i < this->n_points; i++) {
    Pair coord = this->hilbert.at(i);
    int index = coord.y * std::pow(2, this->order) + coord.x;
    res.at(i) = arr.at(index);
  }

  return res;
}

std::vector<uchar> Hilbert::toHilbert(cv::Mat arr) {
  std::vector<uchar> res(this->n_points);
  for (int i = 0; i < this->n_points; i++) {
    Pair coord = this->hilbert.at(i);
    res.at(i) = arr.at<uchar>(coord.x, coord.y);
  }
  return res;
}





