// Game Of Life 
#include <FastLED.h>
#define FASTLED_INTERNAL

#define LED_PIN     3
#define NUM_LEDS    288
#define BRIGHTNESS  64
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_ROWS    18
#define NUM_COLS    16
CRGB leds[NUM_LEDS];          // our display
int led_pos=0;

// Game Of Life variables
CRGB gameoflife[NUM_LEDS];    // our array
CRGB liveCell = CRGB::White;
CRGB deadCell = CHSV(30,127,64);
int endGameCounter = 0;
int endGameThreshold = 30;

// forward declaration
int CountNeighbors(int x, int y);

// convert X,Y position to a linear array, with zigzag wiring
// position 1,1 is lower-left corner, first row
// first row wiring is left-to-right
// second row wiring is right-to-left
int LEDArrayPosition(int x, int y){
  // do some bounds checking 
  if (x>NUM_COLS) x=NUM_COLS;
  if (x<1) x=1;
  if (y>NUM_ROWS) y = NUM_ROWS;
  if (y<1) y=1;

  if (y%2==0){
    led_pos = ((y) * NUM_COLS) - x;  // even row
  } else {
    led_pos = x + ((y-1) * NUM_COLS) -1;  // odd row 
  }
  return led_pos;
}

// draw a single pixel on the matrix screen at specified color
void DrawPixel(uint8_t x, uint8_t y, CRGB pixelcolor){
  leds[LEDArrayPosition(x, y)] = pixelcolor;
}


void InitGameOfLife(float populationPercent){
  int randNumber;
  // seed random number
  randomSeed(analogRead(0));

  for (int x=1;x<=NUM_COLS;x++){
    for (int y=1;y<=NUM_ROWS;y++){
      randNumber = random(100);
      if (randNumber<populationPercent){
        // this cell is alive
        gameoflife[LEDArrayPosition(x,y)] = liveCell;
      } else {
        // this cell is dead/blank
        gameoflife[LEDArrayPosition(x,y)] = deadCell;
      }
    } // y
  } // x 
}

void UpdateGameOfLife(){
  // THE GoL RULES:
  // #1. Any live cell with two or three live neighbours survives.
  // #2. Any dead cell with three live neighbours becomes a live cell.
  // #3. All other live cells die in the next generation. Similarly, all other dead cells stay dead.

  // we'll check the leds[] array, and update the gameoflife[] array with the resulting next generation
  for (int x=1;x<=NUM_COLS;x++){
    for (int y=1;y<=NUM_ROWS;y++){
      // Rule #1 Any live cell with two or three live neighbours survives.
      if (leds[LEDArrayPosition(x,y)]==liveCell) {
        if (CountNeighbors(x,y)==2 || CountNeighbors(x,y)==3)
          gameoflife[LEDArrayPosition(x,y)] = liveCell;
        else 
          gameoflife[LEDArrayPosition(x,y)] = deadCell;
      }; // liveCell

      // Rule #2 Any dead cell with three live neighbours becomes a live cell.
      if (leds[LEDArrayPosition(x,y)]==deadCell) {
        if (CountNeighbors(x,y)==3)
          gameoflife[LEDArrayPosition(x,y)] = liveCell;
        else 
          gameoflife[LEDArrayPosition(x,y)] = deadCell;
      } // deadCell 
    } // y
  } // x 

  // then copy our GoL array to the leds array
  // this does not update the LED Matrix Display
  // call FastLED.show() in main loop to actually light up the LEDs
  memcpy(leds, gameoflife, sizeof(leds));
}


// count surrounding neighbors of cell at position x,y
int CountNeighbors(int x, int y){
  int neighborCount=0;
  int xlow, xhigh, ylow, yhigh;

  // NOTE: we'll be counting neighbors using the leds[] array
  // because we'll be updating the gameoflife[] array for the next generation
  // NOTE: take into accountt boundary edges when doing x-1, x+1, y-1, y+1 that we don't exceed NUM_ROWS and NUM_COLS
  // position X, Y

  xlow = x-1;  xhigh = x+1;
  ylow = y-1;  yhigh = y+1;

  // clockwise, starting from top left
  if (leds[LEDArrayPosition(xlow,yhigh)] == liveCell && xlow>0 && yhigh<=NUM_ROWS )           neighborCount++;
  if (leds[LEDArrayPosition(x,yhigh)] == liveCell && yhigh<=NUM_ROWS )                        neighborCount++;
  if (leds[LEDArrayPosition(xhigh,yhigh)] == liveCell && xhigh<=NUM_COLS && yhigh<=NUM_ROWS ) neighborCount++;
  if (leds[LEDArrayPosition(xhigh,y)] == liveCell && xhigh<=NUM_COLS )                        neighborCount++;
  if (leds[LEDArrayPosition(xhigh,ylow)] == liveCell && xhigh<=NUM_COLS && ylow>0)            neighborCount++;
  if (leds[LEDArrayPosition(x,ylow)] == liveCell && ylow>0 )                                  neighborCount++;
  if (leds[LEDArrayPosition(xlow,ylow)] == liveCell && xlow>0 && ylow >0 )                    neighborCount++;
  if (leds[LEDArrayPosition(xlow,y)] == liveCell && xlow>0 )                                  neighborCount++;

  return neighborCount;
}

// bug: not working
void CheckForEndGame(){
  int n = memcmp(leds, gameoflife, sizeof(leds));
  if (n==0){
    endGameCounter++;
    if (endGameCounter > endGameThreshold){
      InitGameOfLife(40);
      delay(2000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  delay( 500 ); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );  

  InitGameOfLife(40);   // 50% populated
  // update display memory
  memcpy(leds, gameoflife, sizeof(leds));
  // light display + Long Pause
  FastLED.show(); 
  delay(1000);  
}

void loop() {
  // follow GoL rules
  UpdateGameOfLife();

  // update display
  FastLED.show(); 

  // slow everything 
  delay(60);
}