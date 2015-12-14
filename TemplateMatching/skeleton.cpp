#include "skeleton.h"
#include <QDebug>

Machines Skeleton::machines;

Skeleton::Skeleton(cv::Mat skeletonizedImage, cv::Mat normalImage){

    //QList<LabeledPoint> startList;
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
                                if (cv::norm(point - dummyJunctionList[k]) <= 1){
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

    // LOOPS

    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;

    cv::Mat imageClone = normalImage.clone();
    cv::Mat dilateElement = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1, 1));
    cv::dilate(imageClone, imageClone, dilateElement);

    findContours(imageClone.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    cv::Mat invertedImage = normalImage.clone();

    if (contours.size() < 1){
        qDebug() << "Error, 0 connected components detected";
    }
    else {
        for (int x = 0; x < normalImage.cols; x++){
            for (int y = 0; y < normalImage.rows; y++){
                if (imageClone.at<uchar>(y, x) == 255){
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


        if (rect.width > 2 && rect.height > 2){
            listHoles.push_back(point);
        }
        else {
            listFakeHoles.push_back(point);
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

    // DELETING JUNCTIONS CLOSE TO FAKE LOOPS

    done = true;

    do {
        done = true;
        int removeIndexJunction = -1;
        for (int i = 0; i < listJunctions.size(); i++){
            for (int j = 0; j < listFakeHoles.size(); j++){
                if (norm(listJunctions[i] - listFakeHoles[j]) < FAKE_LOOPS_DISTANCE){
                    removeIndexJunction = i;
                }
            }
        }

        if (removeIndexJunction != -1){
            done = false;
            listJunctions.removeAt(removeIndexJunction);
        }
    } while (!done);

    // DELETING LINE ENDS CLOSE TO FAKE LOOPS

    done = true;

    do {
        done = true;
        int removeIndexEnd = -1;
        for (int i = 0; i < listLineEnds.size(); i++){
            for (int j = 0; j < listFakeHoles.size(); j++){
                if (norm(listLineEnds[i] - listFakeHoles[j]) < FAKE_LOOPS_DISTANCE){
                    removeIndexEnd = i;
                }
            }
        }

        if (removeIndexEnd != -1){
            done = false;
            listLineEnds.removeAt(removeIndexEnd);
        }
    } while (!done);

    // DELETING JUNCTIONS CLOSE TO BORDERS, ALWAYS WRONG DATA
    done = true;

    do {
        done = true;
        int removeIndexJunctions = -1;
        for (int i = 0; i < listJunctions.size(); i++){
            if (listJunctions[i].x < JUNCTION_MARGIN || listJunctions[i].y < JUNCTION_MARGIN || listJunctions[i].x > 1 - JUNCTION_MARGIN || listJunctions[i].y > 1 - JUNCTION_MARGIN){
                removeIndexJunctions = i;
            }
        }

        if (removeIndexJunctions != -1){
            done = false;
            listJunctions.removeAt(removeIndexJunctions);
        }
    } while (!done);

    massCenter = getMassCenter(normalImage);

    total = getCount(normalImage);

    setParts(normalImage);

    listLineEnds = sort(listLineEnds);
    listHoles = sort(listHoles);
    listJunctions = sort(listJunctions);

}

QList<int> Skeleton::possibleNumbers(QList<int> digitsOnField){
    QList<int> list;

    /*if (listHoles.size() > 2 || listJunctions.size() > 5 || listLineEnds.size() > 5){
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
    }*/

    if (HOLE_SEPARATION){
        if (listHoles.size() == 0){
            QList<int> zeroHole;
            zeroHole.push_back(1);
            zeroHole.push_back(2);
            zeroHole.push_back(3);
            zeroHole.push_back(5);
            zeroHole.push_back(7);

            int dim = Skeleton::getDim(M0);

            QList<double> vect = vectorization(M0);

            float sampleData[dim];

            for (int i = 0; i < dim; i++){
                sampleData[i] = vect[i];
            }

            cv::Mat sampleMat(1, dim, CV_32FC1, sampleData);

            int maxVote = 0;
            int intMaxVote = -1;

            for (int i = 0; i < 10; i++){
                int vote = 0;
                for (int j = 0; j < 10; j++){
                    if (i != j && zeroHole.contains(i) && zeroHole.contains(j)){
                        float response = Skeleton::machines.m[min(i,j)][max(i,j)]->predict(sampleMat);

                        if ((response == 0.0 && i == min(i,j)) || (response == 1.0 && i == max(i,j))){
                            vote++;
                        }
                    }
                }

                //qDebug() << i << ": " << vote;

                if (vote > maxVote){
                    maxVote = vote;
                    intMaxVote = i;
                }
            }

            if (intMaxVote == -1){
                qDebug() << "Error : No vote is greater than 0";
            }
            else {
                list.push_back(intMaxVote);
            }
        }
        else if (listHoles.size() == 1){
            QList<int> oneHole;
            oneHole.push_back(0);
            oneHole.push_back(4);
            oneHole.push_back(6);
            oneHole.push_back(9);

            int dim = Skeleton::getDim(M1);

            QList<double> vect = vectorization(M1);

            float sampleData[dim];

            for (int i = 0; i < dim; i++){
                sampleData[i] = vect[i];
            }

            cv::Mat sampleMat(1, dim, CV_32FC1, sampleData);

            int maxVote = 0;
            int intMaxVote = -1;

            for (int i = 0; i < 10; i++){
                int vote = 0;
                for (int j = 0; j < 10; j++){
                    if (i != j && oneHole.contains(i) && oneHole.contains(j)){
                        float response = Skeleton::machines.m[min(i,j)][max(i,j)]->predict(sampleMat);

                        if ((response == 0.0 && i == min(i,j)) || (response == 1.0 && i == max(i,j))){
                            vote++;
                        }
                    }
                }

                //qDebug() << i << ": " << vote;

                if (vote > maxVote){
                    maxVote = vote;
                    intMaxVote = i;
                }
            }

            if (intMaxVote == -1){
                qDebug() << "Error : No vote is greater than 0";
            }
            else {
                list.push_back(intMaxVote);
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
    }
    else {


        int dim = Skeleton::getDim(M0);
        QList<double> vect = vectorization(M0);

        float sampleData[dim];

        for (int i = 0; i < dim; i++){
            sampleData[i] = vect[i];
        }

        cv::Mat sampleMat(1, dim, CV_32FC1, sampleData);

        int maxVote = 0;
        int intMaxVote = -1;

        for (int i = 0; i < 10; i++){
            int vote = 0;
            for (int j = 0; j < 10; j++){
                if (i != j && digitsOnField.contains(i) && digitsOnField.contains(j)) {
                    float response = Skeleton::machines.m[min(i,j)][max(i,j)]->predict(sampleMat);

                    if ((response == 0.0 && i == min(i,j)) || (response == 1.0 && i == max(i,j))){
                        vote++;
                    }
                }
            }

            //qDebug() << i << ": " << vote;

            if (vote > maxVote){
                maxVote = vote;
                intMaxVote = i;
            }
        }

        if (intMaxVote == -1){
            qDebug() << "Error : No vote is greater than 0";
        }
        else {
            list.push_back(intMaxVote);
        }
    }

    return list;
}

QList<double> Skeleton::vectorization(int type) {
    QList<double> result;

    int count = getDim(type);
    int endCount = getEndCount(type);
    int junctionCount = getJunctionCount(type);
    int holeCount = getHoleCount(type);
    int massCenterCount = getMassCenterCount(type);
    int totalCount = getTotalCount(type);
    int partCount = getPartCount(type);

    int index = 0;

    for (int i = 0; i < count; i++){
        result.push_back(0.0001);
    }



    for (int i = 0; i < listLineEnds.size() && i < endCount; i++){
        result[index] = (listLineEnds[i].x);
        index++;

        result[index] = (listLineEnds[i].y);
        index++;
    }

    for (int i = 0; i < listJunctions.size() && i < junctionCount; i++){
        result[index] = (listJunctions[i].x);
        index++;

        result[index] = (listJunctions[i].y);
        index++;
    }

    for (int i = 0; i < listHoles.size() && i < holeCount; i++){
        result[index] = (listHoles[i].x);
        index++;

        result[index] = (listHoles[i].y);
        index++;
    }

    for (int i = 0; i < massCenterCount; i++){
        result[index] = (massCenter.x);
        index++;

        result[index] = (massCenter.y);
        index++;
    }



    for (int i = 0; i < totalCount; i++){
        result[index] = (total);
        index++;
    }

    for (int i = 0; i < partCount; i++){
        for (int x = 0; x < PART_X; x++){
            for (int y = 0; y < PART_Y; y++){
                result[index] = parts[x][y];
                index++;
            }
        }
    }

    return result;
}

cv::Point2d Skeleton::getMassCenter(cv::Mat ske){
    int sumX = 0;
    int sumY = 0;
    double count = 0;
    for (int i = 0; i < ske.cols; i++){
        for (int j = 0; j < ske.rows; j++){
            if (ske.at<uchar>(j,i) == 255){
                sumX += i;
                sumY += j;
                count ++;
            }
        }
    }

    double tempX = (sumX / count) / ske.cols;
    double tempY = (sumY / count) / ske.rows;

    tempX = (tempX - 0.5) * 3;

    if (tempX < 0){
        tempX = 0;
    }
    else if (tempX > 1){
        tempX = 1;
    }

    return cv::Point2d(tempX, tempY);
}

double Skeleton::getCount(cv::Mat ske){
    double count = 0;
    for (int i = 0; i < ske.cols; i++){
        for (int j = 0; j < ske.rows; j++){
            if (ske.at<uchar>(j,i) == 255){
                count ++;
            }
        }
    }

    double tempCount = count / (ske.cols * ske.rows);

    //return min(tempCount * 20, 1.0);
    return tempCount;
}

void Skeleton::setParts(cv::Mat ske){
    for (int x = 0; x < PART_X; x++){
        for (int y = 0; y < PART_Y; y++){
            parts[x][y] = 0;
        }
    }

    for (int i = 0; i < ske.cols; i++){
        for (int j = 0; j < ske.rows; j++){
            if (ske.at<uchar>(j,i) == 255){
                parts[(PART_X * i) / ske.cols][(PART_Y * j) / ske.rows] ++;
            }
        }
    }

    // normalization
    for (int x = 0; x < PART_X; x++){
        for (int y = 0; y < PART_Y; y++){
            parts[x][y] = (parts[x][y] / ((ske.cols / PART_X) * (ske.rows / PART_Y)));
        }
    }

    // increase value, for better detection results, otherwise, each number is very close to 0
    /*for (int x = 0; x < PART_X; x++){
        for (int y = 0; y < PART_Y; y++){
            parts[x][y] = min(parts[x][y] * 20, 1.0);
        }
    }*/
}


int Skeleton::getDim(int type){
    switch(type){
        case M0 :
        return VECTOR_DIMENSION_0;

         default :
        return VECTOR_DIMENSION_1;
    }
}

int Skeleton::getEndCount(int type){
    switch(type){
        case M0 :
        return END_0;

         default :
        return END_1;
    }
}

int Skeleton::getJunctionCount(int type){
    switch(type){
        case M0 :
        return JUNCTION_0;

         default :
        return JUNCTION_1;
    }
}

int Skeleton::getHoleCount(int type){
    switch(type){
        case M0 :
        return HOLE_0;

         default :
        return HOLE_1;
    }
}

int Skeleton::getMassCenterCount(int type){
    switch(type){
        case M0 :
        return MASS_CENTER_0;

         default :
        return MASS_CENTER_1;
    }
}

int Skeleton::getTotalCount(int type){
    switch(type){
        case M0 :
        return TOTAL_0;

         default :
        return TOTAL_1;
    }
}

int Skeleton::getPartCount(int type){
    switch(type){
        case M0 :
        return PARTS_0;

         default :
        return PARTS_1;
    }
}

double Skeleton::min(double a, double b){
    return a < b ? a : b;
}

double Skeleton::max(double a, double b){
    return a > b ? a : b;
}

int Skeleton::min(int a, int b){
    return a < b ? a : b;
}

int Skeleton::max(int a, int b){
    return a > b ? a : b;
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

void Skeleton::setMachines(Machines newMachines){
    Skeleton::machines = newMachines;
}
