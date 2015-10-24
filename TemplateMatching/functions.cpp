#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "functions.h"

Output* templateMatching(QString imagePath, Template* tem, int method, QString methodName){
    cv::Mat img = cv::imread(imagePath.toStdString());;

    int resultWidth =  img.cols - tem->getWidth() + 1;
    int resultHeight = img.rows - tem->getHeigth() + 1;

    cv::Mat result;

    result.create(resultHeight, resultWidth, CV_32FC1);

    matchTemplate(img, tem->getSingleImage(), result, method);
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

    Output* output = new Output(imagePath, methodName, tem);
    output->addData(matchLoc.x, matchLoc.y, 6);

    return output;
}

Output* basicTemplateMatching(QString imagePath, Template* tem){
    return templateMatching(imagePath, tem, CV_TM_SQDIFF_NORMED, "basicTemplateMatching");
}
