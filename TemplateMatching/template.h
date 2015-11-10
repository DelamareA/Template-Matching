#ifndef TEMPLATE_H
#define TEMPLATE_H

#define TEMPLATES_COUNT 10

#include <QApplication>
#include <opencv2/opencv.hpp>
#include "histogram.h"

class Template {

    public:
        Template(QString path);
        int getWidth();
        int getHeigth();
        cv::Mat getSingleImage();
        cv::Mat getTemplate(int i);
        cv::Point2f getMassCenter(int i);
        cv::Point2f getHalfMassCenter(int half, int i);
        cv::Point2f getHalfMassCenterHori(int half, int i);
        Histogram* getHistoHori(int i);
        Histogram* getHistoVerti(int i);

    private:
        cv::Mat images[TEMPLATES_COUNT];
        cv::Point2f massCenters[TEMPLATES_COUNT];
        cv::Point2f halfMassCenters[TEMPLATES_COUNT][2];
        cv::Point2f halfMassCentersHori[TEMPLATES_COUNT][2];
        Histogram* histoHori[TEMPLATES_COUNT];
        Histogram* histoVerti[TEMPLATES_COUNT];
        int width;
        int height;
};

#endif // TEMPLATE_H
