#include "output.h"


Output::Output(QString imagePath, QString functionName){
    baseImage = cv::imread(imagePath.toStdString());
    this->functionName = functionName;
}

void Output::addData(unsigned int x, unsigned int y, unsigned int num){
    listX.append(x);
    listY.append(y);
    listNum.append(num);
}

void Output::display(){
    cv::namedWindow(functionName.toStdString());
    cv::imshow(functionName.toStdString(), baseImage);
    cv::waitKey(10000);
}

QString Output::toString(){
    return "Kapoue";
}
