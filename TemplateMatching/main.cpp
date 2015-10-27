#include <QApplication>
#include <opencv2/opencv.hpp>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "functions.h"
#include "output.h"

int templateMatching(QString imagePath, QString templatesPath, int function, QString outputPath);

int main(int argc, char *argv[]){
    /*if (argc != 5){
        return 1;
    }*/

    QString imagePath = "screenshot2.png";  //QString(argv[1]);
    QString templatesPath = "templatesNumber/";  //QString(argv[2]);
    int function = 0;  //QString(argv[3]).toInt();
    QString outputPath = "output.txt";  //QString(argv[4]);

    return templateMatching(imagePath, templatesPath, function, outputPath);
}

int templateMatching(QString imagePath, QString templatesPath, int function, QString outputPath){

    if (function < 0 || function >= FUNCTIONS_COUNT){
        return 2;
    }

    Template* templateNumers = new Template(templatesPath);

    Output* out = 0;

    switch (function){
        default:
            out = basicTemplateMatching(imagePath, templateNumers);
        break;
    }

    out->display();

    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << out->toString() << endl;
    }
    else {
        qDebug() << "Cannot open " + outputPath;
    }


    return 0;
}
