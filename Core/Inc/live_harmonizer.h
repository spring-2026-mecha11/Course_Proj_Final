#ifndef LIVE_HARMONIZER_H
#define LIVE_HARMONIZER_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

void LiveHarmonizer_Reset(void);
void LiveHarmonizer_Task(uint32_t now_ms);

extern volatile float live_harmonizer_kp;
extern volatile float live_harmonizer_ki;

#endif
