#include "colorrange.h"

ColorRange::ColorRange(bool invertHRange, int h1, int h2, int s1, int s2, int v1, int v2){
    this->invertHRange = invertHRange;
    this->h1 = h1;
    this->h2 = h2;
    this->s1 = s1;
    this->s2 = s2;
    this->v1 = v1;
    this->v2 = v2;
}

bool ColorRange::isInvertHRange(){
    return invertHRange;
}

int ColorRange::getH1(){
    return h1;
}

int ColorRange::getH2(){
    return h2;
}

int ColorRange::getS1(){
    return s1;
}

int ColorRange::getS2(){
    return s2;
}
int ColorRange::getV1(){
    return v1;
}

int ColorRange::getV2(){
    return v2;
}
