#include <QList>
#include <QPixmap>
#include <QPainter>
#include <QTime>
#include <QApplication>
#include <QDebug>
#include <QColor>
#include <QFile>
#include <qglobal.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml.hpp>

#include "functions.h"

using namespace cv;
using namespace cv::ml;

void generateDataSet(QList<int> numbers, int countPerNumber, int width, int height, QString outputPath) {
    char **argv = new char*[1];
    argv[0] = new char[1];
    int argc = 1;
    QApplication a(argc, argv); // just to use QPixmaps

    qsrand(QTime::currentTime().msec());

    int size = 50;

    QString labels = QString::number(numbers.size() * countPerNumber);

    for (int i = 0; i < numbers.size(); i++){
        for (int j = 0; j < countPerNumber; j++){
            QPixmap pix(3 * width, 3 * height);
            pix.fill(Qt::black);
            QPainter painter(&pix);
            painter.setPen(QColor(255, 255, 255));

            switch(rand() % 3){
                default :
                painter.setFont(QFont("Arial", size));
                break;

                case 1:
                painter.setFont(QFont("Courrier", size));
                break;

                case 2:
                painter.setFont(QFont("Calibri", size));
                break;
            }

            painter.translate(width, size);
            painter.rotate((rand() % 100) - 50);
            painter.drawText(QPoint(0, size), QString::number(numbers[i]));

            QImage image = pix.toImage();
            int minX = pix.width()-1;
            int maxX = 0;
            int minY = pix.height()-1;
            int maxY = 0;
            for (int x = 0; x < pix.width(); x++){
                for (int y = 0; y < pix.height(); y++){
                    QRgb val = image.pixel(x, y);

                    if (qRed(val) > 120){
                        image.setPixel(x, y, qRgb(255, 255, 255));

                        if (x < minX){
                            minX = x;
                        }
                        if (x > maxX){
                            maxX = x;
                        }
                        if (y < minY){
                            minY = y;
                        }
                        if (y > maxY){
                            maxY = y;
                        }
                    }
                    else {
                        image.setPixel(x, y, qRgb(0, 0, 0));
                    }
                }
            }

            QRect rect(QPoint(minX, minY), QPoint(maxX, maxY));
            image.copy(rect).scaled(width, height).save(outputPath + "dataset/" + QString::number(i * countPerNumber + j) + ".png");

            labels += " " + QString::number(numbers[i]);
        }
    }

    QFile file(outputPath + "labels.txt");
    if (file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << labels << endl;
        file.close();
    }
    else {
        qDebug() << "Cannot open " + outputPath + "labels.txt";
    }
}


void generateSVM(QString path){
    QFile file(path + "labels.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Cannot open " + path + "labels.txt";
        return;
    }

    QTextStream in(&file);
    QStringList labelsString = in.readAll().split(' ');

    int count = labelsString[0].toInt();

    double labels[count];
    double trainingData[count][VECTOR_DIMENSION];

    for (int i = 0; i < count; i++){
        cv::Mat image;
        image = cv::imread((path + "dataset/" + QString::number(i) + ".png").toStdString(), CV_LOAD_IMAGE_COLOR);

        cv::Mat grayScaleImage;
        cvtColor(image, grayScaleImage, CV_BGR2GRAY);

        cv::Mat skeleton = thinningGuoHall(grayScaleImage);
        Skeleton ske(skeleton, grayScaleImage);

        cv::imwrite((path + "skeletons/" + QString::number(i) + ".png").toStdString(), skeleton);

        QList<double> vect = ske.vectorization();

        labels[i] = labelsString[i+1].toInt();
        for (int j = 0; j < VECTOR_DIMENSION; j++){
            trainingData[i][j] = vect[j];
        }
    }
    Mat labelsMat(count, 1, CV_32SC1, labels);
    Mat trainingDataMat(count, VECTOR_DIMENSION, CV_32FC1, trainingData);

    Ptr<ml::SVM> svm = ml::SVM::create();

    Ptr<TrainData> data = TrainData::create(trainingDataMat, ROW_SAMPLE, labelsMat);

    svm->trainAuto(data, 200);
    svm->save((path + "svm.xml").toStdString());
}
