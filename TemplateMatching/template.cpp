#include "template.h"
#include "functions.h"
#include <QDebug>

Template::Template(QString path){
    width = -2;
    height = -2;
    for (int i = 0; i < TEMPLATES_COUNT; i++){
        QString file = path + QString::number(i) + ".png";
        cv::Mat im = cv::imread(file.toStdString());
        cvtColor(im, images[i], CV_RGB2GRAY);

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

        std::vector<std::vector<cv::Point> > templateContours;
        std::vector<cv::Vec4i> hierarchy;

        findContours(images[i].clone(), templateContours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

        if (templateContours.size() == 0){
            qDebug() << "Error, template doesn't have a contour";
        }
        else {
            cv::Moments imageMoments = moments(templateContours[0], false);
            massCenters[i] = cv::Point2f(imageMoments.m10/imageMoments.m00 , imageMoments.m01/imageMoments.m00);
        }

        cv::Mat firstHalf = cv::Mat(images[i].clone(), cv::Rect(0, 0, images[i].cols/2, images[i].rows));
        cv::Mat secondHalf = cv::Mat(images[i].clone(), cv::Rect(images[i].cols/2, 0, images[i].cols/2, images[i].rows));

        halfMassCenters[i][0] = getMassCenterFromImage(firstHalf);
        halfMassCenters[i][1] = getMassCenterFromImage(secondHalf);

        cv::Mat firstHalfHori = cv::Mat(images[i].clone(), cv::Rect(0, 0, images[i].cols, images[i].rows/2));
        cv::Mat secondHalfHori = cv::Mat(images[i].clone(), cv::Rect(0, images[i].rows/2, images[i].cols, images[i].rows/2));

        halfMassCentersHori[i][0] = getMassCenterFromImage(firstHalfHori);
        halfMassCentersHori[i][1] = getMassCenterFromImage(secondHalfHori);


        Histogram* hori = new Histogram(width);
        Histogram* verti = new Histogram(height);

        for (int x = 0; x < images[i].cols; x++){
            for (int y = 0; y < images[i].rows; y++){
                hori->add(x, images[i].at<uchar>(y, x)/255.0);
                verti->add(y ,images[i].at<uchar>(y, x)/255.0);
            }
        }

        histoHori[i] = hori;
        histoVerti[i] = verti;
    }
}

int Template::getWidth(){
    return width;
}

int Template::getHeigth(){
    return height;
}

cv::Mat Template::getTemplate(int i){
    return images[i];
}

cv::Point2f Template::getMassCenter(int i){
    return massCenters[i];
}

cv::Point2f Template::getHalfMassCenter(int half, int i){
    return halfMassCenters[i][half];
}

cv::Point2f Template::getHalfMassCenterHori(int half, int i){
    return halfMassCentersHori[i][half];
}

cv::Mat Template::getSingleImage(){
    return images[6];
}

Histogram* Template::getHistoHori(int i){
    return histoHori[i];
}

Histogram* Template::getHistoVerti(int i){
    return histoVerti[i];
}
