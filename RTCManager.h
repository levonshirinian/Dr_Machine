#ifndef RTC_MANAGER_H
#define RTC_MANAGER_H

#include <Arduino.h>
#include <RTClib.h>

void initializeRTC();
String getFormattedDateTime();

#endif