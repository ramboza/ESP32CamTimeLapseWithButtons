#include <WiFi.h>
#include <WiFiGeneric.h>

#include "app_httpd.h"
#include "camera.h"
#include "config.h"
#include "file.h"
#include "lapse.h"
#include "photo.h"

uint32_t wifi_test_counter;

void initWifi() {
	WiFi.begin(SSID, PASSWORD);

	// no wifi, no problem
	Serial.println("\nInit Wi-Fi...\n");
	if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
		Serial.println("cannot connect to wifi");
		WiFi.disconnect(true, false);
	} else {
		startCameraServer();
		Serial.printf("\nCamera Ready!\n\nUse 'http://%s' to connect\n", WiFi.localIP().toString());
	}
}


/////////////////////////////////////////////////////////////////
// De-bounce for buttons.
void button_TX_Lapse(void) {
	static uint16_t btndbc = 0;

	btndbc = (btndbc << 1) | digitalRead(PIN_BUTTON_TX) | 0xe000;
	// Switch / toggle
	if (btndbc == 0xf000) {
		if (!inLapse()) {
			blinkN(10);
			startLapse();
		} else {
			blinkN(3);
			stopLapse();
		}
		btndbc = 0xe000;
	}
}


void button_RX_Photo(void) {
	static uint16_t btndbc = 0;

	btndbc = (btndbc << 1) | digitalRead(PIN_BUTTON_RX) | 0xe000;

	if (btndbc == 0xf000) {
		// flash light once, make photo, turn led off
		digitalWrite(33, LOW);
		makePhoto();
		digitalWrite(33, HIGH);
		btndbc = 0xe000;
	}
}


void blinkN(int times) {
	for (int i = 0; i < times; i++) {
		digitalWrite(33, LOW);
		delay(333);
		digitalWrite(33, HIGH);
		delay(333);
	}
}


void setup() {
	Serial.begin(115200);
	//Serial.setDebugOutput(true);

	Serial.println("\nInit SD filesystem ...");
	initFileSystem();

	// init on demand
	//Serial.println("\nInit camera ...");
	//initCamera();

	initWifi();

	Serial.println("\nReconfiguring UART RX/TX as Photo/TimeLapse buttons...\ndisconnected");
	Serial.flush();
	Serial.end();

	pinMode(PIN_BUTTON_TX, INPUT_PULLUP);
	pinMode(PIN_BUTTON_RX, INPUT_PULLUP);

	pinMode(33, OUTPUT);
	digitalWrite(33, LOW);  // reallly is high :)
}


void loop() {
	unsigned long t = millis();
	static unsigned long ot = 0;
	unsigned long dt = t - ot;
	ot = t;
	processLapse(dt);

	button_RX_Photo();
	button_TX_Lapse();

	/* abstract interval :) */
	if (wifi_test_counter > WIFI_TEST_INTERVAL_SEC * 1000) {
		wifi_test_counter = 0;
		if (!WiFi.isConnected()) {
			initWifi();
		}
	}
	delay(10);
	wifi_test_counter += 10;
}

