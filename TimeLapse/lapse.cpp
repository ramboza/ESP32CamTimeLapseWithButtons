#include <stdio.h>

#include "Arduino.h"
#include "camera.h"
#include "file.h"

unsigned long fileIndex = 0;
unsigned long lapseIndex = 0;
unsigned long frameInterval = 20000;
bool lapseRunning = false;
unsigned long lastFrameDelta = 0;

void setInterval(unsigned long delta) {
	frameInterval = delta;
}

bool inLapse() {
	return lapseRunning;
}

void stopLapse() {
	lapseRunning = false;
}

bool startLapse() {
        if (lapseRunning) return true;

        fileIndex = 0;
        char path[32];
        for (; lapseIndex < 10000; lapseIndex++) {
                sprintf(path, "/lapse%03d", lapseIndex);
                if (!fileExists(path)) {
                        createDir(path);
                        lastFrameDelta = 0;
                        lapseRunning = true;
                        return true;
                }
        }
        return false;
}


bool processLapse(unsigned long dt) {
        if (!lapseRunning) return false;

        lastFrameDelta += dt;
        if (lastFrameDelta >= frameInterval) {
		digitalWrite(33, HIGH);

		initCamera();

		lastFrameDelta -= frameInterval;
                camera_fb_t *fb = esp_camera_fb_get();
                if (!fb) {
                        Serial.println("Camera capture failed");
			esp_camera_deinit();
                        return false;
                }

                char path[32];
                sprintf(path, "/lapse%03d/pic%05d.jpg", lapseIndex, fileIndex);
                Serial.println(path);
                if (!writeFile(path, (const unsigned char *)fb->buf, fb->len)) {
                        lapseRunning = false;
			esp_camera_deinit();
                        return false;
                }
                fileIndex++;
                esp_camera_fb_return(fb);
		esp_camera_deinit();
		digitalWrite(33, LOW);
        }
        return true;
}
