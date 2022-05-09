#pragma once

void setInterval(unsigned long delta);
bool startLapse();
void stopLapse();
bool inLapse();
bool processLapse(unsigned long dt);
