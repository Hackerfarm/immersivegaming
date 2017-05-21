#include <Adafruit_NeoPixel.h>
#define LED_PIN            6
#define MAX_LED_VALUE      64
#define NUM_LEDS           8

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(8, LED_PIN, NEO_GRB + NEO_KHZ800);

void show_hp(int hp, int maxhp=NUM_LEDS)
{
  uint32_t green = pixels.Color(0,MAX_LED_VALUE,0);
  uint32_t red   = pixels.Color(MAX_LED_VALUE,0,0);
  uint32_t off   = pixels.Color(0,0,0);
  
  for(int i=0;i<hp;i++)
  {
    pixels.setPixelColor(i, green);
  }
  for(int i=hp;i<maxhp;i++)
  {
    pixels.setPixelColor(i, red);
  }
  for(int i=maxhp;i<NUM_LEDS;i++)
  {
    pixels.setPixelColor(i, off);
  }
  pixels.show();
}

void setup() {
  pixels.begin(); // NeoPixel init
  
}

int current_hp=0;
int dir = 1;

void loop() {
  delay(200);
  current_hp+=dir;
  if(current_hp==6)
  {
    dir=-1;
  }
  if(current_hp==0)
  {
    dir=1;
  }
  show_hp(current_hp, 6);
}
