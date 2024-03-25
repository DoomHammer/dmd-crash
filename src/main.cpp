#include <Arduino.h>

#include <DMD_RGB.h>

#include "gfx/mapa.h"
#include "gfx/statki.h"

#define DISPLAYS_ACROSS 2
#define DISPLAYS_DOWN 1
#define DISPLAY_TYPE RGB64x64plainS32

#define ENABLE_DUAL_BUFFER true

#define DMD_PIN_A 6
#define DMD_PIN_B 7
#define DMD_PIN_C 8
#define DMD_PIN_D 9
#define DMD_PIN_E 10

#define DMD_PIN_nOE 13
#define DMD_PIN_SCLK 12

#define DMD_PIN_CLK 11
#define DMD_PIN_R0 0
#define DMD_PIN_G0 1
#define DMD_PIN_B0 2
#define DMD_PIN_R1 3
#define DMD_PIN_G1 4
#define DMD_PIN_B1 5

class Display {
public:
  Display();
  void init(uint8_t brightness = 127);

  void swapBuffers(boolean copy);
  void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w,
                     int16_t h);

private:
  DMD_RGB_BASE *display = nullptr;
};

Display::Display() {
  uint8_t mux_list[] = {DMD_PIN_A, DMD_PIN_B, DMD_PIN_C, DMD_PIN_D, DMD_PIN_E};
  uint8_t custom_rgbpins[] = {DMD_PIN_CLK, DMD_PIN_R0, DMD_PIN_G0, DMD_PIN_B0,
                              DMD_PIN_R1,  DMD_PIN_G1, DMD_PIN_B1};
  display = new DMD_RGB<DISPLAY_TYPE, COLOR_4BITS>(
      mux_list, DMD_PIN_nOE, DMD_PIN_SCLK, custom_rgbpins, DISPLAYS_ACROSS,
      DISPLAYS_DOWN, ENABLE_DUAL_BUFFER);
}

void Display::init(uint8_t brightness) {
  display->init();
  display->setBrightness(brightness);
}

void Display::swapBuffers(boolean copy) { display->swapBuffers(copy); }

void Display::drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[],
                            int16_t w, int16_t h) {
  display->drawRGBBitmap(x, y, bitmap, w, h);
}

Display display;

uint32_t ptime = 0;
uint16_t screen[128 * 64];
char buff[50];
int heading = 0;
float viewpos = 0;

#define HEADSTEP	(512/32)

void setup(void) {
	uint8_t brightness = 10;
	display.init(brightness);
	ptime = millis();
}

void loop()
{
	heading &= 31;

  sprintf(buff, "%dh", heading);
  Serial.println(buff);

	// ================================================ Gameplay ================================================
	uint32_t time = millis();
	uint32_t delta = time - ptime;
	ptime = time;

  int vdelta = (heading * HEADSTEP) - (int(floor(viewpos+0.5f)) & (32*HEADSTEP - 1));
  if( vdelta <= -16*HEADSTEP ) vdelta += 32*HEADSTEP;
  if( vdelta >= 16*HEADSTEP ) vdelta -= 32*HEADSTEP;
  viewpos += vdelta * (1.0f - pow(0.0001f, delta/1000.f));

	// ================================================ Draw ================================================
	// Sea
	int pos = floor(viewpos+0.5f);
	uint16_t *dst = screen;
	for (int y = 0; y < 64; y++)
		for (int x = 0; x < 128; x++) {
			int sx = (x + pos) & 511;
			*dst++ = mapa[sx + y * 512];
		}

	float ftime = time/1000.f;

	uint8_t *mask = STATKI;
	dst = screen;
	for (int y = 0; y < 64; y++)
		for (int x = 0; x < 128; x++) {
			if( !*mask++ )
				*dst = 0;
			dst++;
		}

	display.drawRGBBitmap(0, 0, screen, 128, 64);
	display.swapBuffers(true);
}
