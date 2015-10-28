#include "configuration.h"

cv::Vec3b Configuration::numberColor;
int Configuration::maxNumberColorDistance = 0;
int Configuration::minDistanceBetweenNumbers = 0;

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

int Configuration::getMaxNumberColorDistance(){
    return Configuration::maxNumberColorDistance;
}

int Configuration::getMinDistanceBetweenNumbers(){
    return Configuration::minDistanceBetweenNumbers;
}
