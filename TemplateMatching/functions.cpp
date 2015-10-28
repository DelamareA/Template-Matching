#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "functions.h"
#include "configuration.h"

Output* templateMatching(QString imagePath, Template* tem, int method, QString methodName){
    cv::Mat image = cv::imread(imagePath.toStdString());
    cv::Mat imageTransformed = image;
    cv::Mat grayScaleImage;
    cvtColor(image, grayScaleImage, CV_BGR2GRAY);
    //cvtColor(image, imageTransformed, CV_BGR2HSV);

    cv::Vec3b numberColor = Configuration::getNumberColor();
    int maxNumberColorDistance = Configuration::getMaxNumberColorDistance();

    for (int x = 0; x < imageTransformed.cols; x++){
        for (int y = 0; y < imageTransformed.rows; y++){
            cv::Vec3b intensity = imageTransformed.at<cv::Vec3b>(y, x);

            int dist = colorDistance(numberColor, intensity);

            if (dist < maxNumberColorDistance){
                grayScaleImage.at<uchar>(y, x) = 255;
            }
            else {
                grayScaleImage.at<uchar>(y, x) = 0;
            }

            /*ColorRange team1 = Configuration::getColorRangeTeam1();
            ColorRange team2 = Configuration::getColorRangeTeam2();
            ColorRange white = Configuration::getColorRangeWhite();

            if (((team1.isInvertHRange() && (intensity.val[0] < team1.getH1() || intensity.val[0] > team1.getH2())) || (!team1.isInvertHRange() && (intensity.val[0] > team1.getH1() && intensity.val[0] < team1.getH2())))
                && (intensity.val[1] > team1.getS1() && intensity.val[1] < team1.getS2())
                && (intensity.val[2] > team1.getV1() && intensity.val[2] < team1.getV2()) ){
                grayScaleImage.at<uchar>(y, x) = 0;
            }*/
            /*else if (((team2.isInvertHRange() && (intensity.val[0] < team2.getH1() || intensity.val[0] > team2.getH2())) || (!team2.isInvertHRange() && (intensity.val[0] > team2.getH1() && intensity.val[0] < team2.getH2())))
                && (intensity.val[1] > team2.getS1() && intensity.val[1] < team2.getS2())
                && (intensity.val[2] > team2.getV1() && intensity.val[2] < team2.getV2()) ){
                grayScaleImage.at<uchar>(y, x) = 0;
            }*/
            /*else if (((white.isInvertHRange() && (intensity.val[0] < white.getH1() || intensity.val[0] > white.getH2())) || (!white.isInvertHRange() && (intensity.val[0] > white.getH1() && intensity.val[0] < white.getH2())))
                && (intensity.val[1] > white.getS1() && intensity.val[1] < white.getS2())
                && (intensity.val[2] > white.getV1() && intensity.val[2] < white.getV2()) ){
                grayScaleImage.at<uchar>(y, x) = 255;
            }
            else {
                grayScaleImage.at<uchar>(y, x) = 255;
            }*/
            /*else if (intensity.val[2] > 10 && intensity.val[2] < 90 && intensity.val[1] < 50 && intensity.val[0] < 50){
                grayScaleImage.at<uchar>(y, x) = 0;
            }
            else if (intensity.val[2] > 90 && intensity.val[2] < 170 && intensity.val[1] > 70 && intensity.val[1] < 150 && intensity.val[0] > 50 && intensity.val[0] < 130){
                grayScaleImage.at<uchar>(y, x) = 255;
            }*/
        }
    }

    int resultWidth =  grayScaleImage.cols - tem->getWidth() + 1;
    int resultHeight = grayScaleImage.rows - tem->getHeigth() + 1;

    Output* output = new Output(grayScaleImage, methodName, tem);

    cv::Mat results[TEMPLATES_COUNT];
    double minVals[TEMPLATES_COUNT];
    double maxVals[TEMPLATES_COUNT];
    cv::Point matchLocs[TEMPLATES_COUNT];

    for (int i = 0; i < TEMPLATES_COUNT; i++){

        results[i].create(resultHeight, resultWidth, CV_32FC1);

        matchTemplate(grayScaleImage, tem->getTemplate(i), results[i], method);
        normalize(results[i], results[i], 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

        cv::Point minLoc;
        cv::Point maxLoc;

        minMaxLoc(results[i], &minVals[i], &maxVals[i], &minLoc, &maxLoc, cv::Mat());

        matchLocs[i] = maxLoc;

        if (method == CV_TM_SQDIFF || method == CV_TM_SQDIFF_NORMED){ // the best match is the min value using these methods
            matchLocs[i] = minLoc;
        }
    }

    for (int i = 0; i < TEMPLATES_COUNT; i++){
        bool willBeDisplayed = true;
        for (int j = 0; j < TEMPLATES_COUNT; j++){
            if (i != j){
                double dist = sqrt((matchLocs[i].x - matchLocs[j].x)*(matchLocs[i].x - matchLocs[j].x) + (matchLocs[i].y - matchLocs[j].y)*(matchLocs[i].y - matchLocs[j].y));

                if (dist < Configuration::getMinDistanceBetweenNumbers() && (maxVals[i] < maxVals[j] || (maxVals[i] == maxVals[j] && j < i))){
                    willBeDisplayed = false;
                }
            }
        }

        if (willBeDisplayed){
            output->addData(matchLocs[i].x, matchLocs[i].y, i);
        }
    }

    return output;
}

Output* basicTemplateMatching(QString imagePath, Template* tem){
    return templateMatching(imagePath, tem, CV_TM_CCOEFF_NORMED, "basicTemplateMatching");
}

int colorDistance(cv::Vec3b c1, cv::Vec3b c2){
    int diff0 = c1[0] - c2[0];
    int diff1 = c1[1] - c2[1];
    int diff2 = c1[2] - c2[2];
    return (int) sqrt((diff0 * diff0) + (diff1 * diff1) + (diff2 * diff2));
}
