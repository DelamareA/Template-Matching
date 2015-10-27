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

    private:
        cv::Mat images[TEMPLATES_COUNT];
        int width;
        int height;
};

#endif // TEMPLATE_H
