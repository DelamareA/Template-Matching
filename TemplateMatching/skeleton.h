#ifndef SKELETON_H
#define SKELETON_H

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <QList>

#include "template.h"

#define MERGE_DISTANCE 0.05
#define DELETE_DISTANCE 0.15
#define FAKE_LOOPS_DISTANCE 0.15
#define JUNCTION_MARGIN 0.05

#define VECTOR_DIMENSION 12
#define VECTOR_DIMENSION_3_5 2

#define END 3
#define END_3_5 1

#define JUNCTION 2
#define JUNCTION_3_5 0

enum {M3_5};

struct LabeledPoint {
    int label;
    cv::Point2i point;
};

class Skeleton {

    public:
        Skeleton(cv::Mat skeletonizedImage, cv::Mat normalImage);
        QList<int> possibleNumbers();
        QList<cv::Point2d> sort(QList<cv::Point2d> list);
        double min(double a, double b);
        QList<double> vectorization(int type);

        static int getDim(int type);
        static int getEndCount(int type);
        static int getJunctionCount(int type);

    public: // CHANGE
        QList<LabeledPoint> startList;
        QList<cv::Point2d> listHoles;
        QList<cv::Point2d> listFakeHoles;
        QList<cv::Point2d> listJunctions;
        QList<cv::Point2d> listLineEnds;
};

#endif // SKELETON_H
