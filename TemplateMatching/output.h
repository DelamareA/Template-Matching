#ifndef OUTPUT_H
#define OUTPUT_H

#include <QList>
#include <opencv2/opencv.hpp>

class Output{

    public:
        Output(QString imagePath, QString functionName);
        void display();
        void addData(unsigned int x, unsigned int y, unsigned int num);
        QString toString();

    private:
        QList<unsigned int> listX;
        QList<unsigned int> listY;
        QList<unsigned int> listNum;
        cv::Mat baseImage;
        QString functionName;

};

#endif // OUTPUT_H
