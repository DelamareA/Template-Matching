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
                painter.setFont(QFont("Arial", size));
                break;

                case 2:
                painter.setFont(QFont("Arial", size));
                break;
            }

            painter.translate(width, size);
            //double realAngle = (rand() % 100) - 50;
            //painter.rotate(realAngle);
            painter.drawText(QPoint(0, size), QString::number(numbers[i]));

            QImage image = pix.toImage();

            Mat mat = Mat::zeros(pix.height(), pix.width(), CV_8U);
            for (int x = 0; x < pix.width(); x++){
                for (int y = 0; y < pix.height(); y++){
                    QRgb val = image.pixel(x, y);

                    if (qRed(val) > 120){
                        mat.at<uchar>(y, x) = 255;
                    }
                    else {
                        mat.at<uchar>(y, x) = 0;
                    }
                }
            }

            // GET CONTOUR

            std::vector<std::vector<Point> > contours;
            std::vector<Vec4i> hierarchy;

            findContours(mat.clone(), contours, hierarchy, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_NONE);

            if (contours.size() < 1){
                qDebug() << "Error : 0 contour found";

                // just to avoid future bugs
                imwrite(QString(outputPath + "dataset/" + QString::number(i * countPerNumber + j) + ".png").toStdString(), mat);
            }
            else {
                RotatedRect rect = minAreaRect(contours[0]);

                float angle = rect.angle;

                Size rectSize = rect.size;
                if (rect.angle <= -35) {
                    angle += 90.0;
                    swap(rectSize.width, rectSize.height);
                }
                //qDebug() << angle << " vs " << realAngle;
                Mat rotationMatrix = getRotationMatrix2D(rect.center, angle, 1.0);
                Mat rotated, cropped;
                warpAffine(mat, rotated, rotationMatrix, mat.size(), INTER_CUBIC);
                getRectSubPix(rotated, rectSize, rect.center, cropped);

                Mat resized;
                resize(cropped, resized, Size(width, height));

                imwrite(QString(outputPath + "dataset/" + QString::number(i * countPerNumber + j) + ".png").toStdString(), resized);
            }

            /*int minX = pix.width()-1;
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
            image.copy(rect).scaled(width, height).save(outputPath + "dataset/" + QString::number(i * countPerNumber + j) + ".png");*/

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


void generateSVM(QString path, int type){
    QFile file(path + "labels.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << "Cannot open " + path + "labels.txt";
        return;
    }

    QTextStream in(&file);
    QStringList labelsString = in.readAll().split(' ');    

    int count = labelsString[0].toInt();

    int dim = Skeleton::getDim(type);

    float labels[count];
    float trainingData[count][dim];

    for (int i = 0; i < count; i++){
        cv::Mat image;
        image = cv::imread((path + "dataset/" + QString::number(i) + ".png").toStdString(), CV_LOAD_IMAGE_COLOR);

        cv::Mat grayScaleImage;
        cvtColor(image, grayScaleImage, CV_BGR2GRAY);

        cv::Mat skeleton = thinningGuoHall(grayScaleImage);
        Skeleton ske(skeleton, grayScaleImage);

        cv::imwrite((path + "skeletons/" + QString::number(i) + ".png").toStdString(), skeleton);

        QList<double> vect = ske.vectorization(type);

        labels[i] = labelsString[i+1].toInt();
        for (int j = 0; j < dim; j++){
            trainingData[i][j] = 2 * vect[j] - 1;
        }
    }
    Mat labelsMat(count, 1, CV_32SC1, labels);
    Mat trainingDataMat(count, dim, CV_32FC1, trainingData);

    Ptr<ml::SVM> svm = ml::SVM::create();

    Ptr<TrainData> data = TrainData::create(trainingDataMat, ROW_SAMPLE, labelsMat);

    //svm->setGamma(3);
    svm->setKernel(cv::ml::SVM::RBF);
    //svm->setType(cv::ml::SVM::C_SVC);
    svm->setGamma(3);
    svm->trainAuto(data);
    svm->save((path + "svm.xml").toStdString());


}
