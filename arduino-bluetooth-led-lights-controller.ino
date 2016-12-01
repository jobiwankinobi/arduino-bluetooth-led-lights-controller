// Uncomment this line if you have any interrupts that are changing pins - this causes the library to be a little bit more cautious
// #define FAST_SPI_INTERRUPTS_WRITE_PINS 1

// Uncomment this line to force always using software, instead of hardware, SPI (why?)
// #define FORCE_SOFTWARE_SPI 1

// Uncomment this line if you want to talk to DMX controllers
// #define FASTSPI_USE_DMX_SIMPLE 1
//-----------------------------------------------------

#include <SoftwareSerial.h>
//#include <FastSPI_LED2.h>
#include <FastLED.h>

#define VERSION_NUMBER 0.51

//---LED SETUP STUFF
#define LED_COUNT 300          //FOR TESTING w/ SIGN
#define LED_DT 9             //SERIAL DATA PIN
//#define LED_CK 11             //SERIAL CLOCK PIN

int BOTTOM_INDEX = 0;
int TOP_INDEX = int(LED_COUNT/2);
int EVENODD = LED_COUNT%2;
struct CRGB leds[LED_COUNT]; 
struct CRGB led_copy_temp;     // used as a memory saving means over former implementation of entire led array available as CRGB array
//int ledsX[LED_COUNT][3];     //-ARRAY FOR COPYING WHATS IN THE LED STRIP CURRENTLY (FOR CELL-AUTOMATA, MARCH, ETC)
//int ledMode = 3;           //-START IN RAINBOW LOOP
int ledMode = 14;         //-START IN DEMO MODE
//int ledMode = 6;             //-MODE TESTING

int thisdelay = 50;          //-FX LOOPS DELAY VAR
int thisstep = 10;           //-FX LOOPS DELAY VAR
int thishue = 0;             //-FX LOOPS DELAY VAR
int thissat = 255;           //-FX LOOPS DELAY VAR
int max_bright = 20;         //-SET MAX BRIGHTNESS TO 1/4

int thisindex = 0;           //-SET SINGLE LED VAR
int thisRED = 0;
int thisGRN = 0;
int thisBLU = 0;

//---SERIAL/SOFTWARE SERIAL SETUP STUFF
//#define SERIAL_BAUDRATE 38400
#define SERIAL_BAUDRATE 9600
//#define SERIAL_BAUDRATE 57600
//#define SERIAL_BAUDRATE 115200
#define SERIAL_TIMEOUT 5

#define SOFT_RX_PIN 2         // BTCHIP-TX -> ARDUINO-SOFT-RX (2)
#define SOFT_TX_PIN 3         // BTCHIP-RX -> ARDUINO-SOFT-TX (3)

SoftwareSerial btSerial(SOFT_RX_PIN, SOFT_TX_PIN); // Setup a SoftwareSerial interface (as opposed to hardware Serial, which is defined by default)
int btBOOL = 1;              //-ACTIVATE BLUETOOTH OR NOT
byte inbyte;                 //-SERIAL INPUT BYTE
int thisarg;                 //-SERIAL INPUT ARG

//---LED FX VARS
int thisidex = 0;
int idex = 0;                //-LED INDEX (0 to LED_COUNT-1
int ihue = 0;                //-HUE (0-255)
int ibright = 0;             //-BRIGHTNESS (0-255)
int isat = 0;                //-SATURATION (0-255)
int bouncedirection = 0;     //-SWITCH FOR COLOR BOUNCE (0-1)
float tcount = 0.0;          //-INC VAR FOR SIN LOOPS
int lcount = 0;              //-ANOTHER COUNTING VAR


//------------------------------------- UTILITY FXNS --------------------------------------
//---SET THE COLOR OF A SINGLE RGB LED
void set_color_led(int adex, int cred, int cgrn, int cblu) {  
  leds[adex].setRGB( cred, cgrn, cblu);
} 

//---FIND INDEX OF HORIZONAL OPPOSITE LED
int horizontal_index(int i) {
  //-ONLY WORKS WITH INDEX < TOPINDEX
  if (i == BOTTOM_INDEX) {return BOTTOM_INDEX;}
  if (i == TOP_INDEX && EVENODD == 1) {return TOP_INDEX + 1;}
  if (i == TOP_INDEX && EVENODD == 0) {return TOP_INDEX;}
  return LED_COUNT - i;  
}

//---FIND INDEX OF ANTIPODAL OPPOSITE LED
int antipodal_index(int i) {
  int iN = i + TOP_INDEX;
  if (i >= TOP_INDEX) {iN = ( i + TOP_INDEX ) % LED_COUNT; }
  return iN;
}

//---FIND ADJACENT INDEX CLOCKWISE
int adjacent_cw(int i) {
  int r;
  if (i < LED_COUNT - 1) {r = i + 1;}
  else {r = 0;}
  return r;
}

//---FIND ADJACENT INDEX COUNTER-CLOCKWISE
int adjacent_ccw(int i) {
  int r;
  if (i > 0) {r = i - 1;}
  else {r = LED_COUNT - 1;}
  return r;
}


//------------------------LED EFFECT FUNCTIONS------------------------
void one_color_all(int cred, int cgrn, int cblu) {       //-SET ALL LEDS TO ONE COLOR
    for(int i = 0 ; i < LED_COUNT; i++ ) {
      leds[i].setRGB( cred, cgrn, cblu);
    }
}

void one_color_allHSV(int ahue) {    //-SET ALL LEDS TO ONE COLOR (HSV)
    for(int i = 0 ; i < LED_COUNT; i++ ) {
      leds[i] = CHSV(ahue, thissat, 255);
    }
}

void rainbow_fade() {                         //-m2-FADE ALL LEDS THROUGH HSV RAINBOW
    ihue++;
    if (ihue > 255) {ihue = 0;}
    for(int idex = 0 ; idex < LED_COUNT; idex++ ) {
      leds[idex] = CHSV(ihue, thissat, 255);
    }
    LEDS.show();    
    delay((int) (thisdelay * .2));
}

void rainbow_loop() {                        //-m3-LOOP HSV RAINBOW
  idex++;
  ihue = ihue + thisstep;
  if (idex >= LED_COUNT) {idex = 0;}
  if (ihue > 255) {ihue = 0;}
  leds[idex] = CHSV(ihue, thissat, 255);
  LEDS.show();
  delay((int) (thisdelay * .4));
}

void random_burst() {                         //-m4-RANDOM INDEX/COLOR
  idex = random(0, LED_COUNT);
  ihue = random(0, 255);  
  leds[idex] = CHSV(ihue, thissat, 255);
  LEDS.show();
  delay((int) (thisdelay * .4));
}

void color_bounce() {                        //-m5-BOUNCE COLOR (SINGLE LED)
  if (bouncedirection == 0) {
    idex = idex + 1;
    if (idex == LED_COUNT) {
      bouncedirection = 1;
      idex = idex - 1;
    }
  }
  if (bouncedirection == 1) {
    idex = idex - 1;
    if (idex == 0) {
      bouncedirection = 0;
    }
  }  
  for(int i = 0; i < LED_COUNT; i++ ) {
    if (i == idex) {leds[i] = CHSV(thishue, thissat, 255);}
    else {leds[i] = CHSV(0, 0, 0);}
  }
  LEDS.show();
  delay((int) (thisdelay * .4));
}

void color_bounceFADE() {                    //-m6-BOUNCE COLOR (SIMPLE MULTI-LED FADE)
  if (bouncedirection == 0) {
    idex = idex + 1;
    if (idex == LED_COUNT) {
      bouncedirection = 1;
      idex = idex - 1;
    }
  }
  if (bouncedirection == 1) {
    idex = idex - 1;
    if (idex == 0) {
      bouncedirection = 0;
    }
  }
  int iL1 = adjacent_cw(idex);
  int iL2 = adjacent_cw(iL1);
  int iL3 = adjacent_cw(iL2);
  int iR1 = adjacent_ccw(idex);
  int iR2 = adjacent_ccw(iR1);
  int iR3 = adjacent_ccw(iR2);
  for(int i = 0; i < LED_COUNT; i++ ) {
    if (i == idex) {leds[i] = CHSV(thishue, thissat, 255);}
    else if (i == iL1) {leds[i] = CHSV(thishue, thissat, 150);}
    else if (i == iL2) {leds[i] = CHSV(thishue, thissat, 80);}
    else if (i == iL3) {leds[i] = CHSV(thishue, thissat, 20);}        
    else if (i == iR1) {leds[i] = CHSV(thishue, thissat, 150);}
    else if (i == iR2) {leds[i] = CHSV(thishue, thissat, 80);}
    else if (i == iR3) {leds[i] = CHSV(thishue, thissat, 20);}    
    else {leds[i] = CHSV(0, 0, 0);}
  }
  LEDS.show();
  delay((int) (thisdelay * .8));
}

void ems_lightsONE() {                    //-m7-EMERGENCY LIGHTS (TWO COLOR SINGLE LED)
  idex++;
  if (idex >= LED_COUNT) {idex = 0;}
  int idexR = idex;
  int idexB = antipodal_index(idexR);
  int thathue = (thishue + 160) % 255;  
  for(int i = 0; i < LED_COUNT; i++ ) {
    if (i == idexR) {leds[i] = CHSV(thishue, thissat, 255);}
    else if (i == idexB) {leds[i] = CHSV(thathue, thissat, 255);}    
    else {leds[i] = CHSV(0, 0, 0);}
  }
  LEDS.show();  
  delay((int) (thisdelay * .8));
}

void ems_lightsALL() {                  //-m8-EMERGENCY LIGHTS (TWO COLOR SOLID)
  idex++;
  if (idex >= LED_COUNT) {idex = 0;}
  int idexR = idex;
  int idexB = antipodal_index(idexR);
  int thathue = (thishue + 160) % 255;
  leds[idexR] = CHSV(thishue, thissat, 255);
  leds[idexB] = CHSV(thathue, thissat, 255);  
  LEDS.show();  
  delay((int) (thisdelay * .4));
}


void bomber(){                           //-m9
   int function_delay;
   if(bouncedirection == 1){
    idex+= (int) LED_COUNT / 20;
    for(int i = 0 ; i < LED_COUNT; i++ ) {
      if(i >= TOP_INDEX - idex && i <= TOP_INDEX + idex){
        leds[i] = CHSV(thishue, thissat, 255);
      }
    }
    if(idex >= TOP_INDEX){
      bouncedirection = 2;
    }
    function_delay = (int) thisdelay * .05;
  } else if(bouncedirection == 0) {
    idex--;
    for(int i = 0 ; i < LED_COUNT; i++ ) {
      if(i > TOP_INDEX + idex || i < TOP_INDEX - idex){
        leds[i] = CRGB(0,0,0);
      } else if(i == TOP_INDEX + idex || i == TOP_INDEX - idex){
        leds[i] = CHSV(thishue, thissat, 255);
      }
    }
    if(idex <= 0){
      bouncedirection = 1;
    }
    function_delay = (int) thisdelay * .1;
  }
  LEDS.show();
  delay(function_delay);

}

void pulse_one_color_all() {              //-m10-PULSE BRIGHTNESS ON ALL LEDS TO ONE COLOR 
  if (bouncedirection == 0) {
    ibright++;
    if (ibright >= 255) {bouncedirection = 1;}
  }
  if (bouncedirection == 1) {
    ibright = ibright - 1;
    if (ibright <= 1) {bouncedirection = 0;}         
  }  
    for(int idex = 0 ; idex < LED_COUNT; idex++ ) {
      leds[idex] = CHSV(thishue, thissat, ibright);
    }
    LEDS.show();    
    delay((int) (thisdelay * .05));
}

void pulse_one_color_all_rev() {           //-m11-PULSE SATURATION ON ALL LEDS TO ONE COLOR 
  if (bouncedirection == 0) {
    isat++;
    if (isat >= 255) {bouncedirection = 1;}
  }
  if (bouncedirection == 1) {
    isat = isat - 1;
    if (isat <= 1) {bouncedirection = 0;}         
  }  
    for(int idex = 0 ; idex < LED_COUNT; idex++ ) {
      leds[idex] = CHSV(thishue, isat, 255);
    }
    LEDS.show();
    delay((int) (thisdelay * .3));
}

void fade_vertical() {                    //-m12-FADE 'UP' THE LOOP
  idex++;
  if (idex > TOP_INDEX) {idex = 0;}  
  int idexA = idex;
  int idexB = horizontal_index(idexA);
  ibright = ibright + 10;
  if (ibright > 255) {ibright = 0;}
  leds[idexA] = CHSV(thishue, thissat, ibright);
  leds[idexB] = CHSV(thishue, thissat, ibright);
  LEDS.show();
  delay((int) (thisdelay * 1.1));
}



void theater_chase(){                    //-m13
  if (idex > 3 || idex < 1) {idex = 1;}
    for(int i = 0; i < LED_COUNT; i++ ) {
      if((i + idex)%3 == 0){
        leds[i] = CHSV(0,0,0);
      } else {
        leds[i] = CHSV(thishue,thissat,255);
      }
  }
  idex++;
  LEDS.show();  
  delay((int) (thisdelay * 1.2));  
}

void color_implode() {                                  //-m14-Rcolor fills from outside in
  for(int i = 0; i < LED_COUNT; i++){
    if(i <= idex || i >= LED_COUNT - idex){
      leds[i] = CHSV(thishue, thissat, 255);
    }  else {
      //leds[i] = CHSV(0, 0, 0);
    }
  }

  if(idex > (int) LED_COUNT / 2){
    idex = 0;
  } else {
    ++idex;
  }
  LEDS.show();  
  delay((int) (thisdelay * .3));
}


void rwb_march() {                    //-m15-R,W,B MARCH CCW
  thisidex++;
  if (thisidex > 2) {thisidex = 0;}
    memmove8( &leds[1], &leds[0], (LED_COUNT - 1) * sizeof( CRGB) );
    switch (thisidex) {
          case 0:
            leds[0].r = 255;
            leds[0].g = 165;
            leds[0].b = 0;
          break;
          case 1:
            leds[0].r = 138;
            leds[0].g = 43;
            leds[0].b = 226;
          break;
          case 2:
            leds[0].r = 50;
            leds[0].g = 205;
            leds[0].b = 50;
          break;
        }
  LEDS.show();  
  delay((int) (thisdelay * 2));
}

void radiation() {                   //-m16-SORT OF RADIATION SYMBOLISH- 
  int N3  = int(LED_COUNT/3);
  int N6  = int(LED_COUNT/6);  
  int N12 = int(LED_COUNT/12);  
  for(int i = 0; i < N6; i++ ) {     //-HACKY, I KNOW...
    tcount = tcount + .02;
    if (tcount > 3.14) {tcount = 0.0;}
    ibright = int(sin(tcount)*255);    
    int j0 = (i + LED_COUNT - N12) % LED_COUNT;
    int j1 = (j0+N3) % LED_COUNT;
    int j2 = (j1+N3) % LED_COUNT;    
    leds[j0] = CHSV(thishue, thissat, ibright);
    leds[j1] = CHSV(thishue, thissat, ibright);
    leds[j2] = CHSV(thishue, thissat, ibright);    
  }    
  LEDS.show();    
  delay(thisdelay);    
}

void color_loop_vardelay() {                    //-m17-COLOR LOOP (SINGLE LED) w/ VARIABLE DELAY
  idex++;
  if (idex > LED_COUNT) {idex = 0;}
  int di = abs(TOP_INDEX - idex);
  int t = constrain((10/di)*10, 10, 500);
  for(int i = 0; i < LED_COUNT; i++ ) {
    if (i == idex) {
      leds[i] = CHSV(0, thissat, 255);
    }
    else {
      leds[i].r = 0; leds[i].g = 0; leds[i].b = 0;
    }
  }
  LEDS.show();  
  delay(t);
}

void white_temps() {                            //-m18-SHOW A SAMPLE OF BLACK BODY RADIATION COLOR TEMPERATURES
  int N9 = int(LED_COUNT/9);
  for (int i = 0; i < LED_COUNT; i++ ) {
    if (i >= 0 && i < N9) {leds[i].r = 255; leds[i].g = 147; leds[i].b = 41;}         //-CANDLE - 1900
    if (i >= N9 && i < N9*2) {leds[i].r = 255; leds[i].g = 197; leds[i].b = 143;}     //-40W TUNG - 2600
    if (i >= N9*2 && i < N9*3) {leds[i].r = 255; leds[i].g = 214; leds[i].b = 170;}   //-100W TUNG - 2850
    if (i >= N9*3 && i < N9*4) {leds[i].r = 255; leds[i].g = 241; leds[i].b = 224;}   //-HALOGEN - 3200
    if (i >= N9*4 && i < N9*5) {leds[i].r = 255; leds[i].g = 250; leds[i].b = 244;}   //-CARBON ARC - 5200
    if (i >= N9*5 && i < N9*6) {leds[i].r = 255; leds[i].g = 255; leds[i].b = 251;}   //-HIGH NOON SUN - 5400
    if (i >= N9*6 && i < N9*7) {leds[i].r = 255; leds[i].g = 255; leds[i].b = 255;}   //-DIRECT SUN - 6000
    if (i >= N9*7 && i < N9*8) {leds[i].r = 201; leds[i].g = 226; leds[i].b = 255;}   //-OVERCAST SKY - 7000
    if (i >= N9*8 && i < LED_COUNT) {leds[i].r = 64; leds[i].g = 156; leds[i].b = 255;}//-CLEAR BLUE SKY - 20000  
  }
  LEDS.show();  
  delay(100);
}

void sin_bright_wave() {        //-m19-BRIGHTNESS SINE WAVE
  for(int i = 0; i < LED_COUNT; i++ ) {
    tcount = tcount + .1;
    if (tcount > 3.14) {tcount = 0.0;}
    ibright = int(sin(tcount)*255);
    leds[i] = CHSV(thishue, thissat, ibright);
    LEDS.show();    
    delay((int) (thisdelay * 2));
  }
}

void pop_horizontal() {        //-m20-POP FROM LEFT TO RIGHT UP THE RING
  int ix;
  if (bouncedirection == 0) {
    bouncedirection = 1;
    ix = idex;  
  }
  else if (bouncedirection == 1) {
    bouncedirection = 0;
    ix = horizontal_index(idex);
    idex++;
    if (idex > TOP_INDEX) {idex = 0;}      
  }
  for(int i = 0; i < LED_COUNT; i++ ) {
    if (i == ix) {
      leds[i] = CHSV(thishue, thissat, 255);
    }
    else {
      leds[i].r = 0; leds[i].g = 0; leds[i].b = 0;
    }
  }
  LEDS.show();
  delay((int) (thisdelay * 2));    
}

void quad_bright_curve() {      //-m21-QUADRATIC BRIGHTNESS CURVER
  int ax;    
  for(int x = 0; x < LED_COUNT; x++ ) {
    if (x <= TOP_INDEX) {ax = x;}
    else if (x > TOP_INDEX) {ax = LED_COUNT-x;}
    int a = 1; int b = 1; int c = 0;
    int iquad = -(ax*ax*a)+(ax*b)+c; //-ax2+bx+c
    int hquad = -(TOP_INDEX*TOP_INDEX*a)+(TOP_INDEX*b)+c;
    ibright = int((float(iquad)/float(hquad))*255);
    leds[x] = CHSV(thishue, thissat, ibright);
  }
  LEDS.show();  
  delay((int) (thisdelay * 2));
}

void flame() {                                    //-m22-FLAMEISH EFFECT
  int idelay = random(0,35);
  float hmin = 0.1; float hmax = 45.0;
  float hdif = hmax-hmin;
  int randtemp = random(0,3);
  float hinc = (hdif/float(TOP_INDEX))+randtemp;
  int ihue = hmin;
  for(int i = 0; i <= TOP_INDEX; i++ ) {
    ihue = ihue + hinc;
    leds[i] = CHSV(ihue, thissat, 255);
    int ih = horizontal_index(i);    
    leds[ih] = CHSV(ihue, thissat, 255);    
    leds[TOP_INDEX].r = 255; leds[TOP_INDEX].g = 255; leds[TOP_INDEX].b = 255;    
    LEDS.show();    
    delay((int) (thisdelay * 2));
  }
}

void rainbow_vertical() {                        //-m23-RAINBOW 'UP' THE LOOP
  idex++;
  if (idex > TOP_INDEX) {idex = 0;}  
  ihue = ihue + thisstep;
  if (ihue > 255) {ihue = 0;}
  int idexA = idex;
  int idexB = horizontal_index(idexA);
  leds[idexA] = CHSV(ihue, thissat, 255);
  leds[idexB] = CHSV(ihue, thissat, 255);
  LEDS.show();  
  delay(thisdelay);
}

void color_explode() {                                  //-m24-color fills from inside out
    for(int i = 0; i < LED_COUNT; i++){

      if(i >= LED_COUNT/2 - idex && i <= LED_COUNT/2 + idex){
        leds[i] = CHSV(thishue, thissat, 255);
      }  else {
        //leds[i] = CHSV(0, 0, 0);
      }
    }
    
  //Serial.println(idex);

  if(idex > (int) LED_COUNT / 2){
    idex = 0;
  } else {
    ++idex;
  }
  LEDS.show();  
  delay((int) (thisdelay * .3));
}


void random_color_pop() {                         //-m25-RANDOM COLOR POP
  idex = random(0, LED_COUNT);
  ihue = random(0, 255);
  one_color_all(0, 0, 0);
  leds[idex] = CHSV(ihue, thissat, 255);
  LEDS.show();
  delay((int) (thisdelay * .7));
}

void ems_lightsSTROBE() {                  //-m26-EMERGENCY LIGHTS (STROBE LEFT/RIGHT)
  int thishue = 0;
  int thathue = (thishue + 160) % 255;
  for(int x = 0 ; x < 5; x++ ) {
    for(int i = 0 ; i < TOP_INDEX; i++ ) {
        leds[i] = CHSV(thishue, thissat, 255);
    }
    LEDS.show(); delay(thisdelay); 
    one_color_all(0, 0, 0);
    LEDS.show(); delay(thisdelay);
  }
  for(int x = 0 ; x < 5; x++ ) {
    for(int i = TOP_INDEX ; i < LED_COUNT; i++ ) {
        leds[i] = CHSV(thathue, thissat, 255);
    }
    LEDS.show(); delay(thisdelay);
    one_color_all(0, 0, 0);
    LEDS.show(); delay((int) (thisdelay * .5));
  }
}

void propeller(int hue1, int hue2, int hue3, int sat1, int sat2, int sat3) {                           //-m27-RGB PROPELLER 
  idex++;
  int N3  = int(LED_COUNT/3);
  int N6  = int(LED_COUNT/6);  
  int N12 = int(LED_COUNT/12);  
  for(int i = 0; i < N3; i++ ) {
    int j0 = (idex + i + LED_COUNT - N12) % LED_COUNT;
    int j1 = (j0+N3) % LED_COUNT;
    int j2 = (j1+N3) % LED_COUNT;    
    leds[j0] = CHSV(hue1, sat1, 255);
    leds[j1] = CHSV(hue2, sat2, 255);
    leds[j2] = CHSV(hue3, sat3, 255);    
  }
  LEDS.show();    
  delay((int) (thisdelay * .5));  
}


void rgb_propeller() {                           //-m27-RGB PROPELLER 
    propeller(thishue % 255, (thishue + 80) % 255, (thishue + 160) % 255, thissat, thissat, thissat);
}

void xmas_propeller() {                           //-m31-RGB PROPELLER 
    propeller(114, 0, 0, 255, 0, 255);
}



void halloween_propeller() {                           //-m32-RGB PROPELLER 
    propeller(214, 114, 18, 255, 255, 255);
}



void kitt() {                                     //-m28-KNIGHT INDUSTIES 2000
  int rand = random(0, TOP_INDEX);
  for(int i = 0; i < rand; i++ ) {
    leds[TOP_INDEX+i] = CHSV(thishue, thissat, 255);
    leds[TOP_INDEX-i] = CHSV(thishue, thissat, 255);
    LEDS.show();
    delay(thisdelay/rand);
  }
  for(int i = rand; i > 0; i-- ) {
    leds[TOP_INDEX+i] = CHSV(thishue, thissat, 0);
    leds[TOP_INDEX-i] = CHSV(thishue, thissat, 0);
    LEDS.show();
    delay((int) (thisdelay * 2)/rand);
  }  
}

void matrix() {                                   //-m29-ONE LINE MATRIX
  int rand = random(0, 100);
  led_copy_temp = leds[LED_COUNT - 1];
  
  //copy_led_array();
  leds[LED_COUNT - 2] = led_copy_temp;

      memmove8( &leds[0], &leds[1], (LED_COUNT - 1) * sizeof( CRGB) );


  if (rand > 90) {
    leds[LED_COUNT - 1] = CHSV(thishue, thissat, 255);
  } else {
    leds[LED_COUNT - 1] = CHSV(thishue, thissat, 0);
  }
  
  LEDS.show();  
  delay((int) (thisdelay * .6));
}


// The following code block represents example code from a stnadalone Halloween sketch.
#define ZOOMING_BEATS_PER_MINUTE 61

// define some shorthands for the Halloween colors
#define PURP 0x6611FF
#define ORAN 0xFF6600
#define GREN 0x00FF11
#define WHIT 0xCCCCCC

// set up a new 16-color palette with the Halloween colors
const CRGBPalette16 HalloweenColors_p
( 
  PURP, PURP, PURP, PURP,
  ORAN, ORAN, ORAN, ORAN,
  PURP, PURP, PURP, PURP,
  GREN, GREN, GREN, WHIT
);


// use the Halloween color palette
CRGBPalette16 gCurrentPalette( HalloweenColors_p );

// you could also use any other pre-defined or custom 
// color palette, e.g., 
// CRGBPalette16 gCurrentPalette( PartyColors_p );
// CRGBPalette16 gCurrentPalette( CloudColors_p );
// CRGBPalette16 gCurrentPalette( LavaColors_p );

// disable palette color 'blending'; i.e., we don't want to
// blend between purple and orange (for example), we just want 
// to use those raw colors directly.
#define BLENDING NOBLEND




// Halloween sketch utility method.
// drawRainbowDashes - draw rainbow-colored 'dashes' of light along the led strip:
//   starting from 'startpos', up to and including 'lastpos'
//   with a given 'period' and 'width'
//   starting from a given hue, which changes for each successive dash by a 'huedelta'
//   at a given saturation and value.
//
//   period = 5, width = 2 would be  _ _ _ X X _ _ _ Y Y _ _ _ Z Z _ _ _ A A _ _ _ 
//                                   \-------/       \-/
//                                   period 5      width 2
//
static void drawRainbowDashes( 
  uint8_t startpos, uint16_t lastpos, uint8_t period, uint8_t width, 
  uint8_t huestart, uint8_t huedelta, uint8_t saturation, uint8_t value)
{
  uint8_t hue = huestart;
  for( uint16_t i = startpos; i <= lastpos; i += period) {
    // Switched from HSV color wheel to color palette
    // Was: CRGB color = CHSV( hue, saturation, value); 
    CRGB color = ColorFromPalette( gCurrentPalette, hue, value, BLENDING);
    
    // draw one dash
    uint16_t pos = i;
    for( uint8_t w = 0; w < width; w++) {
      leds[ pos ] = color;
      pos++;
      if( pos >= LED_COUNT) {
        break;
      }
    }
    
    hue += huedelta;
  }
}


// Halloween sketch utility method.
// discoWorker updates the positions of the dashes, and calls the draw function
//
void discoWorker( 
    uint8_t dashperiod, uint8_t dashwidth, int8_t  dashmotionspeed,
    uint8_t stroberepeats,
    uint8_t huedelta)
 {
  static uint8_t sRepeatCounter = 0;
  static int8_t sStartPosition = 0;
  static uint8_t sStartHue = 0;

  // Always keep the hue shifting a little
  sStartHue += 1;

  // Increment the strobe repeat counter, and
  // move the dash starting position when needed.
  sRepeatCounter = sRepeatCounter + 1;
  if( sRepeatCounter>= stroberepeats) {
    sRepeatCounter = 0;
    
    sStartPosition = sStartPosition + dashmotionspeed;
    
    // These adjustments take care of making sure that the
    // starting hue is adjusted to keep the apparent color of 
    // each dash the same, even when the state position wraps around.
    if( sStartPosition >= dashperiod ) {
      while( sStartPosition >= dashperiod) { sStartPosition -= dashperiod; }
      sStartHue  -= huedelta;
    } else if( sStartPosition < 0) {
      while( sStartPosition < 0) { sStartPosition += dashperiod; }
      sStartHue  += huedelta;
    }
  }

  // draw dashes with full brightness (value), and somewhat
  // desaturated (whitened) so that the LEDs actually throw more light.
  const uint8_t kSaturation = 208;
  const uint8_t kValue = 255;

  // call the function that actually just draws the dashes now
  drawRainbowDashes( sStartPosition, LED_COUNT-1, 
                     dashperiod, dashwidth, 
                     sStartHue, huedelta, 
                     kSaturation, kValue);
}


// Halloween sketch utility method.
void discostrobe()
{
  //Serial.print("discostrobing");
  // First, we black out all the LEDs
  fill_solid( leds, LED_COUNT, CRGB::Black);

  // To achive the strobe effect, we actually only draw lit pixels
  // every Nth frame (e.g. every 4th frame).  
  // sStrobePhase is a counter that runs from zero to kStrobeCycleLength-1,
  // and then resets to zero.  
  const uint8_t kStrobeCycleLength = 4; // light every Nth frame
  static uint8_t sStrobePhase = 0;
  sStrobePhase = sStrobePhase + 1;
  if( sStrobePhase >= kStrobeCycleLength ) { 
    sStrobePhase = 0; 
  }

  // We only draw lit pixels when we're in strobe phase zero; 
  // in all the other phases we leave the LEDs all black.
  if( sStrobePhase  >= 0 ) {

    // The dash spacing cycles from 4 to 9 and back, 8x/min (about every 7.5 sec)
    uint8_t dashperiod= beatsin8( 8/*cycles per minute*/, 4,10);
    // The width of the dashes is a fraction of the dashperiod, with a minimum of one pixel
    uint8_t dashwidth = (dashperiod / 4) + 1;
    
    // The distance that the dashes move each cycles varies 
    // between 1 pixel/cycle and half-the-dashperiod/cycle.
    // At the maximum speed, it's impossible to visually distinguish
    // whether the dashes are moving left or right, and the code takes
    // advantage of that moment to reverse the direction of the dashes.
    // So it looks like they're speeding up faster and faster to the
    // right, and then they start slowing down, but as they do it becomes
    // visible that they're no longer moving right; they've been 
    // moving left.  Easier to see than t o explain.
    //
    // The dashes zoom back and forth at a speed that 'goes well' with
    // most dance music, a little faster than 120 Beats Per Minute.  You
    // can adjust this for faster or slower 'zooming' back and forth.
    uint8_t zoomBPM = ZOOMING_BEATS_PER_MINUTE;
    int8_t  dashmotionspeed = beatsin8( (zoomBPM /2), 1,dashperiod);
    // This is where we reverse the direction under cover of high speed
    // visual aliasing.
    if( dashmotionspeed >= (dashperiod/2)) { 
      dashmotionspeed = 0 - (dashperiod - dashmotionspeed );
    }

    
    // The hueShift controls how much the hue of each dash varies from 
    // the adjacent dash.  If hueShift is zero, all the dashes are the 
    // same color. If hueShift is 128, alterating dashes will be two
    // different colors.  And if hueShift is range of 10..40, the
    // dashes will make rainbows.
    // Initially, I just had hueShift cycle from 0..130 using beatsin8.
    // It looked great with very low values, and with high values, but
    // a bit 'busy' in the middle, which I didnt like.
    //   uint8_t hueShift = beatsin8(2,0,130);
    //
    // So instead I layered in a bunch of 'cubic easings'
    // (see http://easings.net/#easeInOutCubic )
    // so that the resultant wave cycle spends a great deal of time
    // "at the bottom" (solid color dashes), and at the top ("two
    // color stripes"), and makes quick transitions between them.
    uint8_t cycle = beat8(2); // two cycles per minute
    uint8_t easedcycle = ease8InOutCubic( ease8InOutCubic( cycle));
    uint8_t wavecycle = cubicwave8( easedcycle);
    uint8_t hueShift = scale8( wavecycle,130);


    // Each frame of the animation can be repeated multiple times.
    // This slows down the apparent motion, and gives a more static
    // strobe effect.  After experimentation, I set the default to 1.
    uint8_t strobesPerPosition = 1; // try 1..4


    // Now that all the parameters for this frame are calculated,
    // we call the 'worker' function that does the next part of the work.
    discoWorker( dashperiod, dashwidth, dashmotionspeed, strobesPerPosition, hueShift);
  }  
  // send the 'leds' array out to the actual LED strip
  FastLED.show();
   // delay just enough to keep a steady frame rate, e.g 100 FPS
  delayToSyncFrameRate( max(5,50 - (int) thisdelay / 2));
}


// Halloween sketch utility method.
// delayToSyncFrameRate - delay how many milliseconds are needed
//   to maintain a stable frame rate.
static void delayToSyncFrameRate( uint8_t framesPerSecond)
{
  static uint32_t msprev = 0;
  uint32_t mscur = millis();
  uint16_t msdelta = mscur - msprev;
  uint16_t mstargetdelta = 1000 / framesPerSecond;
  if( msdelta < mstargetdelta) {
    delay( mstargetdelta - msdelta);
  }
  msprev = mscur;
}


// End block of code related to standalone Halloween sketch.
void new_rainbow_loop(){                       //-m88-RAINBOW FADE FROM FAST_SPI2
  ihue -= 1;
  fill_rainbow( leds, LED_COUNT, ihue );
  LEDS.show();
  delay(thisdelay);
}

//-Many of the following cases were commented out to prevent the strip's hue, saturation, and delay from resetting when another mode is chosen.
//-The responsibility of changing these parameters prior to a mode change would preferibly be done in the bluetooth controlling device fia multiple commands in the same byte stream.
void change_mode(int newmode){
  thissat = 255;
  switch (newmode) {
    case 0: one_color_all(0,0,0); LEDS.show(); break;   //---ALL OFF
    case 1: one_color_all(255,255,255); LEDS.show(); break;   //---ALL ON
//    case 2: thisdelay = 20; break;                      //---STRIP RAINBOW FADE
//    case 3: thisdelay = 20; thisstep = 10; break;       //---RAINBOW LOOP
//    case 4: thisdelay = 20; break;                      //---RANDOM BURST
//    case 5: thisdelay = 20; thishue = 0; break;         //---CYLON v1
//    case 6: thisdelay = 40; thishue = 0; break;         //---CYLON v2
//    case 7: thisdelay = 40; thishue = 0; break;         //---POLICE LIGHTS SINGLE
//    case 8: thisdelay = 40; thishue = 0; break;         //---POLICE LIGHTS SOLID
//    case 9: break;         //---STRIP FLICKER
    case 9: idex=TOP_INDEX; break;         //---BOMBER
//    case 10: thisdelay = 15; thishue = 0; break;        //---PULSE COLOR BRIGHTNESS
//    case 11: thisdelay = 15; thishue = 0; break;        //---PULSE COLOR SATURATION
//    case 12: thisdelay = 60; thishue = 180; break;      //---VERTICAL SOMETHING
    case 13: idex = 1; break;                  //---theater chase - RULE 30 (RED)
    case 14: idex = 0; break;                     //---MARCH RANDOM COLORS
    case 15: break;                     //---MARCH RWB COLORS
    case 16: one_color_all(0,0,0); break;       //---RADIATION SYMBOL
//    case 16: thisdelay = 60; thishue = 95; break;       //---RADIATION SYMBOL
//    //---PLACEHOLDER FOR COLOR LOOP VAR DELAY VARS
//    case 19: thisdelay = 35; thishue = 180; break;      //---SIN WAVE BRIGHTNESS
//    case 20: thisdelay = 100; thishue = 0; break;       //---POP LEFT/RIGHT
//    case 21: thisdelay = 100; thishue = 180; break;     //---QUADRATIC BRIGHTNESS CURVE
//    //---PLACEHOLDER FOR FLAME VARS
//    case 23: thisdelay = 50; thisstep = 15; break;      //---VERITCAL RAINBOW
    case 24: idex=0; break;                     //---PACMAN
//    case 25: thisdelay = 35; break;                     //---RANDOM COLOR POP
//    case 26: thisdelay = 25; thishue = 0; break;        //---EMERGECNY STROBE
//    case 27: thisdelay = 25; thishue = 0; break;        //---RGB PROPELLER
//    case 28: thisdelay = 100; thishue = 0; break;       //---KITT
//    case 29: thisdelay = 50; thishue = 95; break;       //---MATRIX RAIN
//    case 50: thisdelay = 100; break;                    //---MARCH STRIP NOW CCW
//    case 51: thisdelay = 100; break;                    //---MARCH STRIP NOW CCW  
//    case 88: thisdelay = 5; break;                      //---NEW RAINBOW LOOP
    case 101: one_color_all(255,0,0); LEDS.show(); break;   //---ALL RED
    case 102: one_color_all(0,255,0); LEDS.show(); break;   //---ALL GREEN
    case 103: one_color_all(0,0,255); LEDS.show(); break;   //---ALL BLUE
    case 104: one_color_all(255,255,0); LEDS.show(); break;   //---ALL COLOR X
    case 105: one_color_all(0,255,255); LEDS.show(); break;   //---ALL COLOR Y
    case 106: one_color_all(255,0,255); LEDS.show(); break;   //---ALL COLOR Z
  }

  bouncedirection = 0;
  //one_color_all(0,0,0);
  ledMode = newmode;
}



//
//void change_mode(int newmode){
//  thissat = 255;
//  switch (newmode) {
//    case 0: one_color_all(0,0,0); LEDS.show(); break;   //---ALL OFF
//    case 1: one_color_all(255,255,255); LEDS.show(); break;   //---ALL ON
//    case 2: thisdelay = 20; break;                      //---STRIP RAINBOW FADE
//    case 3: thisdelay = 20; thisstep = 10; break;       //---RAINBOW LOOP
//    case 4: thisdelay = 20; break;                      //---RANDOM BURST
//    case 5: thisdelay = 20; thishue = 0; break;         //---CYLON v1
//    case 6: thisdelay = 40; thishue = 0; break;         //---CYLON v2
//    case 7: thisdelay = 40; thishue = 0; break;         //---POLICE LIGHTS SINGLE
//    case 8: thisdelay = 40; thishue = 0; break;         //---POLICE LIGHTS SOLID
////    case 9: thishue = 160; thissat = 50; break;         //---STRIP FLICKER
//    case 9: idex=0; break;         //---STRIP FLICKER
//    case 10: thisdelay = 15; thishue = 0; break;        //---PULSE COLOR BRIGHTNESS
//    case 11: thisdelay = 15; thishue = 0; break;        //---PULSE COLOR SATURATION
//    case 12: thisdelay = 60; thishue = 180; break;      //---VERTICAL SOMETHING
//    case 13:  break; // had idex = 1;, was cauisng breakage                   //---theater chase - RULE 30 (RED)
//    case 14: idex = 0; break;                     //---MARCH RANDOM COLORS
//    case 15: break;                     //---MARCH RWB COLORS
//    case 16: thisdelay = 60; thishue = 95; break;       //---RADIATION SYMBOL
//    //---PLACEHOLDER FOR COLOR LOOP VAR DELAY VARS
//    case 19: thisdelay = 35; thishue = 180; break;      //---SIN WAVE BRIGHTNESS
//    case 20: thisdelay = 100; thishue = 0; break;       //---POP LEFT/RIGHT
//    case 21: thisdelay = 100; thishue = 180; break;     //---QUADRATIC BRIGHTNESS CURVE
//    //---PLACEHOLDER FOR FLAME VARS
//    case 23: thisdelay = 50; thisstep = 15; break;      //---VERITCAL RAINBOW
//    case 24: idex=0; break;                     //---PACMAN
//    case 25: thisdelay = 35; break;                     //---RANDOM COLOR POP
//    case 26: thisdelay = 25; thishue = 0; break;        //---EMERGECNY STROBE
//    case 27: thisdelay = 25; thishue = 0; break;        //---RGB PROPELLER
//    case 28: thisdelay = 100; thishue = 0; break;       //---KITT
//    case 29: thisdelay = 50; thishue = 95; break;       //---MATRIX RAIN
//    case 50: thisdelay = 100; break;                    //---MARCH STRIP NOW CCW
//    case 51: thisdelay = 100; break;                    //---MARCH STRIP NOW CCW  
//    case 88: thisdelay = 5; break;                      //---NEW RAINBOW LOOP
//    case 101: one_color_all(255,0,0); LEDS.show(); break;   //---ALL RED
//    case 102: one_color_all(0,255,0); LEDS.show(); break;   //---ALL GREEN
//    case 103: one_color_all(0,0,255); LEDS.show(); break;   //---ALL BLUE
//    case 104: one_color_all(255,255,0); LEDS.show(); break;   //---ALL COLOR X
//    case 105: one_color_all(0,255,255); LEDS.show(); break;   //---ALL COLOR Y
//    case 106: one_color_all(255,0,255); LEDS.show(); break;   //---ALL COLOR Z
//  }
////  Serial.print("change mode ");
////  Serial.println(newmode);
//  bouncedirection = 0;
//  //one_color_all(0,0,0);
//  ledMode = newmode;
//}


//------------------SETUP------------------
void setup()  
{
  Serial.begin(SERIAL_BAUDRATE);      // SETUP HARDWARE SERIAL (USB OR BLUETOOTH)
  Serial.setTimeout(SERIAL_TIMEOUT);

  btSerial.begin(SERIAL_BAUDRATE);    // SETUP SOFTWARE SERIAL (USB OR BLUETOOTH)
  btSerial.setTimeout(SERIAL_TIMEOUT);

  LEDS.setBrightness(max_bright);

  //BELOW ARE VARIOUS IMPLEMTNATIONS USING DIFFERENT TYPES OF LED STRIPS
  //LEDS.addLeds<SM16716>(leds, LED_COUNT);
  //LEDS.addLeds<UCS1903, 13>(leds, LED_COUNT);
  //LEDS.addLeds<LPD8806, 10, 11 BGR>(leds, LED_COUNT);
  //LEDS.addLeds<TM1809, 13>(leds, LED_COUNT);
  //LEDS.addLeds<WS2801, LED_CK, LED_DT, RBG, DATA_RATE_MHZ(1)>(leds, LED_COUNT);
  LEDS.addLeds<WS2811, LED_DT, GRB>(leds, LED_COUNT);

  one_color_all(0,0,0); //-CLEAR STRIP
  LEDS.show();
}


//------------------MAIN LOOP------------------
void loop() {
  switch (ledMode) {
      case 999: break;
      case  2: rainbow_fade(); break;
      case  3: rainbow_loop(); break;
      case  4: random_burst(); break;
      case  5: color_bounce(); break;
      case  6: color_bounceFADE(); break;
      case  7: ems_lightsONE(); break;
      case  8: ems_lightsALL(); break;
      case  9: bomber(); break;
      case 10: pulse_one_color_all(); break;
      case 11: pulse_one_color_all_rev(); break;
      case 12: fade_vertical(); break;
      case 13: theater_chase(); break;
      case 14: color_implode(); break;
      case 15: rwb_march(); break;
      case 16: radiation(); break;
      case 17: color_loop_vardelay(); break;
      case 18: white_temps(); break;
      case 19: sin_bright_wave(); break;
      case 20: pop_horizontal(); break;
      case 21: quad_bright_curve(); break;
      case 22: flame(); break;
      case 23: rainbow_vertical(); break;
      case 24: color_explode(); break;
      case 25: random_color_pop(); break;
      case 26: ems_lightsSTROBE(); break;
      case 27: rgb_propeller(); break;
      case 28: kitt(); break;
      case 29: matrix(); break;      
      case 30: discostrobe(); break;
      case 31: xmas_propeller(); break;
      case 32: halloween_propeller(); break;    
    }

  //---PROCESS HARDWARE SERIAL COMMANDS AND ARGS
  while (Serial.available() > 0) {inbyte = Serial.read();
    switch(inbyte) {
    case 59: break; //---BREAK IF INBYTE = ';'
    case 108:      //---"l" - SET SINGLE LED VALUE RGB
      thisindex = Serial.parseInt();
      thisRED = Serial.parseInt();
      thisGRN = Serial.parseInt();
      thisBLU = Serial.parseInt();      
      if (ledMode != 999) {
        ledMode = 999;
        one_color_all(0,0,0);}        
      leds[thisindex].setRGB( thisRED, thisGRN, thisBLU);
      break;
    case 118:      //---"v" - SET SINGLE LED VALUE HSV
      thisindex = Serial.parseInt();
      thishue = Serial.parseInt();
      thissat = Serial.parseInt();
      //thisVAL = Serial.parseInt();      
      if (ledMode != 999) {
        ledMode = 999;
        one_color_all(0,0,0);}
      leds[thisindex] = CHSV(thishue, thissat, 255);
      break;      
    case 100:      //---"d" - SET DELAY VAR
      thisarg = Serial.parseInt();
      thisdelay = thisarg;
      break;
    case 115:      //---"s" - SET STEP VAR
      thisarg = Serial.parseInt();
      thisstep = thisarg;
      break;
    case 104:      //---"h" - SET HUE VAR
      thisarg = Serial.parseInt();
      thishue = thisarg;
      break;
    case 116:      //---"t" - SET SATURATION VAR
      thisarg = Serial.parseInt();
      thissat = thisarg;
      break;
    case 98:      //---"b" - SET MAX BRIGHTNESS
      max_bright = Serial.parseInt();
      LEDS.setBrightness(max_bright);
      break;      
    case 109:      //---"m" - SET MODE
      thisarg = Serial.parseInt();      
      change_mode(thisarg);
      break;
    case 99:      //---"c" - CLEAR STRIP
      one_color_all(0,0,0);
      break;
    case 97:      //---"a" - SET ALL TO ONE COLOR BY HSV 0-255
      thisarg = Serial.parseInt();
      one_color_allHSV(thisarg);
      break;
    case 122:      //---"z" - COMMAND TO 'SHOW' LEDS
      LEDS.show();
      break;
    case 81:      //---"Q" - COMMAND RETURN VERSION NUMBER
      Serial.print(VERSION_NUMBER);
      break;
    }
  }

  //---PROCESS SOFTWARE SERIAL COMMANDS AND ARGS
  //---NOTE* USE OF SOFTWARE SERIAL WITH A HIGH NUMBER OF LEDS (MORE THAN 120) MAY CAUSE LATENCY AND LOSS OF BYTES WITH SOFTWARE SERIAL IMPLEMENTATIONS
  while (btSerial.available() > 0) {
    
    inbyte = btSerial.read();
    if (btSerial.overflow()){Serial.println("SO!");}
    switch(inbyte) {
    case 59:
      break; //---BREAK IF INBYTE = ';'
    case 108:      //---"l" - SET SINGLE LED VALUE
      thisindex = btSerial.parseInt();
      thisRED = btSerial.parseInt();
      thisGRN = btSerial.parseInt();
      thisBLU = btSerial.parseInt();    
      if (ledMode != 999) {
        ledMode = 999;
        one_color_all(0,0,0);}        
      leds[thisindex].setRGB( thisRED, thisGRN, thisBLU);
      break;
    case 100:      //---"d" - SET DELAY VAR
      thisarg = btSerial.parseInt();
      thisdelay = thisarg;
      break;
    case 115:      //---"s" - SET STEP VAR
      thisarg = btSerial.parseInt();
      thisstep = thisarg;
      break;
    case 104:      //---"h" - SET HUE VAR
      thisarg = btSerial.parseInt();
      thishue = thisarg;
      break;
    case 116:      //---"t" - SET SATURATION VAR
      thisarg = btSerial.parseInt();
      thissat = thisarg;
      break;
    case 98:      //---"b" - SET MAX BRIGHTNESS
      max_bright = btSerial.parseInt();
      LEDS.setBrightness(max_bright);
      break;      
    case 109:      //---"m" - SET MODE
      thisarg = btSerial.parseInt();    
      change_mode(thisarg);
      break;
    case 99:      //---"c" - CLEAR STRIP
      one_color_all(0,0,0);
      break;
    case 97:      //---"a" - SET ALL TO ONE COLOR BY HSV 0-255
      thisarg = btSerial.parseInt();
      one_color_allHSV(thisarg);
      break;
    case 122:      //---"z" - COMMAND TO 'SHOW' LEDS
      LEDS.show();
      break;        
    }
  }

}
