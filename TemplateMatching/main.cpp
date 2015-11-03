#include <QApplication>
#include <opencv2/opencv.hpp>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "functions.h"
#include "output.h"
#include "configuration.h"

int templateMatching(QString imagePath, QString templatesPath, int function, QString outputPath, QString configPath, QString backgroundPath);

int main(int argc, char *argv[]){
    /*if (argc != 7){
        return 1;
    }*/

    QString imagePath = "screenshot3.png";  //QString(argv[1]);
    QString templatesPath = "templatesNumber4/";  //QString(argv[2]);
    int function = 0;  //QString(argv[3]).toInt();
    QString outputPath = "output.txt";  //QString(argv[4]);
    QString configPath = "config.txt";  //QString(argv[5]);
    QString backgroundPath = "background.png";  //QString(argv[6]);

    return templateMatching(imagePath, templatesPath, function, outputPath, configPath, backgroundPath);
}

int templateMatching(QString imagePath, QString templatesPath, int function, QString outputPath, QString configPath, QString backgroundPath){

    if (function < 0 || function >= FUNCTIONS_COUNT){
        return 2;
    }

    Template* templateNumers = new Template(templatesPath);

    Configuration::setConfigFromFile(configPath);

    Output* out = 0;

    switch (function){
        default:
            out = basicTemplateMatching(imagePath, templateNumers, backgroundPath);
        break;
    }

    out->display();

    QFile file(outputPath);
    if (file.open(QIODevice::WriteOnly)){
        QTextStream stream(&file);
        stream << out->toString() << endl;
        file.close();
    }
    else {
        qDebug() << "Cannot open " + outputPath;
    }


    return 0;
}
