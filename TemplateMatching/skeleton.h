#ifndef SKELETON_H
#define SKELETON_H

#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <QList>

#include "template.h"
#include "functions.h"

#define MERGE_DISTANCE 0.05
#define DELETE_DISTANCE 0.15
#define FAKE_LOOPS_DISTANCE 0.15
#define JUNCTION_MARGIN 0.05

#define END_0 1
#define END_1 1

#define JUNCTION_0 0
#define JUNCTION_1 1

#define HOLE_0 0
#define HOLE_1 1

#define MASS_CENTER_0 0
#define MASS_CENTER_1 0

#define TOTAL_0 1
#define TOTAL_1 0

#define VECTOR_DIMENSION_0 (2*END_0 + 2*JUNCTION_0 + 2*HOLE_0 + 2*MASS_CENTER_0 + TOTAL_0)
#define VECTOR_DIMENSION_1 (2*END_1 + 2*JUNCTION_1 + 2*HOLE_1 + 2*MASS_CENTER_1 + TOTAL_1)

enum {M0, M1};


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
        cv::Point2d getMassCenter(cv::Mat ske);
        double getCount(cv::Mat ske);

        static int getDim(int type);
        static int getEndCount(int type);
        static int getJunctionCount(int type);
        static int getHoleCount(int type);
        static int getMassCenterCount(int type);
        static int getTotalCount(int type);

        static void setMachines(Machines newMachines);

    public: // CHANGE
        QList<LabeledPoint> startList;
        QList<cv::Point2d> listHoles;
        QList<cv::Point2d> listFakeHoles;
        QList<cv::Point2d> listJunctions;
        QList<cv::Point2d> listLineEnds;
        cv::Point2d massCenter;
        double total;


};

#endif // SKELETON_H
