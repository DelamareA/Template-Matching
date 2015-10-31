#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include "functions.h"
#include "configuration.h"
#include <vector>

Output* templateMatching(QString imagePath, Template* tem, int method, QString methodName, QString backgroundPath){
    cv::Mat image = cv::imread(imagePath.toStdString());
    cv::Mat background = cv::imread(backgroundPath.toStdString());

    if (image.rows != background.rows || image.cols != background.cols){
        qDebug() << "Image and background have not the same size";
    }

    cv::Mat imageTransformed = image;

    cv::Mat backgroundMask1;
    cv::Mat grayScaleImage;
    cvtColor(image, grayScaleImage, CV_BGR2GRAY);
    cvtColor(image, backgroundMask1, CV_BGR2GRAY);
    //cvtColor(image, imageTransformed, CV_BGR2HSV);

    int maxBackgroundColorDistance = Configuration::getMaxBackgroundColorDistance();

    for (int x = 0; x < backgroundMask1.cols; x++){
        for (int y = 0; y < backgroundMask1.rows; y++){
            cv::Vec3b intensity = imageTransformed.at<cv::Vec3b>(y, x);
            cv::Vec3b backgroundPixel = background.at<cv::Vec3b>(y, x);

            int distBackground = colorDistance(backgroundPixel, intensity);

            if (distBackground < maxBackgroundColorDistance){
                backgroundMask1.at<uchar>(y, x) = 0;
            }
            else {
                backgroundMask1.at<uchar>(y, x) = 255;
            }
        }
    }

    cv::Mat backgroundMask2 = closeGaps(backgroundMask1, 2, 0.3);

    cv::Vec3b numberColor = Configuration::getNumberColor();
    int maxNumberColorDistance = Configuration::getMaxNumberColorDistance();

    for (int x = 0; x < grayScaleImage.cols; x++){
        for (int y = 0; y < grayScaleImage.rows; y++){
            if (backgroundMask2.at<uchar>(y, x) == 0){
                grayScaleImage.at<uchar>(y, x) = 0;
            }
            else {
                cv::Vec3b intensity = imageTransformed.at<cv::Vec3b>(y, x);

                int distWhite = colorDistance(numberColor, intensity);

                if (distWhite < maxNumberColorDistance){
                    grayScaleImage.at<uchar>(y, x) = 255;
                }
                else {
                    grayScaleImage.at<uchar>(y, x) = 0;
                }
            }
        }
    }

    cv::Mat grayScaleImage2 = closeGaps(grayScaleImage, 1, 0.7);

    //GaussianBlur(grayScaleImage, grayScaleImage, cv::Size(7, 7), 0, 0);

    //______________________________________________________

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours(grayScaleImage2.clone(), contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

    QList<cv::Rect> filteredRects;

    for (unsigned int i = 0; i < contours.size(); i++){
        cv::Rect rect = minAreaRect(contours[i]).boundingRect();
        double ratio = (double) rect.width / rect.height;
        if (rect.width > 8 && rect.height > 15 && rect.width < 25 && rect.height < 30 && ratio > 0.5 && ratio < 1){
            filteredRects.push_back(rect);
        }
    }

    QList<std::vector<cv::Point> > filteredContours;

    for (unsigned int i = 0; i < contours.size(); i++){
        cv::Rect rect = minAreaRect(contours[i]).boundingRect();
        double ratio = (double) rect.width / rect.height;
        if (rect.width > 8 && rect.height > 15 && rect.width < 25 && rect.height < 30 && ratio > 0.5 && ratio < 1){
            filteredContours.push_back(contours[i]);
        }
    }

    cv::Mat grayScaleFilteredComponentsImage = grayScaleImage2.clone();

    for (int x = 0; x < grayScaleFilteredComponentsImage.cols; x++){
        for (int y = 0; y < grayScaleFilteredComponentsImage.rows; y++){
            if (grayScaleFilteredComponentsImage.at<uchar>(y, x) == 255){
                grayScaleFilteredComponentsImage.at<uchar>(y, x) = 0;
                for (int i = 0; i < filteredContours.size(); i++){
                    if (pointPolygonTest(filteredContours[i], cv::Point2f(x,y), true ) >= 0){
                        grayScaleFilteredComponentsImage.at<uchar>(y, x) = 255;
                    }
                }
            }
        }
    }

    cv::Mat* possibleNumbers = new cv::Mat[filteredRects.size()];
    cv::Size temSize(tem->getWidth(), tem->getHeigth());

    cv::Mat finalImage = grayScaleFilteredComponentsImage;

    Output* output = new Output(finalImage, methodName, tem);
    int** correlations = new int*[filteredRects.size()];

    for (int i = 0; i < filteredRects.size(); i++){
        correlations[i] = new int[TEMPLATES_COUNT];
        resize(grayScaleImage2(filteredRects[i]), possibleNumbers[i], temSize);

        cv::imshow("Kapoie", possibleNumbers[i]);
        cv::waitKey(1000);

        for (int j = 0; j < TEMPLATES_COUNT; j++){
            cv::Mat result;
            result.create(1, 1, CV_8UC1);

            matchTemplate(possibleNumbers[i], tem->getTemplate(j), result, method);

            correlations[i][j] = result.at<uchar>(0,0);
        }
    }

    qDebug() << correlations[0][8];

    for (int i = 0; i < filteredRects.size(); i++){
        int max = 0;
        int maxIndex = -1;

        for (int j = 0; j < TEMPLATES_COUNT; j++){
            if (correlations[i][j] > max){
                max = correlations[i][j];
                maxIndex = j;
            }
        }

        if (maxIndex != -1){
            output->addData(filteredRects[i].x, filteredRects[i].y, maxIndex);
        }
    }


   //_____________________________________________________________________

    int resultWidth =  finalImage.cols - tem->getWidth() + 1;
    int resultHeight = finalImage.rows - tem->getHeigth() + 1;

    /*cv::Mat results[TEMPLATES_COUNT];
    double minVals[TEMPLATES_COUNT];
    double maxVals[TEMPLATES_COUNT];
    cv::Point matchLocs[TEMPLATES_COUNT];

    for (int i = 0; i < TEMPLATES_COUNT; i++){

        results[i].create(resultHeight, resultWidth, CV_32FC1);

        matchTemplate(finalImage, tem->getTemplate(i), results[i], method);
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
                    results[i].at<uchar>(matchLocs[i].x, matchLocs[i].y) = 0;

                    cv::Point minLoc;
                    cv::Point maxLoc;
                    minMaxLoc(results[i], &minVals[i], &maxVals[i], &minLoc, &maxLoc, cv::Mat());
                    j = 0; // restart the search
                }
            }
        }

        if (willBeDisplayed){
            output->addData(matchLocs[i].x, matchLocs[i].y, i);
        }
    }*/

    return output;
}

Output* basicTemplateMatching(QString imagePath, Template* tem, QString backgroundPath){
    return templateMatching(imagePath, tem, CV_TM_CCORR_NORMED, "basicTemplateMatching", backgroundPath);
}

int colorDistance(cv::Vec3b c1, cv::Vec3b c2){
    int diff0 = c1[0] - c2[0];
    int diff1 = c1[1] - c2[1];
    int diff2 = c1[2] - c2[2];
    return (int) sqrt((diff0 * diff0) + (diff1 * diff1) + (diff2 * diff2));
}

cv::Mat closeGaps(cv::Mat image, int patchSize, double ratio){
    cv::Mat result = image.clone();

    for (int x = 0; x < result.cols; x++){
        for (int y = 0; y < result.rows; y++){
            if (image.at<uchar>(y, x) == 0){
                int whiteCount = 0;

                for (int i = -patchSize; i <= patchSize; i++){
                    for (int j = -patchSize; j <= patchSize; j++){
                        int posx = i+x;
                        int posy = j+y;

                        if (posx >= 0 && posx < image.cols && posy >= 0 && posy < image.rows){
                            if (image.at<uchar>(posy, posx) == 255){
                                whiteCount ++;
                            }
                        }
                    }
                }

                if (whiteCount > (2*patchSize+1)*(2*patchSize+1)*ratio){
                    result.at<uchar>(y, x) = 255;
                }
                else {
                    result.at<uchar>(y, x) = 0;
                }
            }
            else {
                result.at<uchar>(y, x) = 255;
            }
        }
    }

    return result;
}

cv::Mat extractBackgroundFromVideo(QString fileName, int maxFrames){
    cv::VideoCapture video(fileName.toStdString());
    cv::Mat background;
    if (video.isOpened()){
        cv::Mat frame;
        cv::Mat foreground;

        cv::Ptr<cv::BackgroundSubtractor> mog;
        mog = cv::createBackgroundSubtractorMOG2();

        int frameCount = 0;

        while(video.read(frame) && frameCount < maxFrames){
            mog->apply(frame, foreground, 0.05);
            //threshold(foreground, foreground ,120,255, cv::THRESH_BINARY_INV);
            imshow(fileName.toStdString(), foreground);

            frameCount ++;
        }
        mog->getBackgroundImage(background);

    }
    else {
        qDebug() << "Could not open video";
    }

    return background;
}

cv::Mat extractBackgroundFromFiles(QStringList filesName){
    cv::Mat* images;
    images = new cv::Mat[filesName.size()];

    for (int i = 0; i < filesName.size(); i++){
        images[i] = cv::imread(filesName[i].toStdString());

    }

    cv::Mat background;
    cv::Mat foreground;

    cv::Ptr<cv::BackgroundSubtractor> mog;
    mog = cv::createBackgroundSubtractorMOG2();

    for (int i = 0; i < filesName.size(); i++){
        mog->apply(images[i], foreground, 0.25);
    }

    mog->getBackgroundImage(background);
    return background;
}
