#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <QApplication>
#include <opencv2/opencv.hpp>

class Template {

    public:
        Template(QString path);

    private:
        cv::Mat images[10];
};

#endif // TEMPLATE_H
