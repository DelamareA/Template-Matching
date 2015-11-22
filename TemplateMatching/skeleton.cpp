#include "skeleton.h"
#include <QDebug>

Skeleton::Skeleton(cv::Mat skeletonizedImage, cv::Mat normalImage){

    QList<LabeledPoint> startList;
    QList<cv::Point2i> dummyJunctionList;
    QList<LabeledPoint> removedPoints;
    QList<int> survivors;
    int currentLabel = 1;

    // SEARCH FOR ALL CURRENT BRANCHES

    for (int x = 0; x < skeletonizedImage.cols; x++){
        for (int y = 0; y < skeletonizedImage.rows; y++){
            if (skeletonizedImage.at<uchar>(y, x) == 255){
                int count = 0;
                QVector<cv::Point2i> list;
                for (int i = -1; i <= 1; i++){
                    for (int j = -1; j <= 1; j++){
                        if (i != 0 || j != 0){
                            if (y+j >= 0 && y+j < skeletonizedImage.rows && x+i >= 0 && x+i < skeletonizedImage.cols && skeletonizedImage.at<uchar>(y+j, x+i) > 0){
                                bool neighbourg = false;
                                cv::Point2i point(x+i,y+j);

                                for (int k = 0; k < list.size(); k++){
                                    if (cv::norm(point - list[k]) == 1){
                                        neighbourg = true;
                                    }
                                }

                                if (!neighbourg){
                                    count++;
                                    list.append(point);
                                }
                            }
                        }
                    }
                }

                if (count == 1){
                    cv::Point2i point(x,y);
                    LabeledPoint lp;
                    lp.label = currentLabel;
                    lp.point = point;
                    startList.append(lp);

                    currentLabel++;
                }
                else if (count > 2){
                    cv::Point2i point(x,y);
                    dummyJunctionList.append(point);
                }
            }
        }
    }

    // GO THROUGH THE BRANCH AND REMOVE ONE PIXEL AT A TIME

    for (int i = 0; i < startList.size(); i++){
        cv::Point2i current = startList[i].point;
        int label = startList[i].label;

        int round = 0;
        bool done = false;

        do {
            LabeledPoint lp;
            lp.point = current;
            lp.label = label;
            removedPoints.append(lp);

            skeletonizedImage.at<uchar>(current.y, current.x) = 0;

            int count = 0;
            int x = current.x;
            int y = current.y;
            QVector<cv::Point2i> list;
            for (int i = -1; i <= 1; i++){
                for (int j = -1; j <= 1; j++){
                    if (i != 0 || j != 0){
                        if (y+j >= 0 && y+j < skeletonizedImage.rows && x+i >= 0 && x+i < skeletonizedImage.cols && skeletonizedImage.at<uchar>(y+j, x+i) == 255){
                            bool junction = false;
                            cv::Point2i point(x+i,y+j);

                            for (int k = 0; k < dummyJunctionList.size(); k++){
                                if (cv::norm(point - dummyJunctionList[k]) == 0){
                                    junction = true;
                                }
                            }

                            if (!junction){
                                count++;
                                list.append(point);
                            }
                        }
                    }
                }
            }

            if (count >= 1){
                current = list[0];
            }

            round ++;
            done = (count < 1);

            if (round == 12){
                survivors.append(label);
            }

        } while(round < 12 && !done);
    }

    // IF THE BRANCH IS STILL LONG ENOUGH, REDRAW IT

    for (int i = 0; i < removedPoints.size(); i++){
        cv::Point2i point = removedPoints[i].point;
        int label = removedPoints[i].label;

        if (survivors.contains(label)){
            skeletonizedImage.at<uchar>(point) = 255;
        }
    }

    // COUNTING JUNCTIONS AND LINE ENDS


    for (int x = 0; x < skeletonizedImage.cols; x++){
        for (int y = 0; y < skeletonizedImage.rows; y++){
            if (skeletonizedImage.at<uchar>(y, x) == 255){
                int count = 0;
                QVector<cv::Point2i> list;
                for (int i = -1; i <= 1; i++){
                    for (int j = -1; j <= 1; j++){
                        if (i != 0 || j != 0){
                            if (y+j >= 0 && y+j < skeletonizedImage.rows && x+i >= 0 && x+i < skeletonizedImage.cols && skeletonizedImage.at<uchar>(y+j, x+i) == 255){
                                bool neighbourg = false;
                                cv::Point2i point(x+i,y+j);

                                for (int k = 0; k < list.size(); k++){
                                    if (cv::norm(point - list[k]) == 1){
                                        neighbourg = true;
                                    }
                                }

                                if (!neighbourg){
                                    count++;
                                    list.append(point);
                                }
                            }
                        }
                    }
                }

                if (count == 1 || count > 2){
                    double xNorm = ((double) x) / skeletonizedImage.cols;
                    double yNorm = ((double) y) / skeletonizedImage.rows;
                    cv::Point2d point(xNorm,yNorm);

                    if (count == 1){
                        listLineEnds.push_back(point);
                    }
                    else {
                        listJunctions.push_back(point);
                    }
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
                if (i != j && norm(listJunctions[i] - listJunctions[j]) < MERGE_DISTANCE){
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
                if (norm(listLineEnds[i] - listJunctions[j]) < DELETE_DISTANCE){
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

    findContours(normalImage.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

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
    }*/
    else {
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

    for (unsigned int i = 0; i < invertedContours.size(); i++){
        cv::Rect rect = minAreaRect(invertedContours[i]).boundingRect();

        double xNorm = ((double) rect.x + rect.width/2) / skeletonizedImage.cols;
        double yNorm = ((double) rect.y + rect.height/2) / skeletonizedImage.rows;

        cv::Point2d point(xNorm, yNorm);
        listHoles.push_back(point);
    }

    listLineEnds = sort(listLineEnds);
    listHoles = sort(listHoles);
    listJunctions = sort(listJunctions);

}

QList<int> Skeleton::possibleNumbers(){
    QList<int> list;

    if (listHoles.size() > 2 || listJunctions.size() > 5 || listLineEnds.size() > 5){
        // certainly a bad 'catch'
    }
    if (listHoles.size() == 0){
        if (listLineEnds.size() >= 2){
            if (listLineEnds[0].x < 0.3 && listLineEnds[0].y < 0.3 && listLineEnds[1].x > 0.7 && listLineEnds[1].y > 0.7){
                list.push_back(2);
            }
            else if (listLineEnds[0].x > 0.7 && listLineEnds[0].y < 0.3 && listLineEnds[1].y > 0.5){
                list.push_back(5);
            }
            else if (listLineEnds[1].x < 0.4){
                list.push_back(7);
            }
            else {
                list.push_back(1);
            }
        }
        else {
            list.push_back(1);
            list.push_back(2);
            list.push_back(5);
            list.push_back(7);
        }
    }
    else if (listHoles.size() == 1){
        if (listHoles[0].x > 0.35 && listHoles[0].x < 0.65 && listHoles[0].y > 0.35 && listHoles[0].y < 0.65){
            if (listJunctions.size() + listLineEnds.size() < 3){
                list.push_back(0);
            }
            else {
                list.push_back(4);
            }
        }
        else if (listHoles.size() == 1 && listJunctions.size() >= 1 && listLineEnds.size() >= 1){
            if (listHoles[0].y > listLineEnds[0].y){
                list.push_back(6);
            }
            else {
                list.push_back(9);
            }
        }
        else {
            list.push_back(0);
            list.push_back(4);
            list.push_back(6);
            list.push_back(9);
        }
    }
    else if (listHoles.size() == 2){
        list.push_back(8);
    }
    else { // if the 'number' is close to a 'normal' number, but not a perfect match, we try all of them
        for (int i = 0; i < TEMPLATES_COUNT; i++){
            list.push_back(i);
        }
    }

    return list;
}

QList<double> Skeleton::vectorization() {
    // 0 -> num ends
    // 1 -> end 1 x
    // 2 -> end 1 y
    // 3 -> end 2 x
    // 4 -> end 2 y
    // 5 -> end 3 x
    // 6 -> end 3 y
    // 7 -> num junc
    // 8 -> junc 1 x
    // 9 -> junc 1 y
    // 10 -> junc 2 x
    // 11 -> junc 2 y

    QList<double> result;

    for (int i = 0; i < VECTOR_DIMENSION; i++){
        result.push_back(0.0001);
    }

    result[0] = (min(listLineEnds.size() / 3.0, 1.0));

    for (int i = 0; i < listLineEnds.size() && i < 3; i++){
        result[1+2*i] = (listLineEnds[i].x);
        result[2+2*i] = (listLineEnds[i].y);
    }

    result[7] = (min(listJunctions.size() / 2.0, 1.0));

    for (int i = 0; i < listJunctions.size() && i < 2; i++){
        result[8+2*i] = (listJunctions[i].x);
        result[9+2*i] = (listJunctions[i].y);
    }

    return result;
}

double Skeleton::min(double a, double b){
    return a < b ? a : b;
}

QList<cv::Point2d> Skeleton::sort(QList<cv::Point2d> list){
    QList<cv::Point2d> result;

    for (int i = 0; i < list.count(); i++){
        double min = 1.1;
        int index = -1;
        for (int j = 0; j < list.count(); j++){
            if (list[j].y < min){
                min = list[j].y;
                index = j;
            }

        }

        if (index != -1){
            cv::Point2d point(list[index].x, list[index].y);
            result.push_back(point);
            list[index].y = 1.1;
        }
        else {
            qDebug() << "Error in sort, some value must be bigger than 1.0";
        }
    }

    return result;
}
