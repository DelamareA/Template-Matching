#include "output.h"


Output::Output(cv::Mat image, Template* tem){
    //cvtColor(image, baseImage, CV_GRAY2BGR);
    baseImage = image;
    this->tem = tem;
}

void Output::addData(unsigned int x, unsigned int y, unsigned int num){

    bool merged = false;
    for (int i = 0; i < listX.size(); i++){
        if (abs(x-listX[i]) < MERGE_DISTANCE_X && abs(y-listY[i]) < MERGE_DISTANCE_Y){
            merged = true;

            /*
             * If the number on the left is a '1', then the real number must be
             * greater than 10, otherwise, it is probably a mistake, so we discard it
            */
            if (num == 1 && x < listX[i]){
                listNum[i] += 10;
            }
            else if (listNum[i] == 1 && x > listX[i]){
                listNum[i] = 10 + num;
            }
        }
    }

    if (!merged){
        listX.append(x);
        listY.append(y);
        listNum.append(num);
    }
}

cv::Mat Output::getImage(){

    for (int i = 0; i < listX.size(); i++){
        cv::Point p1;
        cv::Point p2;
        cv::Point p3;

        p1.x = listX[i];
        p1.y = listY[i];

        p2.x = listX[i] + tem->getWidth();
        p2.y = listY[i] + tem->getHeigth();

        p3.x = listX[i];
        p3.y = listY[i] + tem->getHeigth();

        rectangle(baseImage, p1, p2, cv::Scalar(0,0,255));

        putText(baseImage, QString::number(listNum[i]).toStdString(), p3, cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 1, cv::Scalar(0,0,255));
    }

    return baseImage;
}

QString Output::toString(){
    QString out;
    out += QString::number(listX.size());
    out += '%';

    for (int i = 0; i < listX.size(); i++){
        out += QString::number(listNum[i]) + "#" + QString::number(listX[i]) + "#" + QString::number(listY[i]);
        out += '%';
    }

    out += '@';

    return out;
}
