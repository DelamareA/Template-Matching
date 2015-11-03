#ifndef FUNCTIONS
#define FUNCTIONS

#define FUNCTIONS_COUNT 1
#define ROTATION_STEP 5
#define ROTATION_MAX 10
#define ROTATION_COUNT (2*ROTATION_MAX/ROTATION_STEP + 1)

#include "output.h"
#include "template.h"

Output* templateMatching(QString imagePath, Template *tem, int method, QString methodName, QString backgroundPath);

Output* basicTemplateMatching(QString imagePath, Template *tem, QString backgroundPath);

int colorDistance(cv::Vec3b c1, cv::Vec3b c2);

cv::Mat extractBackgroundFromVideo(QString fileName, int maxFrames);
cv::Mat extractBackgroundFromFiles(QStringList filesName);

cv::Mat closeGaps(cv::Mat image, int patchSize, double ratio);

cv::Point2f getMassCenterFromImage(cv::Mat image);

#endif // FUNCTIONS

