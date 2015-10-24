#include "template.h"
#include <QDebug>

Template::Template(QString path){
    width = -2;
    height = -2;
    for (int i = 0; i < 10; i++){
        QString file = path + QString::number(i) + ".png";
        images[i] = cv::imread(file.toStdString());

        if (width == -2){
            width = images[i].cols;
        }
        else if (images[i].cols != width){
            qDebug() << "Error : all templates must be the same size";
        }

        if (height == -2){
            height = images[i].rows;
        }
        else if (images[i].rows != height){
            qDebug() << "Error : all templates must be the same size";
        }
    }
}

int Template::getWidth(){
    return width;
}

int Template::getHeigth(){
    return height;
}

cv::Mat Template::getSingleImage(){
    return images[6];
}
