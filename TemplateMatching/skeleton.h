#ifndef SKELETON_H
#define SKELETON_H

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <QList>

#include "template.h"

#define MERGE_DISTANCE 0.05
#define DELETE_DISTANCE 0.15

struct LabeledPoint {
    int label;
    cv::Point2i point;
};

class Skeleton {

    public:
        Skeleton(cv::Mat skeletonizedImage, cv::Mat normalImage);
        QList<int> possibleNumbers();
        QList<cv::Point2d> sort(QList<cv::Point2d> list);

    public: // CHANGE
        QList<cv::Point2d> listHoles;
        QList<cv::Point2d> listJunctions;
        QList<cv::Point2d> listLineEnds;
};

#endif // SKELETON_H
