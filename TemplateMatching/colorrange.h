#ifndef COLORRANGE_H
#define COLORRANGE_H


class ColorRange{
    public:
        ColorRange(bool invertHRange, int h1, int h2, int s1, int s2, int v1, int v2);
        bool isInvertHRange();
        int getH1();
        int getH2();
        int getS1();
        int getS2();
        int getV1();
        int getV2();

     private:
        bool invertHRange;
        int h1;
        int h2;
        int s1;
        int s2;
        int v1;
        int v2;
};

#endif // COLORRANGE_H
