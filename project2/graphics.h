#ifndef graphics
#define graphics

#include <lcdutils.h>
#include <lcddraw.h>
#include <abCircle.h>
#include <shape.h>

typedef struct MovLayer_s{
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

extern AbRectOutline fieldOutline;

extern Layer fieldLayer, layer1, layer0;

extern MovLayer ml1, ml0;

void movLayerDraw(MovLayer *movLayers, Layer *layers);

void mlAdvance(MovLayer *ml, Region *fence);

#endif
