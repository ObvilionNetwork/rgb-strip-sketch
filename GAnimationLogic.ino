color rainbow_colors[3][3] = {
  { 255, 0, 0 },
  { 0, 255, 0 },
  { 0, 0, 255 }
};

void renderFromToColors(int32_t delta, color color_list[2][3], uint16_t period, byte colors[3]) {
  colors[0] = (byte) (color_list[0][0] + (color_list[1][0] - color_list[0][0]) * delta / period);
  colors[1] = (byte) (color_list[0][1] + (color_list[1][1] - color_list[0][1]) * delta / period);
  colors[2] = (byte) (color_list[0][2] + (color_list[1][2] - color_list[0][2]) * delta / period);
}

void renderColors(uint32_t delta, color color_list[][3], byte count, uint16_t period, byte colors[3]) {
  byte from_color = delta / (period / count);
  byte to_color = from_color + 1;

  if (to_color >= count) {
    to_color = 0;
  }

  uint16_t part = period / count;
  int32_t delta_rp = delta - part * from_color;

  colors[0] = (byte) (color_list[from_color][0] + (color_list[to_color][0] - color_list[from_color][0]) * delta_rp / part);
  colors[1] = (byte) (color_list[from_color][1] + (color_list[to_color][1] - color_list[from_color][1]) * delta_rp / part);
  colors[2] = (byte) (color_list[from_color][2] + (color_list[to_color][2] - color_list[from_color][2]) * delta_rp / part);
}

byte renderFade(uint32_t delta, uint16_t period) {
  uint16_t fp2 = period / 2;

  if (delta <= fp2) {
    return       255 * delta         / fp2;
  } else {
    return 255 - 255 * (delta - fp2) / fp2;
  }
}


void renderCustom() {
  // Находим длительность всей анимации
  uint32_t sum = 0;
  
  for (byte i = 0; i < config.custom_animation_colors_count; i++) {
    sum += config.custom_animation_delays[i];
  }

  // Находим промежуток времени с начала анимации
  uint32_t delta = millis() - mode_time_flag;
  
  if (delta >= sum) {
    mode_time_flag = millis();
    delta = 0;
  }

  // Находим цвет, на котором сейчас сидим
  uint32_t tmp = 0;
  uint16_t duration = 0;
  uint8_t index, pred_index = 0;

  for (uint8_t i = 0; i < config.custom_animation_colors_count; i++) {
    if (tmp + config.custom_animation_delays[i] <= delta) {
      tmp += config.custom_animation_delays[i];
    } else {
      index = i;
      duration = config.custom_animation_delays[i];
      break;
    }
  }

  int32_t delta_i = delta - tmp;

  pred_index = index - 1;
  color color_list[2][3];
  
  if (pred_index == 255) {
    pred_index = config.custom_animation_colors_count - 1;
  }

  memcpy(color_list[0], config.custom_animation_colors[pred_index], 3);
  memcpy(color_list[1], config.custom_animation_colors[index], 3);
  
  renderFromToColors(delta_i, color_list, duration, colors);
}

void renderSmoothRainbow() {
  uint16_t delta = millis() - mode_time_flag;
  
  if (delta >= config.smooth_rainbow_period) {
    mode_time_flag = millis();
    delta = 0;
  }

  renderColors(delta, rainbow_colors, 3, config.smooth_rainbow_period, colors);
}

void renderSmoothFadeRainbow() {
  uint16_t delta = millis() - mode_time_flag;
  
  if (delta >= config.smooth_fade_rainbow_period) {
    mode_time_flag = millis();
    delta = 0;
  }

  byte buf[3];
  renderColors(delta, rainbow_colors, 3, config.smooth_fade_rainbow_period, buf);

  // +500 для начала с еденицы, высчитываем остаток от периода смены цвета
  delta = (millis() - mode_time_flag + 500) % (config.smooth_fade_rainbow_period / 3);
  int16_t br = renderFade(delta, config.smooth_fade_rainbow_period / 3);
  
  colors[0] = buf[0] * br / 255;
  colors[1] = buf[1] * br / 255;
  colors[2] = buf[2] * br / 255;
}

void renderRainbow() {
  uint32_t delta = millis() - mode_time_flag;
  
  if (delta >= config.rainbow_period) {
    mode_time_flag = millis();
    delta = 0;
  }

  uint8_t period = delta / (config.rainbow_period / config.rainbow_period_colors);

  uint8_t per_mode = config.rainbow_period_colors / 3;
  uint8_t cur_m = period % per_mode;

  uint8_t from = period / per_mode;
  uint8_t to = from + 1;

  if (to > 2) {
    to = 0;
  }

  colors[0] = rainbow_colors[from][0] + 
      (rainbow_colors[to][0] - rainbow_colors[from][0]) * cur_m / per_mode;
      
  colors[1] = rainbow_colors[from][1] + 
      (rainbow_colors[to][1] - rainbow_colors[from][1]) * cur_m / per_mode;
      
  colors[2] = rainbow_colors[from][2] + 
      (rainbow_colors[to][2] - rainbow_colors[from][2]) * cur_m / per_mode;
}

void renderFading() {
  uint32_t delta = millis() - mode_time_flag;
  
  if (delta > config.fade_period) {
    mode_time_flag = millis();
    delta = 0;
  }

  byte brightness = renderFade(delta, config.fade_period);

  for (byte i = 0; i < 3; i++) {
    colors[i] = config.fade_rgb[i] * brightness / 255;
  }
}

void renderPulse() {
  uint16_t delta = millis() - mode_time_flag;
  
  if (delta > config.fade_period) {
    mode_time_flag = millis();
    delta = 0;
  }

  byte brightness;
  uint16_t fp2 = config.fade_period / 2;

  if (delta <= fp2) {
    brightness = config.pulse_min_level + (255 - config.pulse_min_level) * delta / fp2;
  } else {
    brightness = 255 - (255 - config.pulse_min_level) * (delta - fp2) / fp2;
  }

  for (byte i = 0; i < 3; i++) {
    colors[i] = config.pulse_rgb[i] * brightness / 255;
  }
}

#define WIFI_CONNECTING_ANIM_TIME  1733

void renderWifiConnecting() {
  if (sys_animation_flag == 0) {
    sys_animation_flag = millis();
  }

  uint16_t fp2 = WIFI_CONNECTING_ANIM_TIME / 2;
  uint32_t delta = millis() - sys_animation_flag;
  byte brightness;
  byte minimum = 15;

  if (delta >= WIFI_CONNECTING_ANIM_TIME) {
    if (CUR_WIFI_MODE == CONNECTING) {
      sys_animation_flag = millis();
    } else {
      sys_animation_flag = 0;
      SYSTEM_ANIMATION = NONE;
    }

    delta = 0;
  }

  if (delta <= fp2) {
    brightness = minimum + (255 - minimum) * delta / fp2;
  } else {
    brightness = 255 - (255 - minimum) * (delta - fp2) / fp2;
  }

  colors[0] = 22 * brightness / 255;
  colors[1] = 184 * brightness / 255;
  colors[2] = 255 * brightness / 255;
}

void renderFirstInit() {
  if (sys_animation_flag == 0) {
    sys_animation_flag = millis();
  }

  uint32_t delta = millis() - sys_animation_flag;

  if (delta >= 3000) {
    sys_animation_flag = millis();
    delta = 0;
  }

  // G R B
  color color_list[][3] = {
    {30, 255, 243}, 
    {48, 38, 255},
  };
  
  renderColors(delta, color_list, 2, 3000, colors);
}

#define WIFI_WRONG_PASS_ANIM_TIME 1700
void renderWifiWrongPass() {
  if (sys_animation_flag == 0) {
    sys_animation_flag = millis();
  }

  uint32_t delta = millis() - sys_animation_flag;

  if (delta >= WIFI_WRONG_PASS_ANIM_TIME) {
    sys_animation_flag = 0;
    SYSTEM_ANIMATION = NONE;
    delta = 0;
  }

  uint16_t _time = WIFI_WRONG_PASS_ANIM_TIME / 2;

  delta = delta % _time;

  uint16_t fp2 = _time / 2;
  byte brightness;
  byte minimum = 15;

  if (delta <= fp2) {
    brightness = minimum + (255 - minimum) * delta / fp2;
  } else {
    brightness = 255 - (255 - minimum) * (delta - fp2) / fp2;
  }

  colors[0] = 5 * brightness / 255;
  colors[1] = 255 * brightness / 255;
  colors[2] = 10 * brightness / 255;
}

#define WIFI_CONNECTED_ANIM_TIME 1000
void renderWifiConnected() {
  if (sys_animation_flag == 0) {
    sys_animation_flag = millis();
  }

  uint32_t delta = millis() - sys_animation_flag;

  if (delta >= WIFI_CONNECTED_ANIM_TIME) {
    sys_animation_flag = 0;
    SYSTEM_ANIMATION = NONE;
    delta = 0;
  }

  uint16_t fp2 = WIFI_CONNECTED_ANIM_TIME / 2;
  byte brightness;
  byte minimum = 15;

  if (delta <= fp2) {
    brightness = minimum + (255 - minimum) * delta / fp2;
  } else {
    brightness = 255 - (255 - minimum) * (delta - fp2) / fp2;
  }

  colors[0] = 230 * brightness / 255;
  colors[1] = 45 * brightness / 255;
  colors[2] = 75 * brightness / 255;
}

// UNUSED
void renderWifiOffline() {
  if (sys_animation_flag == 0) {
    sys_animation_flag = millis();
  }

  uint32_t delta = millis() - sys_animation_flag;

  if (delta >= WIFI_WRONG_PASS_ANIM_TIME) {
    sys_animation_flag = 0;
    SYSTEM_ANIMATION = NONE;
    delta = 0;
  }

  uint16_t _time = WIFI_WRONG_PASS_ANIM_TIME / 2;

  delta = delta % _time;

  uint16_t fp2 = _time / 2;
  byte brightness;
  byte minimum = 15;

  if (delta <= fp2) {
    brightness = minimum + (255 - minimum) * delta / fp2;
  } else {
    brightness = 255 - (255 - minimum) * (delta - fp2) / fp2;
  }

  colors[0] = 45 * brightness / 255;
  colors[1] = 255 * brightness / 255;
  colors[2] = 5 * brightness / 255;
}
