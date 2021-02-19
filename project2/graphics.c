#include <msp430.h>
#include <libTimer.h>
#include <shape.h>
#include <abCircle.h>
#include <lcddraw.h>
#include <lcdutils.h>
#include "graphics.h"

AbRectOutline fieldOutline = {
  abRectOutlineGetBounds, abRectOutlineCheck,
  {screenWidth/2-10, screenHeight/2-10},
};

Layer fieldLayer = {
  (AbShape *) &fieldOutline,
  {screenWidth/2, screenHeight/2},
  {0,0}, {0,0},
  COLOR_BLACK,
  0,
};

Layer layer1 = {
  (AbShape *)&circle4,
  {(screenWidth/2), 20},
  {0,0}, {0,0},
  COLOR_GOLD,
  &layer1,
};

Layer layer0 = {
  (AbShape *)&circle4,
  {(screenWidth/2), (screenHeight/2)},
  {0,0}, {0,0},
  COLOR_DEEP,
  &layer1,
};

/*typedef struct MovLayer_s{
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;
*/

MovLayer ml1 = { &layer1, {2, 1}, 0};
MovLayer ml0 = { &layer0, {1, 2}, &ml1};

void movLayerDraw(MovLayer *movLayers, Layer *layers){
  int row, col;
  MovLayer *movLayer;

  and_sr(~8);
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next){ //for each moving layer
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8); //disable interrupts

  for(movLayer = movLayers; movLayer; movLayer = movLayer->next){ // for each moving layer
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds); //Gets bounds from layer shape
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1],
	        bounds.botRight.axes[0], bounds.botRight.axes[1]);
      
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++){
      for(col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++){
        Vec2 pixelPos = {col, row};
        u_int color = bgColor;
        Layer *probeLayer;
        for(probeLayer = layers; probeLayer;
            probeLayer = probeLayer->next){ //probe all layers, in order
          if(abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)){
            color = probeLayer->color;
            break;
	  } //if probe check
        } //for checking all layers at col, row
        lcd_writeColor(color);
      } // for col
    } //for row
  } //for moving layer being updated
}

void mlAdvance(MovLayer *ml, Region *fence){
  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;

  for(; ml; ml = ml->next){
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity); //newPos = posNext + velocity
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis ++){
      if((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
         (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]) ){
        int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        newPos.axes[axis] += (2*velocity);
      } // if outside of fence
    } // for axis
    ml->layer->posNext = newPos;
  } // for ml
}
