// ----- AP (точка доступа) -------
#define AP_SSID "Obvilion RGB"
#define AP_PASS "12345678"
#define AP_IP   IPAddress(192, 168, 4, 100)

// ----- TCP локальный сокет -------
#define TCP_SERVER_PORT             9922
#define TCP_SERVER_MAX_CLIENTS      3
byte    TCP_SERVER_FIRST_BYTES[3] = { 222, 111, 222 };

// ----- AC (подключение к WiFi) -----
#define AC_HOSTNAME "Obvilion RGB"

// ----- Obvilion Home -----
#define OBV_HOME_ADDRESS        "home.obvilion.ru"
#define OBV_HOME_PORT           25055
#define OBV_HOME_DEVICE         "BicycleRGB"
#define OBV_HOME_DEVICE_VERSION 1

// ----- Пины цветов RGB лент -------
#define R_LED D2
#define G_LED D1
#define B_LED D8
#define I_LED D4

#define DEBUG true

// Imports
#include <ESP8266WiFi.h>
#include "Config.h"
#include "Timer.h"
#include "WiFiStatus.h"

typedef byte color;

union Int2Bytes {
  int16_t value;
  byte bytes[2];
};
union Long2Bytes {
  long value;
  byte bytes[4];
};
union Float2Bytes {
  float value;
  byte  bytes[4];
};

ICACHE_RAM_ATTR void ISR (void) {
  timer0_write(ESP.getCycleCount() + (80000000L * 2 / 100)); 
  
  tickTimerAnimation();
}

void setup() {
  #ifdef DEBUG
    Serial.begin(19200);
  #endif
  
  pinMode(R_LED, OUTPUT); 
  pinMode(G_LED, OUTPUT);
  pinMode(B_LED, OUTPUT);
  pinMode(I_LED, OUTPUT);

  digitalWrite(I_LED, 1);
  
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(ISR);
  timer0_write(ESP.getCycleCount() + (80000000L * 2 / 100));
  interrupts();
  
  loadConfig();
  loadWiFi();
  loadTCPServer();
  loadAnimation();
}

void loop() {
  tickConfig();
  tickWiFi();
  tickTCPServer();
  tickLoopAnimation();
}
