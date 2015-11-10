#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QList>

class Histogram {

    public:
        Histogram(int size);
        void add(int index, double value);
        double compare(Histogram other);
        QList<double> getNormalized();

    private:
        QList<double> list;
};

#endif // HISTOGRAM_H
