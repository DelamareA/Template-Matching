#ifndef TEMPLATE_H
#define TEMPLATE_H

#define TEMPLATES_COUNT 10

#include <QApplication>
#include <opencv2/opencv.hpp>

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

    private:
        cv::Mat images[TEMPLATES_COUNT];
        cv::Point2f massCenters[TEMPLATES_COUNT];
        cv::Point2f halfMassCenters[TEMPLATES_COUNT][2];
        cv::Point2f halfMassCentersHori[TEMPLATES_COUNT][2];
        int width;
        int height;
};

#endif // TEMPLATE_H
