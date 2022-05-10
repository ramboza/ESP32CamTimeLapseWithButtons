#include <WiFi.h>

#include "camera.h"
#include "file.h"
#include "lapse.h"
#include "photo.h"

const char *ssid = "...";
const char *password = "........";
const int PIN_BUTTON_TX = 1;
const int PIN_BUTTON_RX = 3;

void startCameraServer();

void setup() {
        Serial.begin(115200);
        Serial.setDebugOutput(true);
        Serial.println();
        initFileSystem();
        initCamera();

        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
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

	pinMode(33, OUTPUT);
	digitalWrite(33, LOW); // in real is high :)
}

void loop() {
        unsigned long t = millis();
        static unsigned long ot = 0;
        unsigned long dt = t - ot;
        ot = t;
        processLapse(dt);

        button_RX_Photo();
        button_TX_Lapse();

	delay(10);
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
	for (int i = 0; i < times; i++){
		digitalWrite(33, LOW); 
		delay(333);
		digitalWrite(33, HIGH);
		delay(333);
	}
}
