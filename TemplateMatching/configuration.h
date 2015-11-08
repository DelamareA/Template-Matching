#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "colorrange.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define CONFIG_LINES_COUNT 8

class Configuration {
    public:
        static void setConfigFromFile(QString path);
        static cv::Vec3b getNumberColor();
        static cv::Vec3b getGreenColor();
        static cv::Vec3b getRedColor();
        static int getMaxNumberColorDistance();
        static int getMaxGreenColorDistance();
        static int getMaxRedColorDistance();
        static int getMaxBackgroundColorDistance();
        static int getMinDistanceBetweenNumbers();

    private:
        Configuration();
        static cv::Vec3b numberColor;
        static cv::Vec3b greenColor;
        static cv::Vec3b redColor;
        static int maxNumberColorDistance;
        static int maxGreenColorDistance;
        static int maxRedColorDistance;
        static int minDistanceBetweenNumbers;
        static int maxBackgroundColorDistance;
};

#endif // CONFIGURATION_H
