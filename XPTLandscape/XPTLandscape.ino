#define TEST_LAYOUT
/* This code is for Landscape mode and works on my TJCTM24028-SPI */
/* This isn't the cleanest code example.
   It's a modification of the XPTPaint example, just to get
   Landscape mode working (which required the rotation AND
   swapping the width and height).
   When in TEST_LAYOUT mode we create an array of buttons,
   but to leave the center space empty and drawable we skip
   a couple rows.  I didn't feel like changing the default
   XPTPaint example to handle two rows properly, at this time,
   nor to handle the button presses, so TEST_LAYOUT mode will
   let you draw but won't let you clear the drawing (because
   the button presses aren't handled. */

#include <Arduino.h>
#include <SPI.h>

#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>

#define sp(v)  Serial.print(v)    // convenience
#define spl(v) Serial.println(v)

// Modify the following two lines to match your hardware
// Also, update calibration parameters below, as necessary

// For the esp shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=*/ 4, /*irq=*/ 5);

Adafruit_GFX_Button button;

#ifdef TEST_LAYOUT
void draw_test_layout_buttons() {
	int lwi=tft.width(), lhi=tft.height();
	sp("Display size: x:"); sp(lwi);
	sp(" y:"); spl(lhi);
	const int bwi=100, bhi=40;
	const int padx=2, pady=2;
	const int bcntx = lwi/(bwi+padx); // button count, centered,
	const int bcnty = lhi/(bhi+pady); // with room for padding
	Adafruit_GFX_Button buttons[bcnty][bcntx];
	sp("Button counts: y:"); sp(bcnty);
	sp(" x:"); spl(bcntx);
	for (int y=0; y<bcnty; y++) {
		for (int x=0; x<bcntx; x++) {
			int xpos = (bwi/2) + (x * (bwi + padx));
			int ypos = (bhi/2) + (y * (bhi + pady));
			char label[15]="Clr ";
			sprintf(label, "%d.%d", xpos, ypos);
			buttons[y][x].initButton(&tft, xpos, ypos, bwi, bhi,
			                  ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW,
			                  label, 2);
			buttons[y][x].drawButton();
		}
		if (y==0) y+=3; // skip rows for this test (wasting some button objects too)
	}
}
#endif

void setup() {
	delay(1000);
	
	Serial.begin(115200);
	SPI.setFrequency(ESP_SPI_FREQ);

	tft.begin();
	tft.setRotation(1); // landscape (for me)
	// This was used for XPTPaint which is in portrait mode on my display
	//touch.begin(tft.width(), tft.height());  // Must be done before setting rion
	// If you set this to width/height it'll be stretched.
	// This was proper for Landscape mode
	touch.begin(tft.height(), tft.width());  // Must be done before setting rion
	touch.setRotation(touch.ROT90);
	Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
	tft.fillScreen(ILI9341_BLACK);
	// Replace these for your screen module
	touch.setCalibration(209, 1759, 1775, 273);
	// 320 x 240
	#ifdef TEST_LAYOUT
		draw_test_layout_buttons();
	#else
		button.initButton(&tft, 100, 100, 70, 40, ILI9341_DARKCYAN, ILI9341_BLUE, ILI9341_GREENYELLOW, "Clear", 2);
		button.drawButton();
	#endif
	
}

static uint16_t prev_x = 0xffff, prev_y = 0xffff;

void loop() {
	uint16_t x, y;
	if (touch.isTouching()) {
		touch.getPosition(x, y);
//	Serial.print("x ="); Serial.print(x); Serial.print(" y ="); Serial.println(y);
		if (prev_x == 0xffff) {
			tft.drawPixel(x, y,ILI9341_BLUE);
		} else {
			tft.drawLine(prev_x, prev_y, x, y,ILI9341_BLUE);
		}
		prev_x = x;
		prev_y = y;
	} else {
		prev_x = prev_y = 0xffff;
	}
	
	#ifndef TEST_LAYOUT // no presses with test layout mode
		button.press(button.contains(x, y)); // tell the button it is pressed
		

	// now we can ask the buttons if their state has changed
		if (button.justReleased()) {
			tft.fillScreen(ILI9341_BLACK);
			button.drawButton(); // draw normal
		}

		if (button.justPressed()) {
			button.drawButton(true); // draw invert!
		}
	#endif

	delay(20);
}
