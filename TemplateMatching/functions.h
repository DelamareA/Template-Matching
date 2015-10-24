#ifndef FUNCTIONS
#define FUNCTIONS

#define FUNCTIONS_COUNT 1

#include "output.h"
#include "template.h"

Output* templateMatching(QString imagePath, Template *tem, int method, QString methodName);

Output* basicTemplateMatching(QString imagePath, Template *tem);

#endif // FUNCTIONS

