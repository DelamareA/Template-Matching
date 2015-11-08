#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include "functions.h"
#include "configuration.h"
#include <vector>

Output* templateMatching(cv::Mat image, Template* tem, int modules[MODULES_COUNT], cv::Mat background){
    cv::Mat imageTransformed = image;
    cv::Mat imageHue;
    cv::Mat backgroundMask1;
    cv::Mat grayScaleImage;
    cvtColor(image, grayScaleImage, CV_BGR2GRAY);
    cvtColor(image, backgroundMask1, CV_BGR2GRAY);
    cvtColor(image, imageHue, CV_BGR2HLS);
    //cvtColor(image, imageTransformed, CV_BGR2HSV);

    // EXTRACT BACKGROUND

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

    cv::Mat backgroundMask2 = closeGaps(backgroundMask1, 5, 0.3);

    //EXTRACT SHIRT COLORS

    cv::Mat shirtMask = backgroundMask2.clone();

    cv::Vec3b greenColor = Configuration::getGreenColor();
    int maxGreenColorDistance = Configuration::getMaxGreenColorDistance();
    cv::Vec3b redColor = Configuration::getRedColor();
    int maxRedColorDistance = Configuration::getMaxRedColorDistance();

    for (int x = 0; x < shirtMask.cols; x++){
        for (int y = 0; y < shirtMask.rows; y++){
            if (backgroundMask2.at<uchar>(y, x) == 0){
                shirtMask.at<uchar>(y, x) = 0;
            }
            else {
                cv::Vec3b intensity = imageHue.at<cv::Vec3b>(y, x);

                if (intensity[0] > 20 && intensity[0] < 75){
                    shirtMask.at<uchar>(y, x) = 255;
                }
                else{
                    if (intensity[0] < 10 || intensity[0] > 250){
                        shirtMask.at<uchar>(y, x) = 255;
                    }
                    else{
                        shirtMask.at<uchar>(y, x) = 0;
                    }
                }
            }
        }
    }


    cv::Mat shirtMask2 = closeGaps(shirtMask, 2, 0.8);

    // GET SHIRT CONNECTED COMPONENTS

    std::vector<std::vector<cv::Point> > shirtContours;
    std::vector<cv::Vec4i> shirtHierarchy;

    findContours(shirtMask2.clone(), shirtContours, shirtHierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

    QList<cv::Rect> shirtFilteredRects;
    QList<std::vector<cv::Point> > shirtFilteredContours;

    for (unsigned int i = 0; i < shirtContours.size(); i++){
        cv::Rect rect = minAreaRect(shirtContours[i]).boundingRect();
        if (rect.width > 20 && rect.height > 40){
            if (rect.x < 0){
                rect.x = 0;
            }
            if (rect.y < 0){
                rect.y = 0;
            }

            if (rect.x + rect.width > shirtMask2.cols){
                rect.x = shirtMask2.cols - rect.width;
            }
            if (rect.y + rect.height > shirtMask2.rows){
                rect.y = shirtMask2.rows - rect.height;
            }

            shirtFilteredRects.push_back(rect);
            shirtFilteredContours.push_back(shirtContours[i]);
        }
    }

    // just to display the image

    /*cv::Mat shirtFilteredImage = shirtMask2.clone();

    for (int x = 0; x < shirtFilteredImage.cols; x++){
        for (int y = 0; y < shirtFilteredImage.rows; y++){
            if (shirtFilteredImage.at<uchar>(y, x) == 255){
                shirtFilteredImage.at<uchar>(y, x) = 0;
                for (int i = 0; i < shirtFilteredContours.size(); i++){
                    if (pointPolygonTest(shirtFilteredContours[i], cv::Point2f(x,y), true) >= 0){
                        shirtFilteredImage.at<uchar>(y, x) = 255;
                    }
                }
            }
        }
    }*/




    // EXTRACT WHITE PIXELS (NUMBERS)
    cv::Vec3b numberColor = Configuration::getNumberColor();
    int maxNumberColorDistance = Configuration::getMaxNumberColorDistance();

    for (int x = 0; x < grayScaleImage.cols; x++){
        for (int y = 0; y < grayScaleImage.rows; y++){
            if (backgroundMask2.at<uchar>(y, x) == 0){
                grayScaleImage.at<uchar>(y, x) = 0;
            }
            else {
                cv::Vec3b intensity = imageHue.at<cv::Vec3b>(y, x);

                if (intensity[0] < 30 && intensity[1] > 70 && intensity[2] < 60){
                    grayScaleImage.at<uchar>(y, x) = 255;
                }
                else {
                    grayScaleImage.at<uchar>(y, x) = 0;
                }
            }
        }
    }



    cv::Mat grayScaleImage3 = grayScaleImage.clone();

    for (int x = 0; x < grayScaleImage3.cols; x++){
        for (int y = 0; y < grayScaleImage3.rows; y++){
            if (grayScaleImage3.at<uchar>(y, x) == 255){
                grayScaleImage3.at<uchar>(y, x) = 0;
                for (int i = 0; i < shirtFilteredContours.size(); i++){
                    if (pointPolygonTest(shirtFilteredContours[i], cv::Point2f(x,y), true) >= 0){
                        grayScaleImage3.at<uchar>(y, x) = 255;
                    }
                }
            }
        }
    }

    cv::Mat grayScaleImage4 = closeGaps(grayScaleImage3, 1, 0.3);

    //GaussianBlur(grayScaleImage, grayScaleImage, cv::Size(7, 7), 0, 0);

    // GET CONNECTED COMPONENTS

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours(grayScaleImage4.clone(), contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

    QList<cv::Rect> filteredRects;
    QList<std::vector<cv::Point> > filteredContours;
    QList<cv::Mat> possibleNumbers;

    for (unsigned int i = 0; i < contours.size(); i++){
        cv::Rect rect = minAreaRect(contours[i]).boundingRect();
        double ratio = (double) rect.width / rect.height;
        if (rect.width > 15 && rect.height > 15 && rect.width < 50 && rect.height < 50  && ratio > 0.5 && ratio < 1.3){
            if (rect.x < 0){
                rect.x = 0;
            }
            if (rect.y < 0){
                rect.y = 0;
            }

            if (rect.x + rect.width > grayScaleImage4.cols){
                rect.x = grayScaleImage4.cols - rect.width;
            }
            if (rect.y + rect.height > grayScaleImage4.rows){
                rect.y = grayScaleImage4.rows - rect.height;
            }

            cv::Mat temp = grayScaleImage4(rect);

            cv::Scalar tempVal = mean(temp);
            uchar meanVal = tempVal.val[0];

            if (meanVal > 60){
                possibleNumbers.push_back(temp);

                filteredRects.push_back(rect);
                filteredContours.push_back(contours[i]);
            }
        }
    }

    // just to display the image

    /*cv::Mat grayScaleFilteredComponentsImage = grayScaleImage4.clone();

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
    }*/


    // GET THE ACTUAL NUMBER


    cv::Size temSize(tem->getWidth(), tem->getHeigth());

    cv::Mat finalImage = image;

    Output* output = new Output(finalImage, tem);
    QMap<long, double> correlations; // first, connected component, then template, then rotation


    for (int i = 0; i < filteredRects.size(); i++){
        //qDebug() << filteredRects[i].width << "  -  " << filteredRects[i].height << "  -  " << filteredRects[i].x << "  -  " << filteredRects[i].y;

        cv::Mat temp = grayScaleImage4(filteredRects[i]);

        resize(temp, possibleNumbers[i], temSize);


        for (int j = 0; j < TEMPLATES_COUNT; j++){
            int index = 0;
            for (int angle = -ROTATION_MAX; angle <= ROTATION_MAX; angle+= ROTATION_STEP){
                correlations.insert(i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index, 1.0);
                index++;
            }
        }
    }

    for (int i = 0; i < filteredRects.size(); i++){
        int index = 0;
        for (int angle = -ROTATION_MAX; angle <= ROTATION_MAX; angle+= ROTATION_STEP){


            cv::Mat rotatedImage;

            if (modules[CENTER_MASS] > 0 | modules[HALVES_CENTER_MASS_VERTI] > 0 | modules[HALVES_CENTER_MASS_HORI] > 0){
                cv::Point2f center(possibleNumbers[i].cols/2.0F, possibleNumbers[i].rows/2.0F);
                cv::Mat rotMat = getRotationMatrix2D(center, angle, 1.0);
                cv::warpAffine(possibleNumbers[i], rotatedImage, rotMat, possibleNumbers[i].size());
            }

            if (modules[CENTER_MASS] > 0){
                cv::Point2f massCenter = getMassCenterFromImage(rotatedImage);

                for (int j = 0; j < TEMPLATES_COUNT; j++){
                    double dist = cv::norm(massCenter - tem->getMassCenter(j));

                    correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index] += modules[CENTER_MASS] / (dist + 0.1);
                }
            }

            if (modules[HALVES_CENTER_MASS_VERTI] > 0){
                cv::Mat firstHalf = cv::Mat(rotatedImage.clone(), cv::Rect(0, 0, rotatedImage.cols/2, rotatedImage.rows));
                cv::Mat secondHalf = cv::Mat(rotatedImage.clone(), cv::Rect(rotatedImage.cols/2, 0, rotatedImage.cols/2, rotatedImage.rows));

                cv::Point2f massCenter1h = getMassCenterFromImage(firstHalf);
                cv::Point2f massCenter2h = getMassCenterFromImage(secondHalf);

                for (int j = 0; j < TEMPLATES_COUNT; j++){
                    double dist1h = cv::norm(massCenter1h - tem->getHalfMassCenter(0, j));
                    double dist2h = cv::norm(massCenter2h - tem->getHalfMassCenter(1, j));

                    correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index] += modules[HALVES_CENTER_MASS_VERTI] / (dist1h + dist2h + 0.1);
                }
            }

            if (modules[HALVES_CENTER_MASS_HORI] > 0){
                cv::Mat firstHalfHori = cv::Mat(rotatedImage.clone(), cv::Rect(0, 0, rotatedImage.cols, rotatedImage.rows/2));
                cv::Mat secondHalfHori = cv::Mat(rotatedImage.clone(), cv::Rect(0, rotatedImage.rows/2, rotatedImage.cols, rotatedImage.rows/2));

                cv::Point2f massCenter1hHori = getMassCenterFromImage(firstHalfHori);
                cv::Point2f massCenter2hHori = getMassCenterFromImage(secondHalfHori);

                for (int j = 0; j < TEMPLATES_COUNT; j++){
                    double dist1hHori = cv::norm(massCenter1hHori - tem->getHalfMassCenterHori(0, j));
                    double dist2hHori = cv::norm(massCenter2hHori - tem->getHalfMassCenterHori(1, j));

                    correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index] += modules[HALVES_CENTER_MASS_HORI] / (dist1hHori + dist2hHori + 0.1);
                }
            }

            index++;
        }
    }

    for (int i = 0; i < filteredRects.size(); i++){

        for (int j = 0; j < TEMPLATES_COUNT; j++){
            int index = 0;
            for (int angle = -ROTATION_MAX; angle <= ROTATION_MAX; angle+= ROTATION_STEP){

                cv::Point2f center(possibleNumbers[i].cols/2.0F, possibleNumbers[i].rows/2.0F);
                cv::Mat rotMat = getRotationMatrix2D(center, angle, 1.0);
                cv::Mat rotatedImage;
                cv::warpAffine(possibleNumbers[i], rotatedImage, rotMat, possibleNumbers[i].size());

                cv::Mat result1;
                result1.create(1, 1, CV_8UC1);
                cv::Mat result2;
                result2.create(1, 1, CV_8UC1);
                cv::Mat result3;
                result3.create(1, 1, CV_8UC1);

                matchTemplate(rotatedImage, tem->getTemplate(j), result1, CV_TM_CCOEFF_NORMED);
                matchTemplate(rotatedImage, tem->getTemplate(j), result2, CV_TM_CCORR_NORMED);
                matchTemplate(rotatedImage, tem->getTemplate(j), result3, CV_TM_SQDIFF_NORMED);

                correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index] += 7 * result1.at<uchar>(0,0);
                index++;


            }
        }
    }

    //qDebug() << correlations[0][5][3];

    for (int i = 0; i < filteredRects.size(); i++){
        double max = 0;
        int maxIndex = -1;

        for (int j = 0; j < TEMPLATES_COUNT; j++){
            int index = 0;
            for (int angle = -ROTATION_MAX; angle <= ROTATION_MAX; angle+= ROTATION_STEP){
                //qDebug() << i << " - " << j << " - " << index << " : " << i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index;
                if (correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index] > max){
                    max = correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index];
                    maxIndex = j;
                }
                index++;
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

Output* basicTemplateMatching(cv::Mat image, Template* tem, cv::Mat background){
    int modules[MODULES_COUNT];
    modules[TEMPLATE_MATCHING] = 7;
    modules[CENTER_MASS] = 3000;
    modules[HALVES_CENTER_MASS_VERTI] = 5000;
    modules[HALVES_CENTER_MASS_HORI] = 3000;
    return templateMatching(image, tem, modules, background);
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

cv::Mat extractBackgroundFromVideo2(QString fileName, int maxFrames){
    cv::VideoCapture video(fileName.toStdString());
    cv::Mat background;
    if (video.isOpened()){
        cv::Mat frame;

        int**** histo = new int***[1920];

        for (int i = 0; i < 1920; i++){
            histo[i] = new int**[1080];

            for (int j = 0; j < 1080; j++){
                histo[i][j] = new int*[3];

                for (int k = 0; k < 3; k++){
                    histo[i][j][k] = new int[63];

                    for (int l = 0; l < 63; l++){
                        histo[i][j][k][l] = 0;
                    }
                }
            }
        }

        video.read(frame);
        background = frame.clone();

        int frameCount = 0;

        while(video.read(frame) && frameCount < maxFrames){
            for (int i = 0; i < 1920; i++){
                for (int j = 0; j < 1080; j++){
                    cv::Vec3b intensity = frame.at<cv::Vec3b>(j, i);
                    uchar blue = intensity.val[0];
                    uchar green = intensity.val[1];
                    uchar red = intensity.val[2];

                    histo[i][j][0][blue/4]++;
                    histo[i][j][1][green/4]++;
                    histo[i][j][2][red/4]++;
                }
            }

            qDebug() << frameCount;

            frameCount ++;
        }

        for (int i = 0; i < 1920; i++){
            for (int j = 0; j < 1080; j++){

                cv::Vec3b color = background.at<cv::Vec3b>(j, i);

                for (int k = 0; k < 3; k++){

                    int max = 0;
                    int index = -1;
                    for (int l = 0; l < 63; l++){
                        if (histo[i][j][k][l] > max){
                            max = histo[i][j][k][l];
                            index = l;
                        }
                    }

                    if (index != -1){
                        color[k] = index*4;
                    }

                }

                background.at<cv::Vec3b>(j, i) = color;
            }
        }

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


cv::Point2f getMassCenterFromImage(cv::Mat image){
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours(image.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    if (contours.size() == 0){
        qDebug() << "Error, image in getMassCenter doesn't have a contour";
        return cv::Point(0, 0);
    }
    else {
        cv::Moments imageMoments = moments(contours[0], false);

        return cv::Point2f(imageMoments.m10/imageMoments.m00 , imageMoments.m01/imageMoments.m00);
    }
}
