#ifndef FUNCTIONS
#define FUNCTIONS

#define MODULES_COUNT 5
#define FUNCTIONS_COUNT 1
#define ROTATION_STEP 10
#define ROTATION_MAX 30
#define ROTATION_COUNT (2*ROTATION_MAX/ROTATION_STEP + 1)

enum {TEMPLATE_MATCHING, CENTER_MASS, HALVES_CENTER_MASS_VERTI, HALVES_CENTER_MASS_HORI, HISTOGRAMS};

#include "output.h"
#include "template.h"
#include "skeleton.h"

Output* templateMatching(cv::Mat image, Template* tem, int modules[MODULES_COUNT], cv::Mat background);

Output* basicTemplateMatching(cv::Mat image, Template* tem, cv::Mat background);

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
void generateSVM(QString path);

#endif // FUNCTIONS

