#include "output.h"


Output::Output(QString imagePath, QString functionName, Template* tem){
    baseImage = cv::imread(imagePath.toStdString());
    this->functionName = functionName;
    this->tem = tem;
}

void Output::addData(unsigned int x, unsigned int y, unsigned int num){
    listX.append(x);
    listY.append(y);
    listNum.append(num);
}

void Output::display(){

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

        putText(baseImage, QString::number(listNum[i]).toStdString(), p3, cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0,0,255));
    }


    cv::namedWindow(functionName.toStdString());
    cv::imshow(functionName.toStdString(), baseImage);
    cv::waitKey(10000);
}

QString Output::toString(){
    QString out = "1@";

    out += QString::number(listX.size());
    out += '%';

    for (int i = 0; i < listX.size(); i++){
        out += QString::number(listNum[i]) + "#" + QString::number(listX[i]) + "#" + QString::number(listY[i]);
        out += '%';
    }

    out += "@";
    return out;
}