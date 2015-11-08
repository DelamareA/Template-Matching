#include "configuration.h"

cv::Vec3b Configuration::numberColor;
cv::Vec3b Configuration::greenColor;
cv::Vec3b Configuration::redColor;
int Configuration::maxNumberColorDistance = 0;
int Configuration::maxGreenColorDistance = 0;
int Configuration::maxRedColorDistance = 0;
int Configuration::minDistanceBetweenNumbers = 0;
int Configuration::maxBackgroundColorDistance = 0;

Configuration::Configuration(){
}

void Configuration::setConfigFromFile(QString path){
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)){
       QString lines;
       QTextStream stream(&file);
       lines = stream.readAll();
       file.close();


       QStringList list = lines.split('\n');

       if (list.size() == CONFIG_LINES_COUNT){
            QStringList l0 = list[0].split(':');
            Configuration::numberColor[0] = l0[1].toInt();
            Configuration::numberColor[1] = l0[2].toInt();
            Configuration::numberColor[2] = l0[3].toInt();

            QStringList l1 = list[1].split(':');
            Configuration::maxNumberColorDistance = l1[1].toInt();

            QStringList l2 = list[2].split(':');
            Configuration::minDistanceBetweenNumbers = l2[1].toInt();

            QStringList l3 = list[3].split(':');
            Configuration::maxBackgroundColorDistance = l3[1].toInt();

            QStringList l4 = list[4].split(':');
            Configuration::greenColor[0] = l4[1].toInt();
            Configuration::greenColor[1] = l4[2].toInt();
            Configuration::greenColor[2] = l4[3].toInt();

            QStringList l5 = list[5].split(':');
            Configuration::maxGreenColorDistance = l5[1].toInt();

            QStringList l6 = list[6].split(':');
            Configuration::redColor[0] = l6[1].toInt();
            Configuration::redColor[1] = l6[2].toInt();
            Configuration::redColor[2] = l6[3].toInt();

            QStringList l7 = list[7].split(':');
            Configuration::maxRedColorDistance = l7[1].toInt();
       }
       else {
           qDebug() << "Config file incorrect";
       }
    }
    else {
        qDebug() << "Cannot open " + path;
    }
}

cv::Vec3b Configuration::getNumberColor(){
    return Configuration::numberColor;
}

cv::Vec3b Configuration::getGreenColor(){
    return Configuration::greenColor;
}

cv::Vec3b Configuration::getRedColor(){
    return Configuration::redColor;
}

int Configuration::getMaxNumberColorDistance(){
    return Configuration::maxNumberColorDistance;
}

int Configuration::getMaxGreenColorDistance(){
    return Configuration::maxGreenColorDistance;
}

int Configuration::getMaxRedColorDistance(){
    return Configuration::maxRedColorDistance;
}

int Configuration::getMaxBackgroundColorDistance(){
    return Configuration::maxBackgroundColorDistance;
}


int Configuration::getMinDistanceBetweenNumbers(){
    return Configuration::minDistanceBetweenNumbers;
}
