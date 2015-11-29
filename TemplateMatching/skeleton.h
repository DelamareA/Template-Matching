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
#define VECTOR_DIMENSION_0 2

#define END 3
#define END_0 1

#define JUNCTION 2
#define JUNCTION_0 0

enum {M0};


struct Machines{
    cv::Ptr<cv::ml::SVM> m[TEMPLATES_COUNT][TEMPLATES_COUNT];
};

struct LabeledPoint {
    int label;
    cv::Point2i point;
};

class Skeleton {

    public:
        static Machines machines;

        Skeleton(cv::Mat skeletonizedImage, cv::Mat normalImage);
        QList<int> possibleNumbers();
        QList<cv::Point2d> sort(QList<cv::Point2d> list);
        double min(double a, double b);
        double max(double a, double b);
        int min(int a, int b);
        int max(int a, int b);
        QList<double> vectorization(int type);

        static int getDim(int type);
        static int getEndCount(int type);
        static int getJunctionCount(int type);

        static void setMachines(Machines newMachines);

    public: // CHANGE
        QList<LabeledPoint> startList;
        QList<cv::Point2d> listHoles;
        QList<cv::Point2d> listFakeHoles;
        QList<cv::Point2d> listJunctions;
        QList<cv::Point2d> listLineEnds;


};

#endif // SKELETON_H
