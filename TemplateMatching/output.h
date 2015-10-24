#ifndef OUTPUT_H
#define OUTPUT_H

#include <QList>
#include <opencv2/opencv.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include "template.h"

class Output{

    public:
        Output(QString imagePath, QString functionName, Template *tem);
        void display();
        void addData(unsigned int x, unsigned int y, unsigned int num);
        QString toString();

    private:
        QList<unsigned int> listX;
        QList<unsigned int> listY;
        QList<unsigned int> listNum;
        cv::Mat baseImage;
        QString functionName;
        Template* tem;

};

#endif // OUTPUT_H
