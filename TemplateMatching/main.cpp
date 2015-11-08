#include <QApplication>
#include <opencv2/opencv.hpp>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "functions.h"
#include "output.h"
#include "configuration.h"

int loadAndRun(QString imagePath, QString videoPath, QString outputVideoPath, bool isVideo, QString templatesPath, QString outputPath, QString configPath, QString backgroundPath);

int main(int argc, char *argv[]){

    bool isVideo = true;
    QString imagePath = "screenshot5.png";
    QString videoPath = "28.mp4";
    QString outputVideoPath = "output.avi";
    QString templatesPath = "templatesNumber4/";
    QString outputPath = "output.txt";
    QString configPath = "config.txt";
    QString backgroundPath = "backgroundBlackBorders2.png";

    return loadAndRun(imagePath, videoPath, outputVideoPath, isVideo, templatesPath, outputPath, configPath, backgroundPath);
}

int loadAndRun(QString imagePath, QString videoPath, QString outputVideoPath, bool isVideo, QString templatesPath, QString outputPath, QString configPath, QString backgroundPath){

    Template* templateNumbers = new Template(templatesPath);
    Configuration::setConfigFromFile(configPath);
    cv::Mat background = cv::imread(backgroundPath.toStdString());

    Output* out = 0;

    if (isVideo){
        cv::VideoCapture inputVideo(videoPath.toStdString());
        if (!inputVideo.isOpened()){
            qDebug() << "Could not open video";
            return -1;
        }

        cv::Size size = cv::Size((int) inputVideo.get(CV_CAP_PROP_FRAME_WIDTH), (int) inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));

        cv::VideoWriter outputVideo;
        outputVideo.open(outputVideoPath.toStdString(), -1, 4, size, true);

        if (!outputVideo.isOpened()){
            qDebug() << "Could not open video output";
            return -1;
        }

        cv::Mat image;
        inputVideo >> image;
        int count = 0;

        while (!image.empty()){
            if (image.rows != background.rows || image.cols != background.cols){
                qDebug() << "Image and background have not the same size : " << image.cols << "x" << image.rows;
            }

            out = basicTemplateMatching(image, templateNumbers, background);
            outputVideo << out->getImage();

            delete out;

            qDebug() << count;

            for (int i = 0; i < 6; i++){
                inputVideo >> image;
                count++;
            }
        }

    }
    else {
        cv::Mat image = cv::imread(imagePath.toStdString());

        if (image.rows != background.rows || image.cols != background.cols){
            qDebug() << "Image and background have not the same size";
        }

        out = basicTemplateMatching(image, templateNumbers, background);

        cv::namedWindow("Output");
        cv::imshow("Output", out->getImage());
        cv::waitKey(10000);

        delete out;
    }

    /*QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << out->toString() << endl;
        file.close();
    }
    else {
        qDebug() << "Cannot open " + outputPath;
    }*/

    return 0;
}
