#include "functions.h"
#include <QDebug>

int mostProbableDigit(cv::Mat image, QList<int> digitsOnField){
    cv::Mat blurredImage;
    cv::Mat hsv;
    cv::Mat final;
    cv::Mat finalOneContour;
    GaussianBlur(image, blurredImage, cv::Size(1, 1), 0, 0);
    cvtColor(blurredImage, hsv, CV_BGR2HSV);
    cvtColor(image, final, CV_BGR2GRAY);


    int maxV = 0;
    long mean = 0;
    for (int x = 0; x < image.cols; x++){
        for (int y = 0; y < image.rows; y++){
            cv::Vec3b intensity = hsv.at<cv::Vec3b>(y, x);

            mean += intensity[2];

            if (intensity[2] > maxV){
                maxV = intensity[2];
            }
        }
    }

    mean /= (image.cols * image.rows);

    for (int x = 0; x < image.cols; x++){
        for (int y = 0; y < image.rows; y++){
            cv::Vec3b intensity = hsv.at<cv::Vec3b>(y, x);

            if (intensity[2] > mean * 1.3 || intensity[2] > maxV * 0.7){
                final.at<uchar>(y, x) = 255;
            }
            else {
                final.at<uchar>(y, x) = 0;
            }
        }
    }

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours(final.clone(), contours, hierarchy, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_NONE);

    if (contours.size() < 1){
        qDebug() << "Error : 0 contour found";
    }
    else {
        std::vector<cv::Point> goodCountour = contours[0];

        int maxH = 0;
        for (unsigned int k = 0; k < contours.size(); k++){
            if (cv::minAreaRect(contours[k]).boundingRect().height > maxH){
                goodCountour = contours[k];
                maxH = cv::minAreaRect(contours[k]).boundingRect().height;
            }
        }

        finalOneContour = final.clone();
        cv::Rect rectC = cv::minAreaRect(goodCountour).boundingRect();

        for (int x = rectC.x; x < image.cols && x < rectC.x + rectC.width; x++){
            for (int y = rectC.y; y < image.rows && y < rectC.y + rectC.height; y++){
                if (final.at<uchar>(y, x) == 255 && pointPolygonTest(goodCountour, cv::Point2f(x, y), true) >= 0){
                    finalOneContour.at<uchar>(y, x) = 255;
                }
                else {
                    finalOneContour.at<uchar>(y, x) = 0;
                }
            }
        }

        cv::RotatedRect rect = cv::minAreaRect(goodCountour);

        float angle = rect.angle;

        cv::Size rectSize = rect.size;
        if (rect.angle <= -45) {
            angle += 90.0;
            cv::swap(rectSize.width, rectSize.height);
        }
        cv::Mat rotationMatrix = getRotationMatrix2D(rect.center, angle, 1.0);
        cv::Mat rotated, cropped;
        warpAffine(finalOneContour, rotated, rotationMatrix, cv::Size(finalOneContour.size().width, 2*finalOneContour.size().height), cv::INTER_LANCZOS4);
        getRectSubPix(rotated, rectSize, rect.center, cropped);
        rotated.release();

        cv::Mat resized;
        cv::resize(cropped, resized, cv::Size(36, 45));
        cropped.release();

        cv::Mat skeleton = thinningGuoHall(resized);
        Skeleton ske(skeleton, resized);

        QList<int> possibleDigits = ske.possibleNumbers(digitsOnField);

        if (possibleDigits.empty()){
            return 0;
        }
        else {
            return possibleDigits[0];
        }
    }
}

void runOnDataSet(QList<int> digitsOnField){
    int success = 0;
    int fail = 0;

    for (int i = 0; i < 10; i++){
        for (int j = 0; j <= 10; j++){
            cv::Mat image = cv::imread(QString("temp/dataset/" + QString::number(i) + "/" + QString::number(j) + ".png").toStdString());

            qDebug() << i << j;

            if (i!=3 && i!=4 && i!=7){ // no dataset for these
                int num = mostProbableDigit(image, digitsOnField);
                if (num == i){
                    success++;
                    qDebug() << "Success !";
                }
                else {
                    fail++;
                    qDebug() << "Failure !" << num;
                }
            }

            image.release();
        }
    }

    qDebug() << "Total Success : " << success << " , " << (success * 100) / (success + fail) << " %";
    qDebug() << "Total Failure : " << fail << " , " << (fail * 100) / (success + fail) << " %";
}
