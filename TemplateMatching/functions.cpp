#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "functions.h"

Output* templateMatching(QString imagePath, Template* tem, int method, QString methodName){
    cv::Mat imageTransformed = cv::imread(imagePath.toStdString());
    cv::Mat grayScaleImage;
    cvtColor(imageTransformed, grayScaleImage, CV_RGB2GRAY);

    for (int x = 0; x < imageTransformed.cols; x++){
        for (int y = 0; y < imageTransformed.rows; y++){
            cv::Vec3b intensity = imageTransformed.at<cv::Vec3b>(y, x);

            if (intensity.val[2] > 20 && intensity.val[2] < 100 && intensity.val[1] > 40 && intensity.val[1] < 120 && intensity.val[0] > 20 && intensity.val[0] < 100){
                grayScaleImage.at<uchar>(y, x) = 0;
            }
            else if (intensity.val[2] > 10 && intensity.val[2] < 90 && intensity.val[1] < 50 && intensity.val[0] < 50){
                grayScaleImage.at<uchar>(y, x) = 0;
            }
            else if (intensity.val[2] > 90 && intensity.val[2] < 170 && intensity.val[1] > 70 && intensity.val[1] < 150 && intensity.val[0] > 50 && intensity.val[0] < 130){
                grayScaleImage.at<uchar>(y, x) = 255;
            }
        }
    }

    int resultWidth =  grayScaleImage.cols - tem->getWidth() + 1;
    int resultHeight = grayScaleImage.rows - tem->getHeigth() + 1;

    Output* output = new Output(grayScaleImage, methodName, tem);

    for (int i = 0; i < TEMPLATES_COUNT; i++){
        cv::Mat result;

        result.create(resultHeight, resultWidth, CV_32FC1);

        matchTemplate(grayScaleImage, tem->getTemplate(i), result, method);
        normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

        double minVal;
        double maxVal;

        cv::Point minLoc;
        cv::Point maxLoc;

        minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());

        cv::Point matchLoc = maxLoc;

        if (method == CV_TM_SQDIFF || method == CV_TM_SQDIFF_NORMED){ // the best match is the min value using these methods
            matchLoc = minLoc;
        }


        output->addData(matchLoc.x, matchLoc.y, i);
    }

    return output;
}

Output* basicTemplateMatching(QString imagePath, Template* tem){
    return templateMatching(imagePath, tem, CV_TM_CCOEFF_NORMED, "basicTemplateMatching");
}
