#include <QList>
#include <QPixmap>
#include <QPainter>
#include <QTime>
#include <QApplication>
#include <QDebug>
#include <QColor>
#include <qglobal.h>

void generateDataSet(QList<int> numbers, int countPerNumber, int width, int height, QString outputPathImages, QString outputPathLabels) {
    char **argv = new char*[1];
    argv[0] = new char[1];
    int argc = 1;
    QApplication a(argc, argv); // just to use QPixmaps

    qsrand(QTime::currentTime().msec());

    int size = 50;

    for (int i = 0; i < numbers.size(); i++){
        for (int j = 0; j < countPerNumber; j++){
            QPixmap pix(3 * width, 3 * height);
            pix.fill(Qt::black);
            QPainter painter(&pix);
            painter.setPen(QColor(255, 255, 255));

            switch(rand() % 3){
                default :
                painter.setFont(QFont("Arial", size));
                break;

                case 1:
                painter.setFont(QFont("Courrier", size));
                break;

                case 2:
                painter.setFont(QFont("Calibri", size));
                break;
            }

            painter.translate(width, size);
            painter.rotate((rand() % 100) - 50);
            painter.drawText(QPoint(0, size), QString::number(numbers[i]));

            QImage image = pix.toImage();
            int minX = pix.width()-1;
            int maxX = 0;
            int minY = pix.height()-1;
            int maxY = 0;
            for (int x = 0; x < pix.width(); x++){
                for (int y = 0; y < pix.height(); y++){
                    QRgb val = image.pixel(x, y);

                    if (qRed(val) > 120){
                        image.setPixel(x, y, qRgb(255, 255, 255));

                        if (x < minX){
                            minX = x;
                        }
                        if (x > maxX){
                            maxX = x;
                        }
                        if (y < minY){
                            minY = y;
                        }
                        if (y > maxY){
                            maxY = y;
                        }
                    }
                    else {
                        image.setPixel(x, y, qRgb(0, 0, 0));
                    }
                }
            }

            QRect rect(QPoint(minX, minY), QPoint(maxX, maxY));
            image.copy(rect).scaled(width, height).save(outputPathImages + QString::number(i * countPerNumber + j) + ".png");
        }
    }
}
