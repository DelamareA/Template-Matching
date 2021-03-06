#ifndef FUNCTIONS
#define FUNCTIONS

#include <opencv2/ml.hpp>

#include "output.h"
#include "template.h"
#include "skeleton.h"

#define MODULES_COUNT 5
#define FUNCTIONS_COUNT 1
#define ROTATION_STEP 10
#define ROTATION_MAX 30
#define ROTATION_COUNT (2*ROTATION_MAX/ROTATION_STEP + 1)

enum {TEMPLATE_MATCHING, CENTER_MASS, HALVES_CENTER_MASS_VERTI, HALVES_CENTER_MASS_HORI, HISTOGRAMS};

Output* templateMatching(cv::Mat image, Template* tem, int modules[MODULES_COUNT], cv::Mat background, QList<int> digitsOnField);

Output* basicTemplateMatching(cv::Mat image, Template* tem, cv::Mat background, QList<int> digitsOnField);

int colorDistance(cv::Vec3b c1, cv::Vec3b c2);

cv::Mat extractBackgroundFromVideo(QString fileName, int maxFrames);
cv::Mat extractBackgroundFromVideo2(QString fileName, int maxFrames);
cv::Mat extractBackgroundFromFiles(QStringList filesName);

cv::Mat closeGaps(cv::Mat image, int patchSize, double ratio);

cv::Point2f getMassCenterFromImage(cv::Mat image);
cv::Mat getSkeleton(cv::Mat image);

cv::Mat thinningGuoHall(cv::Mat image);
void thinningGuoHallIteration(cv::Mat& im, int iter);

void generateDataSet(QList<int> numbers, int countPerNumber, int width, int height, QString outputPath);
void generateSVM(QString path, int type);

//int mostProbableNumber(cv::Mat image, QList<int> digitsOnField);
//int mostProbableDigit(cv::Mat numberImage, QList<int> digitsOnField, std::vector<cv::Point> contour);
void runOnDataSet(QList<int> digitsOnField);

#endif // FUNCTIONS

