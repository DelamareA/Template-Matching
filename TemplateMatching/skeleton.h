#ifndef SKELETON_H
#define SKELETON_H

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <QList>

class Skeleton {

    public:
        Skeleton(cv::Mat skeletonizedImage, cv::Mat normalImage);

    public: // CHANGE
        QList<cv::Point2i> listHoles;
        QList<cv::Point2i> listJunctions;
        QList<cv::Point2i> listLineEnds;
};

#endif // SKELETON_H
