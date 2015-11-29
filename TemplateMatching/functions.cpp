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
    cvtColor(image, imageHue, CV_BGR2HSV);
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

    int dilateSize = 2;
    cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*dilateSize + 1, 2*dilateSize + 1), cv::Point(dilateSize, dilateSize) );
    cv::dilate(backgroundMask1, backgroundMask1, dilateElement);

    /*int erodeSize = 5;
    cv::Mat erodeElement = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*erodeSize + 1, 2*erodeSize + 1), cv::Point(erodeSize, erodeSize) );
    cv::erode(backgroundMask1, backgroundMask1, erodeElement);*/

    cv::Mat backgroundMask2 = backgroundMask1;

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

                if (intensity[0] > 15 && intensity[0] < 95 && intensity[1] > 55 && intensity[1] < 125 && intensity[2] < 100){
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
    cv::Mat shirtMask2;
    int dilateSizeShirt = 1;
    cv::Mat dilateElementShirt = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*dilateSizeShirt + 1, 2*dilateSizeShirt + 1), cv::Point(dilateSizeShirt, dilateSizeShirt));
    cv::dilate(shirtMask, shirtMask2, dilateElementShirt);

    // GET SHIRT CONNECTED COMPONENTS

    std::vector<std::vector<cv::Point> > shirtContours;
    std::vector<cv::Vec4i> shirtHierarchy;

    findContours(shirtMask2.clone(), shirtContours, shirtHierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    QList<cv::Rect> shirtFilteredRects;
    QList<std::vector<cv::Point> > shirtFilteredContours;

    for (unsigned int i = 0; i < shirtContours.size(); i++){
        cv::Rect rect = minAreaRect(shirtContours[i]).boundingRect();
        if (rect.width > 40 && rect.height > 50 && rect.width < 130 && rect.height < 200){
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

    cv::Mat shirtFilteredImage = shirtMask2.clone();

    for (int x = 0; x < shirtFilteredImage.cols; x++){
        for (int y = 0; y < shirtFilteredImage.rows; y++){
            if (shirtFilteredImage.at<uchar>(y, x) == 255){
                shirtFilteredImage.at<uchar>(y, x) = 0;
                for (int i = 0; i < shirtFilteredContours.size(); i++){
                    if (pointPolygonTest(shirtFilteredContours[i], cv::Point2f(x, y), true) >= 5 && y > shirtFilteredRects[i].y + shirtFilteredRects[i].height * 0.2 && y < shirtFilteredRects[i].y + shirtFilteredRects[i].height * 0.65){
                        shirtFilteredImage.at<uchar>(y, x) = 255;
                    }
                }
            }
        }
    }


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
                cv::Vec3b bgr = image.at<cv::Vec3b>(y, x);

                if ((intensity[0] < 25 && intensity[1] > 40 && intensity[1] < 110 && intensity[2] > 80 && (bgr[2] - bgr[1]) < 40) || (intensity[0] < 25 && intensity[1] < 90 && intensity[2] > 80)){
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
                    if (pointPolygonTest(shirtFilteredContours[i], cv::Point2f(x,y), true) >= 5 && y > shirtFilteredRects[i].y + shirtFilteredRects[i].height * 0.2 && y < shirtFilteredRects[i].y + shirtFilteredRects[i].height * 0.65 ){
                        grayScaleImage3.at<uchar>(y, x) = 255;
                    }
                }
            }
        }
    }

    cv::Mat grayScaleImage4 = closeGaps(grayScaleImage3, 1, 0.6);

    cv::Mat grayScaleImage5 = grayScaleImage4.clone();

    /*for (int x = 0; x < grayScaleImage5.cols; x++){
        for (int y = 0; y < grayScaleImage5.rows; y++){
            if (grayScaleImage5.at<uchar>(y, x) == 255 && sobelImage.at<uchar>(y, x) == 255){
                grayScaleImage5.at<uchar>(y, x) = 0;
            }
        }
    }

    grayScaleImage5 = closeGaps(grayScaleImage5, 2, 0.8);*/

    //GaussianBlur(grayScaleImage, grayScaleImage, cv::Size(7, 7), 0, 0);

    // GET CONNECTED COMPONENTS

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours(grayScaleImage5.clone(), contours, hierarchy, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_NONE);

    QList<cv::Rect> filteredRects;
    QList<std::vector<cv::Point> > filteredContours;
    QList<cv::Mat> possibleNumbers;

    for (unsigned int i = 0; i < contours.size(); i++){
        cv::Rect rect = minAreaRect(contours[i]).boundingRect();
        double ratio = (double) rect.width / rect.height;
        if (rect.width > 10 && rect.height > 15 && rect.width < 40 && rect.height < 40  && ratio > 0.5 && ratio < 1.5){
            if (rect.x < 0){
                rect.x = 0;
            }
            if (rect.y < 0){
                rect.y = 0;
            }

            if (rect.x + rect.width > grayScaleImage5.cols){
                rect.x = grayScaleImage5.cols - rect.width;
            }
            if (rect.y + rect.height > grayScaleImage5.rows){
                rect.y = grayScaleImage5.rows - rect.height;
            }
            cv::Mat temp = grayScaleImage5(rect);
            cv::Scalar tempVal = mean(temp);
            uchar meanVal = tempVal.val[0];

            if (meanVal > 40 && meanVal < 190){
                possibleNumbers.push_back(temp);

                filteredRects.push_back(rect);
                filteredContours.push_back(contours[i]);
            }
        }
    }

    // just to display the image

    cv::Mat grayScaleFilteredComponentsImage = grayScaleImage5.clone();

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


    // GET THE ACTUAL NUMBER


    cv::Size temSize(tem->getWidth(), tem->getHeigth());
    QList<cv::Mat> skeletons;
    QList<QList<int> > listPossibleNumbers;
    cv::Mat finalImage = grayScaleFilteredComponentsImage;

    Output* output = new Output(finalImage, tem);
    QMap<long, double> correlations; // first, connected component, then template, then rotation


    for (int i = 0; i < filteredRects.size(); i++){
        //qDebug() << filteredRects[i].width << "  -  " << filteredRects[i].height << "  -  " << filteredRects[i].x << "  -  " << filteredRects[i].y;

        cv::Mat temp = grayScaleImage5(filteredRects[i]).clone();

        for (int x = 0; x < temp.cols; x++){
            for (int y = 0; y < temp.rows; y++){
                if (temp.at<uchar>(y, x) == 255){
                    if (pointPolygonTest(filteredContours[i], cv::Point2f(x + filteredRects[i].x, y + filteredRects[i].y), true) < 0){
                        temp.at<uchar>(y, x) = 0;
                    }
                }
            }
        }

        // rotate the contour
        cv::RotatedRect rect = minAreaRect(filteredContours[i]);
        rect.center.x = temp.cols/2;
        rect.center.y = temp.rows/2;

        float angle = rect.angle;

        cv::Size rectSize = rect.size;
        if (rect.angle <= -35) {
            angle += 90.0;
            cv::swap(rectSize.width, rectSize.height);
        }
        cv::Mat rotationMatrix = cv::getRotationMatrix2D(rect.center, angle, 1.0);
        cv::Mat rotated, cropped;
        cv::warpAffine(temp, rotated, rotationMatrix, temp.size(), cv::INTER_CUBIC);
        cv::getRectSubPix(rotated, rectSize, rect.center, cropped);

        cv::resize(cropped, possibleNumbers[i], temSize);

        threshold(possibleNumbers[i], possibleNumbers[i], 127, 255, cv::THRESH_BINARY);

        skeletons.push_back(thinningGuoHall(possibleNumbers[i]));
        cv::imwrite(("temp/" + QString::number(i) + ".png").toStdString(), skeletons[skeletons.size()-1]);

        Skeleton ske(skeletons[skeletons.size()-1], possibleNumbers[i]);
        listPossibleNumbers.push_back(ske.possibleNumbers());

        //qDebug() << ske.listJunctions.size();

        //qDebug() << ske.listHoles.size();

        for (int j = 0; j < TEMPLATES_COUNT; j++){
            int index = 0;
            for (int angle = -ROTATION_MAX; angle <= ROTATION_MAX; angle+= ROTATION_STEP){
                correlations.insert(i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index, 1.0);
                index++;
            }
        }
    }


    for (int i = 0; i < filteredRects.size(); i++){
        if (listPossibleNumbers[i].size() == 1){
            correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + 0 * ROTATION_COUNT + listPossibleNumbers[i][0]] = 100.0;
        }
    }

    for (int i = 0; i < filteredRects.size(); i++){
        if (listPossibleNumbers[i].size() > 1){
            int index = 0;
            for (int angle = -ROTATION_MAX; angle <= ROTATION_MAX; angle+= ROTATION_STEP){


                cv::Mat rotatedImage;

                if (modules[CENTER_MASS] > 0 || modules[HALVES_CENTER_MASS_VERTI] > 0 || modules[HALVES_CENTER_MASS_HORI] > 0){
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
    }

    for (int i = 0; i < filteredRects.size(); i++){
        if (listPossibleNumbers[i].size() > 1){
            for (int j = 0; j < TEMPLATES_COUNT; j++){
                if (listPossibleNumbers[i].contains(j)) {
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
        }
    }

    for (int i = 0; i < filteredRects.size(); i++){
        int index = 0;
        for (int angle = -ROTATION_MAX; angle <= ROTATION_MAX; angle+= ROTATION_STEP){
            cv::Point2f center(possibleNumbers[i].cols/2.0F, possibleNumbers[i].rows/2.0F);
            cv::Mat rotMat = getRotationMatrix2D(center, angle, 1.0);
            cv::Mat rotatedImage;
            cv::warpAffine(possibleNumbers[i], rotatedImage, rotMat, possibleNumbers[i].size());

            Histogram hori(rotatedImage.cols);
            Histogram verti(rotatedImage.rows);

            for (int x = 0; x < rotatedImage.cols; x++){
                for (int y = 0; y < rotatedImage.rows; y++){

                    hori.add(x, rotatedImage.at<uchar>(y, x)/255.0);
                    verti.add(y, rotatedImage.at<uchar>(y, x)/255.0);
                }
            }

            for (int j = 0; j < TEMPLATES_COUNT; j++){
                if (listPossibleNumbers[i].contains(j)) {
                    correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index] += modules[HISTOGRAMS] * tem->getHistoHori(j)->compare(hori);
                    correlations[i * TEMPLATES_COUNT * ROTATION_COUNT + j * ROTATION_COUNT + index] += modules[HISTOGRAMS] * tem->getHistoVerti(j)->compare(verti);
                }
            }

            index++;
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
    modules[CENTER_MASS] = 0;
    modules[HALVES_CENTER_MASS_VERTI] = 0;
    modules[HALVES_CENTER_MASS_HORI] = 0;
    modules[HISTOGRAMS] = 3000;
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

cv::Mat getSkeleton(cv::Mat image){
    cv::Mat img = image.clone();
    threshold(img, img, 127, 255, cv::THRESH_BINARY);
    cv::Mat skel(img.size(), CV_8UC1, cv::Scalar(0));
    cv::Mat temp;
    cv::Mat eroded;

    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

    bool done;
    do {
      erode(img, eroded, element);
      cv::dilate(eroded, temp, element); // temp = open(img)
      cv::subtract(img, temp, temp);
      cv::bitwise_or(skel, temp, skel);
      eroded.copyTo(img);

      done = (cv::countNonZero(img) == 0);
    } while (!done);

    return skel;
}

// TEMPORARY FUNCTIONS

void thinningGuoHallIteration(cv::Mat& im, int iter) {
    cv::Mat marker = cv::Mat::zeros(im.size(), CV_8UC1);

    for (int i = 1; i < im.rows; i++){
        for (int j = 1; j < im.cols; j++){
            uchar p2 = im.at<uchar>(i-1, j);
            uchar p3 = im.at<uchar>(i-1, j+1);
            uchar p4 = im.at<uchar>(i, j+1);
            uchar p5 = im.at<uchar>(i+1, j+1);
            uchar p6 = im.at<uchar>(i+1, j);
            uchar p7 = im.at<uchar>(i+1, j-1);
            uchar p8 = im.at<uchar>(i, j-1);
            uchar p9 = im.at<uchar>(i-1, j-1);

            int C  = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) + (!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
            int N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
            int N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
            int N  = N1 < N2 ? N1 : N2;
            int m  = iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

            if (C == 1 && (N >= 2 && N <= 3) && m == 0){
                marker.at<uchar>(i,j) = 1;
            }

        }
    }

    im &= ~marker;
}

cv::Mat thinningGuoHall(cv::Mat image){
    cv::Mat im = image.clone();
    for (int i = 0; i < im.rows; i++){
        for (int j = 0; j < im.cols; j++){
            if (i == 0 || j == 0 || i == im.rows-1 || j == im.cols-1){
                im.at<uchar>(i,j) = 0;
            }
        }
    }
    im /= 255;

    cv::Mat prev = cv::Mat::zeros(im.size(), CV_8UC1);
    cv::Mat diff;

    do {
        thinningGuoHallIteration(im, 0);
        thinningGuoHallIteration(im, 1);
        cv::absdiff(im, prev, diff);
        im.copyTo(prev);
    }
    while (cv::countNonZero(diff) > 0);

    im *= 255;
    return im;
}
