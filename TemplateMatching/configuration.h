#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "colorrange.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define CONFIG_LINES_COUNT 4

class Configuration {
    public:
        static void setConfigFromFile(QString path);
        static cv::Vec3b getNumberColor();
        static int getMaxNumberColorDistance();
        static int getMaxBackgroundColorDistance();
        static int getMinDistanceBetweenNumbers();

    private:
        Configuration();
        static cv::Vec3b numberColor;
        static int maxNumberColorDistance;
        static int minDistanceBetweenNumbers;
        static int maxBackgroundColorDistance;
};

#endif // CONFIGURATION_H
