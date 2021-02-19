#include <msp430.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <shape.h>
#include "player.h"
#include <p2switches.h>

void playerGetBounds(const Player *player, const Vec2 *center, Region *bounds){
  const Vec2 half = {5/2, 7/2};

  vec2Sub(&bounds->topLeft, center, &half);
  vec2Add(&bounds->botRight, center, &half);
}

int playerCheck(const Player *player, const Vec2 *center, const Vec2 *pixel){
  Vec2 relPos;

  int col, row, inside;

  vec2Sub(&relPos, pixel, center); //Allows to determine position for moving obj

  row = relPos.axes[1];
  col = relPos.axes[0];

  if( (row > -5 && row<5) && (col>-5 && col<5) ){ //Draw checkerboard pattern
    if(col % 2 == 0)
      inside = 1;
    else
      inside = 0;
  }
  else
    inside = 0;
  
  return inside;
}
/*
void updateDirection(MovLayer *layer){
  u_int switches = p2sw_read();

  if(!(switches & BIT1)){
    vec2Sub(&layer->velocity, &layer->velocity, &layer->velocity);
    vec2Add(&layer->velocity, &layer->velocity, (Vec2) {0, -2});
  }
}*/
