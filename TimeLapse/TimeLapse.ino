#include <WiFi.h>
#include "file.h"
#include "camera.h"
#include "lapse.h"
#include "photo.h"

const char *ssid = "...";
const char *password = "...";
const int PIN_BUTTON_TX = 1;
const int PIN_BUTTON_RX = 3;

void startCameraServer();

void setup()
{
	Serial.begin(115200);
	Serial.setDebugOutput(true);
	Serial.println();
	initFileSystem();
	initCamera();

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");
	startCameraServer();
	Serial.print("Camera Ready! Use 'http://");
	Serial.print(WiFi.localIP());
	Serial.println("' to connect");

	Serial.end();
	pinMode(PIN_BUTTON_TX, INPUT_PULLUP);
	pinMode(PIN_BUTTON_RX, INPUT_PULLUP);
}

void loop()
{
	unsigned long t = millis();
	static unsigned long ot = 0;
	unsigned long dt = t - ot;
	ot = t;
	processLapse(dt);

	button_RX_Photo();
	button_TX_Lapse();
}


/////////////////////////////////////////////////////////////////
// De-bounce for buttons.
void button_TX_Lapse(void) {
	static uint16_t btndbc = 0;

	btndbc=(btndbc<<1) | digitalRead(PIN_BUTTON_TX) | 0xe000;
	// Switch / toggle
	if (btndbc==0xf000) {
		if (!inLapse()){
			startLapse();
		}else{
			stopLapse();
		}
		btndbc = 0xe000;
	}
}

void button_RX_Photo(void) {
	static uint16_t btndbc = 0;

	btndbc=(btndbc<<1) | digitalRead(PIN_BUTTON_RX) | 0xe000;

	if (btndbc==0xf000) { 
		makePhoto();
		btndbc = 0xe000;
	}
}
