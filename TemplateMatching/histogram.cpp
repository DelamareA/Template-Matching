#include "histogram.h"
#include <QDebug>
#include <qmath.h>

Histogram::Histogram(int size){
    for (int i = 0; i < size; i++){
        list.push_back(0.0);
    }
}

void Histogram::add(int index, double value){
    list[index]+=value;

     //qDebug() << value;
}

double Histogram::compare(Histogram other){
    if (other.list.size() != list.size()){
        qDebug() << "The two histograms dont have the same size";
    }

    QList<double> norm = getNormalized();
    QList<double> otherNorm = other.getNormalized();

    double diff = 0;

    for (int i = 0; i < list.size(); i++){
        if (norm[i] > otherNorm[i]){
            diff += norm[i] - otherNorm[i];
        }
        else {
            diff += otherNorm[i] - norm[i];
        }
    }

    return 1.0 / (diff + 0.0000001);

}

QList<double> Histogram::getNormalized(){
    QList<double> norm;

    double max = 0;

    for (int i = 0; i < list.size(); i++){
        if (list[i] > max){
            max = list[i];
        }
    }

    for (int i = 0; i < list.size(); i++){
        norm.push_back(list[i] / max);
    }

    return norm;
}
