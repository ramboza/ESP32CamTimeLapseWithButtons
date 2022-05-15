#include <WiFi.h>

#include "camera.h"
#include "file.h"
#include "lapse.h"
#include "photo.h"

const char *ssid = "WIFI_SSID";
const char *password = "CHANGE_IT_ON_YOUR_WIFIPASSWORD";
const int PIN_BUTTON_TX = 1;
const int PIN_BUTTON_RX = 3;
int wifi_attempts = 15;

void startCameraServer();

void setup() {
        Serial.begin(115200);
        Serial.setDebugOutput(true);
        Serial.println();

        initFileSystem();
        initCamera();

        WiFi.begin(ssid, password);

	// no wifi, no problem
	Serial.println("\nConnecting to Wi-Fi...\n");
        for (int i = 1; i < wifi_attempts; i++) {
		if (WiFi.status() != WL_CONNECTED) {
                	delay(500);
                	Serial.printf("retry %d of %d\n", i, wifi_attempts);
		} else {
			startCameraServer();
        		Serial.printf("\nCamera Ready!\n\nUse 'http://%s' to connect\n", WiFi.localIP().toString());
			break;
		}
	}

       	Serial.println("\nReconfiguring UART RX/TX as Photo/TimeLapse buttons...\ndisconnected");
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
