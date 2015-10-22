#include "template.h"

Template::Template(QString path){
    for (int i = 0; i < 10; i++){
        QString file = path + QString::number(i) + ".png";
        images[i] = cv::imread(file.toStdString());
    }
}

