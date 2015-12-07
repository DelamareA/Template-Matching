#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/background_segm.hpp>
#include "functions.h"
#include "configuration.h"
#include <vector>

Output* templateMatching(cv::Mat image, Template* tem, int modules[MODULES_COUNT], cv::Mat background, QList<int> digitsOnField){
    cv::Mat imageTransformed = image;
    cv::Mat blurredImage;
    cv::Mat imageHue;
    cv::Mat blurredImageHue;
    cv::Mat backgroundMask1;
    cv::Mat grayScaleImage;
    cv::Mat blurredBackground;
    GaussianBlur(background, blurredBackground, cv::Size(3, 3), 0, 0);
    GaussianBlur(image, blurredImage, cv::Size(3, 3), 0, 0);
    cvtColor(image, grayScaleImage, CV_BGR2GRAY);
    cvtColor(image, backgroundMask1, CV_BGR2GRAY);
    cvtColor(blurredImage, blurredImageHue, CV_BGR2HSV);
    cvtColor(image, imageHue, CV_BGR2HSV);


    // EXTRACT BACKGROUND

    int maxBackgroundColorDistance = Configuration::getMaxBackgroundColorDistance();

    for (int x = 0; x < backgroundMask1.cols; x++){
        for (int y = 0; y < backgroundMask1.rows; y++){
            cv::Vec3b intensity = blurredImage.at<cv::Vec3b>(y, x);
            cv::Vec3b backgroundPixel = blurredBackground.at<cv::Vec3b>(y, x);

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

    cv::Mat backgroundMask2 = backgroundMask1;

    std::vector<std::vector<cv::Point> > backgroundContours;
    std::vector<cv::Vec4i> backgroundHierarchy;

    findContours(backgroundMask1.clone(), backgroundContours, backgroundHierarchy, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_NONE);

    QList<cv::Rect> backgroundFilteredRects;
    QList<std::vector<cv::Point> > backgroundFilteredContours;

    for (unsigned int i = 0; i < backgroundContours.size(); i++){
        cv::Rect rect = minAreaRect(backgroundContours[i]).boundingRect();
        if (rect.width > 40 && rect.height > 50){

            rect.y += rect.height * 0.2;
            rect.height *= 0.3;

            if (rect.x < 0){
                rect.x = 0;
            }
            if (rect.y < 0){
                rect.y = 0;
            }

            if (rect.x + rect.width > backgroundMask1.cols){
                rect.x = backgroundMask1.cols - rect.width;
            }
            if (rect.y + rect.height > backgroundMask1.rows){
                rect.y = backgroundMask1.rows - rect.height;
            }

            backgroundFilteredRects.push_back(rect);
            backgroundFilteredContours.push_back(backgroundContours[i]);
        }
    }

    // just to display the image

    cv::Mat backgroundFilteredImage = backgroundMask2.clone();
    cv::Mat backgroundFilteredImageColor = image.clone();


    cv::Vec3b black;
    black[0] = 0;
    black[1] = 0;
    black[2] = 0;
    for (int x = 0; x < backgroundFilteredImage.cols; x++){
        for (int y = 0; y < backgroundFilteredImage.rows; y++){
            if (backgroundMask2.at<uchar>(y, x) == 255){
                backgroundFilteredImage.at<uchar>(y, x) = 0;
                backgroundFilteredImageColor.at<cv::Vec3b>(y,x) = black;
                for (int i = 0; i < backgroundFilteredContours.size(); i++){
                    if (pointPolygonTest(backgroundFilteredContours[i], cv::Point2f(x, y), true) >= 5 && y >= backgroundFilteredRects[i].y && y < backgroundFilteredRects[i].y + backgroundFilteredRects[i].height){
                        backgroundFilteredImage.at<uchar>(y, x) = 255;
                        backgroundFilteredImageColor.at<cv::Vec3b>(y,x) = image.at<cv::Vec3b>(y,x);
                    }
                }
            }
            else {
                backgroundFilteredImageColor.at<cv::Vec3b>(y,x) = black;
            }
        }
    }

    Output* output = new Output(backgroundFilteredImageColor, tem);

    return output;
}

Output* basicTemplateMatching(cv::Mat image, Template* tem, cv::Mat background, QList<int> digitsOnField){
    int modules[MODULES_COUNT];
    modules[TEMPLATE_MATCHING] = 7;
    modules[CENTER_MASS] = 0;
    modules[HALVES_CENTER_MASS_VERTI] = 0;
    modules[HALVES_CENTER_MASS_HORI] = 0;
    modules[HISTOGRAMS] = 3000;
    return templateMatching(image, tem, modules, background, digitsOnField);
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
