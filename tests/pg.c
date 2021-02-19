#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>

#define GREEN_LED BIT6

/*void main(){

  configureClocks();
  lcd_init();
  p2sw_init(15);
  or_sr(0x8);
  u_char width = screenWidth, height = screenHeight;

 clearScreen(COLOR_BLACK);

 drawString5x7(50, 50, "Pelas", COLOR_WHITE, COLOR_BLUE);

 drawRectOutline(20, 20, 20, 20, COLOR_GREEN);
*/

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
  &fieldLayer,
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

    u_int bgColor = COLOR_BLUE; // The background color
    int redrawScreen = 1; // Bool for whether screen needs to be redrawn
    Region fieldFence; // fence around playing field

    void main(){
      //P1DIR |= GREEN_LED;
      //P1OUT |= GREEN_LED;

      configureClocks();
      lcd_init();
      shapeInit();
      p2sw_init(1);

      shapeInit();

      layerInit(&layer0);
      layerDraw(&layer0);

      layerGetBounds(&fieldLayer, &fieldFence);

      enableWDTInterrupts();
      or_sr(0x8);

      for(;;){
	redrawScreen = 0;
	movLayerDraw(&ml0, &layer0);
      }
   }

    void wdt_c_handler(){
      static short count = 0;
      P1OUT |= GREEN_LED;
      count ++;
      if(count == 15){
	mlAdvance(&ml0, &fieldFence);
	if(p2sw_read())
	  redrawScreen = 1;
	count = 0;
      }
      P1OUT &=  ~GREEN_LED;
    }
