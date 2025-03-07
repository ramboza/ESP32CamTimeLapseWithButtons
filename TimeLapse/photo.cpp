#include <stdio.h>

#include "Arduino.h"
#include "camera.h"
#include "file.h"

long photoIndex = -1;
bool isRunning = false;

const char photoDir[] = "/photos";

long getLastIndex() {
	if (!fileExists(photoDir)) {
		createDir(photoDir);
	}

	char path[32];
	for (int i = 0; i < LONG_MAX; i++) {
		sprintf(path, "%s/pic%05d.jpg", photoDir, i);
		if (!fileExists(path)) {
			return i;
		}
	}
	return 0;
}

bool makePhoto() {
	if (isRunning) return false;
	// a bit ugly, but fine for testing ;)
	if (photoIndex = -1) photoIndex = getLastIndex();

	initCamera();

	esp_err_t res = ESP_OK;
	camera_fb_t *fb = esp_camera_fb_get();
	if (!fb) {
		Serial.println("Camera capture failed");
		esp_camera_deinit();
		return false;
	}

	char path[32];
	sprintf(path, "%s/pic%05d.jpg", photoDir, photoIndex++);
	Serial.println(path);

	if (!writeFile(path, (const unsigned char *)fb->buf, fb->len)) {
		esp_camera_deinit();
		return false;
	}
	esp_camera_fb_return(fb);
	esp_camera_deinit();
	return true;
}
