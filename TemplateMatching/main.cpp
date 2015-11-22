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
    QString imagePath = "screenshot4.png";
    QString videoPath = "28.mp4";
    QString outputVideoPath = "output.avi";
    QString templatesPath = "templatesNumber4/";
    QString outputPath = "output.txt";
    QString configPath = "config.txt";
    QString backgroundPath = "backgroundBlackBorders2.png";


    QList<int> numbers;
    numbers.push_back(3);
    numbers.push_back(5);
    generateDataSet(numbers, 100, 36, 45, "dataset35/", "labels/35.txt");


    return 0;//loadAndRun(imagePath, videoPath, outputVideoPath, isVideo, templatesPath, outputPath, configPath, backgroundPath);
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
        int frameCount = 0;

        QString outputText;
        int width = 0;
        int height = 0;

        while (!image.empty()){
            if (image.rows != background.rows || image.cols != background.cols){
                qDebug() << "Image and background have not the same size : " << image.cols << "x" << image.rows;
            }

            width = image.cols;
            height = image.rows;

            qDebug() << "Start frame : " << frameCount;

            out = basicTemplateMatching(image, templateNumbers, background);
            outputVideo << out->getImage();
            frameCount++;
            outputText += out->toString();

            delete out;

            qDebug() << "End frame : " << frameCount-1;

            for (int i = 0; i < inputVideo.get(CV_CAP_PROP_FPS); i++){
                inputVideo >> image;
                count++;
            }
        }

        outputText = QString::number(width) + '@' + QString::number(height) + '@' + QString::number(frameCount) + "@" + outputText + "@";

        QFile file(outputPath);
        if (file.open(QIODevice::WriteOnly)){
            QTextStream stream(&file);
            stream << outputText << endl;
            file.close();
        }
        else {
            qDebug() << "Cannot open " + outputPath;
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
        cv::waitKey(40000);

        delete out;
    }

    return 0;
}
