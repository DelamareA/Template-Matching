#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QApplication>
#include <opencv2/opencv.hpp>

class Template {

    public:
        Template(QString path);
        int getWidth();
        int getHeigth();
        cv::Mat getSingleImage();

    private:
        cv::Mat images[10];
        int width;
        int height;
};

#endif // TEMPLATE_H
