#ifndef HILBERT_H
#define HILBERT_H

#include "settings.hpp"
#include <cmath>
#include <vector>
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

class Hilbert {
  private:
    int order;
    int n_points;
    std::vector<Pair> hilbert;  

    Pair getCoord(int index);

    void generateHilbert();

    int getNPoints(int order);

  public:
    Hilbert(Settings settings);

    template<typename T>
    std::vector<T> toHilbert(std::vector<std::vector<T>> arr) {
      int len = arr.size();
      int wid = arr.at(0).size();
      std::vector<T> res(this->n_points);
      if (len != wid) {
        std::cerr << "ERROR (toHilbert): Non-square array passed to toHilbert" << std::endl;
        return res;
      }

      int size = len * wid;

      if (size != this->n_points) {
        std::cerr << "ERROR (toHilbert): 2D array passed is not same size as order " << this->order << " Hilbert curve" << std::endl << "Array passed is of size: " << len << " by " << wid;
        return res;
      }

      for (int i = 0; i < this->n_points; i++) {
        Pair coord = this->hilbert.at(i);
        res.at(i) = arr.at(coord.y).at(coord.x);
      }

      return res;
    };
    
    template<typename T>
    std::vector<T> toHilbert(std::vector<T> arr) {
      std::vector<T> res(this->n_points);
      if (arr.size() != this->n_points) {
        std::cerr << "ERROR (toHilbert): 1D array passed is not same size as order " << this->order << " Hilbert curve" << std::endl;
        return res;
      }

      for (int i = 0; i < this->n_points; i++) {
        Pair coord = this->hilbert.at(i);
        int index = coord.y * std::pow(2, this->order) + coord.x;
        res.at(i) = arr.at(index);
      }

      return res;
    };

    int get_order() { return this->order; };
    int get_n_points() { return this->n_points; };
};

#endif // !HILBERT_H

