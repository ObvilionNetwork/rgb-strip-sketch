#include <EEPROM.h>

// Первый байт EEPROM для проверки валидности структуры
// По умолчанию в EEPROM памяти 0xFF является первой инициализацией, не использовать
#define CONFIG_VERSION 0x01

struct FadeConfig {
  byte period;
};

struct Config {
  // Имя сети для подключения к WiFi
  char ssid[32] = "ObvilionNetwork";
  // Пароль для подключения к WiFi
  char pass[32] = "password";

  // Токен доступа к Obvilion Home API
  char token[40] = "";

  // Дефолтный режим при включении (0 - запоминать последний включенный режим)
  byte default_mode = 0;

  // Последний рабочий режим
  byte last_mode = 5;

  // Количество цветов для анимации перехода для каждой ленты
  byte custom_animation_colors_count = 3;

  // Задержки для анимации перехода
  uint16_t custom_animation_delays[10] = {
    2000, 2000, 2000
  };

  // Цвета анимации перехода
  //  8 RGB цветов анимации: 
  //    3 цвета по отдельности: r, g, b
  color custom_animation_colors[10][3] = {
    { 255, 0, 0 },
    { 0, 255, 0 },
    { 0, 0, 255 }
  };

  // Время перехода на режим
  uint16_t mode_transition_time = 2000;

  // Яркость 255 - 100%, 0 - 0%
  byte brightness = 255;

  // Статичный цвет G R B
  color static_rgb[3] = { 0, 0, 255 };
  

  // Цвет эффекта fade
  color fade_rgb[3] = { 0, 255, 0 };

  // Период режима Fade
  uint16_t fade_period = 3000;


  // Цвет эффекта pulse
  color pulse_rgb[9] = { 255, 0, 0, /**/  0, 255, 0, /**/ 0, 0, 255 };
  
  // Период режима Pulse
  uint16_t pulse_period = 3000;
  
  // Минимальный уровень режима Pulse
  byte pulse_min_level = 32; // 32 из 255


  // Период режима радуга
  uint16_t rainbow_period = 5000;

  // Количество цветов в режиме
  byte rainbow_period_colors = 6;


  // Период режима плавной радуги
  uint16_t smooth_rainbow_period = 4000;


   // Период режима плавной радуги с затемнением
  uint16_t smooth_fade_rainbow_period = 5000;

  // Настройки анимации огня
  byte flare_colors_count = 5;

  uint16_t flare_delays[5] = {
    200, 700, 1000, 200, 1400
  };

  color flare_colors[5][3] = {
    { 2, 50, 1 },
    { 4, 101, 2 },
    { 20, 250, 0 },
    { 25, 254, 1 },
    { 40, 255, 2 }
  };

  // Настройки анимации биения сердца
  uint16_t breathing_delays[6] = {
    550, 300, 300, 20, 140, 140
  };

  color breathing_colors[6][3] = {
    { 0, 0, 0 },
    { 0, 255, 0 },
    { 0, 0, 0 },
    { 0, 0, 0 },
    { 0, 50, 0 },
    { 0, 0, 0 },
  };

  color breathing_color[3] = {
    2, 255, 22
  };
};

// Конфиг всего контроллера
Config config;

// Флаги
uint32_t eepromTimer;
boolean configChanged = false;

void loadConfig() {
  EEPROM.begin(sizeof(config) + 1); // +1 на версию конфига

  if (EEPROM.read(0) == CONFIG_VERSION) {
    EEPROM.get(1, config);
  } else {
    #ifdef DEBUG
      Serial.println(F("First initialization!"));
    #endif
  
    // Первая инициализация
    //EEPROM.write(0, CONFIG_VERSION);
    //EEPROM.put(1, config);

    configChanged = true;
  }
}

void tickConfig() {
  if (configChanged && millis() - eepromTimer > 60000) {
    configChanged = false;
    eepromTimer = millis();

    #ifdef DEBUG
      Serial.println(F("Config saved"));
    #endif
  }
}
