#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"
#include "player.h"

char player_collision = '0'; //Collision flag
char wall_collision = '0';   //Collision flag


//Creating shapes to be drawn
Player player = {playerGetBounds, playerCheck}; //Algorithmically created player

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

Layer player_layer = { //Player
  (AbShape *)&player,
  {(screenWidth/2), (screenHeight - 30)},
  {0,0}, {0,0},
  COLOR_RED,
  &fieldLayer,
 };

Layer layer1 = { //Gold Circle
  (AbShape *)&circle4,
  {(screenWidth/2), 20},
  {0,0}, {0,0},
  COLOR_GOLD,
  &player_layer,
};

Layer layer0 = {
  (AbShape *)&circle4,               //Shape type
  {(screenWidth/2), (screenHeight/2)}, //Starting pos
  {0,0}, {0,0},                       //Last, Next pos
  COLOR_DEEP,
  &layer1,
};

typedef struct MovLayer_s{          //Linked list holding layers
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

MovLayer ml2 = { &player_layer, {0, 0}, 0};
MovLayer ml1 = { &layer1, {2, 1}, &ml2};
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
	  wall_collision = '1'; //Obj collided with wall, play sound
	  play_note(3000);
	  int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
	  newPos.axes[axis] += (2*velocity);
	  wall_collision = '1'; //Obj collided with wall, play sound
	} // if outside of fence
      } // for axis
      ml->layer->posNext = newPos;
    } // for ml
  }

    u_int bgColor = COLOR_DARK_VIOLE; // The background color
    int redrawScreen = 1; // Bool for whether screen needs to be redrawn
    Region fieldFence; // fence around playing field
    static int duration = 300;

    void main(void){

      configureClocks();
      lcd_init();
      shapeInit();
      p2sw_init(15);
      or_sr(0x8);
      buzzer_init();

      shapeInit();

      layerInit(&layer0);
      layerDraw(&layer0);

      layerGetBounds(&fieldLayer, &fieldFence);

      enableWDTInterrupts();
      //or_sr(0x8);

      for(;;){
	redrawScreen = 0;
	movLayerDraw(&ml0, &layer0);
	drawString5x7(screenWidth-17, 3, "Go!", COLOR_WHITE, COLOR_BLACK);
	//drawChar5x7(20, 20, p2sw_read()+'0', COLOR_WHITE, COLOR_BLACK);
      }
   }

Vec2 playerNewPos;    //To update player direction
Vec2 speed = {0, 5};  //How fast to move player
Vec2 up = {0, -2};
Vec2 down = {0, 2};

void updateDirection(MovLayer *ml){
  u_int switches = p2sw_read();

  if(!(switches & BIT0)){
    vec2Sub(&ml->velocity, &ml->velocity, &ml->velocity);
    vec2Add(&ml->velocity, &ml->velocity, &up);
  }
  else if(!(switches & BIT1)){
    vec2Sub(&ml->velocity, &ml->velocity, &ml->velocity);
    vec2Add(&ml->velocity, &ml->velocity, &down);
  }
  else
    vec2Sub(&ml->velocity, &ml->velocity, &ml->velocity);
    
}

    void wdt_c_handler(){
      static int count = 0;
      count ++;
     
      if(count == 15){
	mlAdvance(&ml0, &fieldFence);
	updateDirection(&ml2);
	if(p2sw_read())
	  redrawScreen = 1;
	count = 0;
      }
      
      if(player_collision == '1'){ //play player collision sound 
	for(duration; duration > 0; duration--){
	  play_note(player_col_note);
	}
	buzzer_set_period(0); //disable sound and reset flags
	player_collision = '0';
	duration = 25;
      }
      if(wall_collision == '1'){ //play wall collision sound
	for(; duration > 0; duration--){
	  play_note(wall_col_note);
	}
        buzzer_set_period(0); //disable sound and reset flags
        wall_collision = '0';
	duration = 25;
      }
      /*
      if( p2sw_read() & BIT1 == 1){ //Reading switch input
	vec2Add(&playerNewPos, &ml2->player_layer->posNext, speed);
	&player_layer->posNext = playerNewPos;
      }
      */
    }
