#ifndef SPRITE_H
#define SPRITE_H

#include "Image.h"
#include "Rectangle.h"

class Sprite
{
private:
    Image* image;
    int firstFrame;
    int lastFrame;
    int delay;
    int currentFrame;
    int delayCounter;
public:
    Sprite();
    ~Sprite();
    void setImage(Image* i);
    Image* getImage();
    void update();
    void draw(int x, int y, Graphics* g);
    void setAnimation(int f, int l, int d);
    void setFrame(int f);

    int getWidth();
    int getHeight();
};

#endif
