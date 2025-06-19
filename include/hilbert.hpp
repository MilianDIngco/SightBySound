#ifndef HILBERT_H
#define HILBERT_H

#include "opencv2/core/mat.hpp"
#include "settings.hpp"
#include <vector>

struct Pair;

class Hilbert {
  private:
    int order;
    int n_points;
    std::vector<Pair> hilbert;  

    Pair getCoord(int index);

    void generateHilbert();

    int getNPoints(int order);

  public:
    Hilbert(int order);
    Hilbert() : Hilbert(3) {};
    Hilbert(Settings settings) : Hilbert(settings.order) {};

    template<typename T>
    std::vector<T> toHilbert(std::vector<std::vector<T>> arr);
    
    template<typename T>
    std::vector<T> toHilbert(std::vector<T> arr);

    std::vector<uchar> toHilbert(cv::Mat arr);
};

#endif // !HILBERT_H

