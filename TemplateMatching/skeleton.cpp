#include "skeleton.h"
#include <QDebug>

Skeleton::Skeleton(cv::Mat skeletonizedImage, cv::Mat normalImage){

    for (int x = 0; x < skeletonizedImage.cols; x++){
        for (int y = 0; y < skeletonizedImage.rows; y++){
            if (skeletonizedImage.at<uchar>(y, x) == 255){
                int count = 0;
                for (int i = -1; i <= 1; i++){
                    for (int j = -1; j <= 1; j++){

                        if (y+j >= 0 && y+j < skeletonizedImage.rows && x+i >= 0 && x+i < skeletonizedImage.cols && skeletonizedImage.at<uchar>(y+j, x+i) == 255){
                            count++;
                        }
                    }
                }

                if (count == 2){
                    cv::Point2i point(x,y);
                    listLineEnds.push_back(point);
                }
                else if (count > 3){
                    cv::Point2i point(x,y);
                    listJunctions.push_back(point);
                }
            }
        }
    }

    // MERGING CLOSE JUNCTIONS
    bool done = true;

    do {
        done = true;
        int keepIndexJunction = -1;
        int removeIndexJunction = -1;
        for (int i = 0; i < listJunctions.size(); i++){
            for (int j = 0; j < listJunctions.size(); j++){
                if (i != j && norm(listJunctions[i] - listJunctions[j]) < 10){
                    keepIndexJunction = i;
                    removeIndexJunction = j;
                }
            }
        }

        if (keepIndexJunction != -1 && removeIndexJunction != -1){
            done = false;
            listJunctions[keepIndexJunction].x = (listJunctions[keepIndexJunction].x + listJunctions[removeIndexJunction].x) / 2;
            listJunctions[keepIndexJunction].y = (listJunctions[keepIndexJunction].y + listJunctions[removeIndexJunction].y) / 2;
            listJunctions.removeAt(removeIndexJunction);
        }
    } while (!done);


    // DELETING CLOSE JUNCTIONS AND LINE ENDS, ALWAYS WRONG DATA
    done = true;

    do {
        done = true;
        int removeIndexLineEnds = -1;
        int removeIndexJunctions = -1;
        for (int i = 0; i < listLineEnds.size(); i++){
            for (int j = 0; j < listJunctions.size(); j++){
                if (norm(listLineEnds[i] - listJunctions[j]) < 10){
                    removeIndexLineEnds = i;
                    removeIndexJunctions = j;
                }
            }
        }

        if (removeIndexLineEnds != -1 && removeIndexJunctions != -1){
            done = false;
            listLineEnds.removeAt(removeIndexLineEnds);
            listJunctions.removeAt(removeIndexJunctions);
        }
    } while (!done);


    // LOOPS

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    findContours(normalImage.clone(), contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    cv::Mat invertedImage = normalImage.clone();

    if (contours.size() < 1){
        qDebug() << "Error, 0 connected components detected";
    }
    /*else if (contours.size() > 1){
        qDebug() << "Error, multiple connected components detected : "  << contours.size();

        cv::Rect rect1 = minAreaRect(contours[0]).boundingRect();
        qDebug() << rect1.width << " " << rect1.height;

        cv::Rect rect2 = minAreaRect(contours[1]).boundingRect();
        qDebug() << rect2.width << " " << rect2.height;
    }
    */else {
        for (int x = 0; x < normalImage.cols; x++){
            for (int y = 0; y < normalImage.rows; y++){
                if (normalImage.at<uchar>(y, x) == 255){
                    invertedImage.at<uchar>(y, x) = 0;
                }
                else if (pointPolygonTest(contours[0], cv::Point2f(x,y), true) >= 0){
                    invertedImage.at<uchar>(y, x) = 255;
                }
                else {
                    invertedImage.at<uchar>(y, x) = 0;
                }
            }
        }
    }

    std::vector<std::vector<cv::Point> > invertedContours;
    std::vector<cv::Vec4i> invertedHierarchy;

    findContours(invertedImage, invertedContours, invertedHierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    for (int i = 0; i < invertedContours.size(); i++){
        cv::Rect rect = minAreaRect(invertedContours[i]).boundingRect();
        cv::Point2i point(rect.x + rect.width/2, rect.y + rect.height/2);
        listHoles.push_back(point);
    }



}

