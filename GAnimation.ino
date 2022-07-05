uint32_t animationTimer;

enum AnimationType {
  NONE,
  
  // Основные режимы ленты
  OFF,                // +То же что и NONE
  CUSTOM,             // +Кастомная последовательность (до 10 цветов)
  STATIC_RGB,         // +Один цвет
  FADING_STATIC_RGB,  // +Один цвет с эффектом затухания
  PULSE,              // +Пульсирующий один цвет
  RAINBOW,            // +Радуга с резким изменением цвета
  SMOOTH_RAINBOW,     // +Переливающаяся радуга
  FADE_RAINBOW,       // -Перечисление цветов с эффектом затухания
  FADING_RAINBOW,     // +Переливающаяся радуга с эффектом затухания
  FLARE,              // Эффект огня, цвет можн настраивать
  BREATHING,          // Эффект дыхания
  VIRTUALIZER,        // Мигание в такт музыки
  SHAKE,              // Реагирует на тряску телефона
  //LEFT_RIGHT,         // Переход цвета с ле вого колеса на правое и обратно (один цвет)
  //LEFT,               // Переход цвета налево
  //RIGHT,              // Переход цвета направо
  //LEFT_RIGHT_RAINBOW, // Переход цвета с левого колеса на правое и обратно (радуга)
  
  // Системные анимации
  FIRST_INITIALIZATION = 255, // Переход синего в фиолетовый и обратно с периодом 4 секунды
  CONNECTING_TO_WIFI   = 254, // Первое подключение к WiFi, синий цвет плавно мигает с периодом 4 секунды 
  CONNECTED_TO_WIFI    = 253, // Успешное подключение к WiFi, плавно меняется цвет в зеленый и затухает
  WIFI_IS_OFFLINE      = 252, // Два раза моргнет фиолетовым и последний плавно затухнет
  WIFI_WRONG_PASSWORD  = 251, // Переход в красный цвет и мигание с периодом 2 секунды 3 раза

  // Входящие локальные пакеты (настройки цвета может передать клиент)
  CANCEL_ANIMATION = 230, // Отменяет последнюю ВТОРИЧНУЮ анимацию
  CUSTOM_ANIMATION = 229, // Читает анимацию из данных пакета: тип, задержка, циклы, r, g, b
  NOTIFICATION     = 228, // Плавно моргает два раза белым цветом с периодом 2 секунды
  INCOMING_CALL    = 227, // Плавно моргает зеленым с периодом 5 секунд
  ANSWERED_CALL    = 226, // Моргает желтым и затухает
  ENDED_CALL       = 225, // Моргает красным и затухает
  MISSED_CALL      = 224, // Моргает красным два раза 1 с и затухает
  OUTGOING_CALL    = 223, // Моргает зеленым два раза 1 с и затухает на 3 секунды, повторяется
  VOLUME_PLUS      = 222, // Плавно моргает зеленым 1 раз за 4 секунды
  VOLUME_MINUS     = 221  // Плавно моргает желтым 1 раз за 4 секунды
};

byte          SYSTEM_ANIMATIONS_QSIZE;
AnimationType SYSTEM_ANIMATIONS_QUEUE[5]; // Очередь из анимаций
AnimationType SYSTEM_ANIMATION;           // Поверхносная анимация

AnimationType ANIMATION_MODE;       // Текущий режим
AnimationType NEXT_MODE;            // Следущий режим
uint32_t      NEXT_MODE_STARTED_AT; // Время старта перехода

color colors[3];            // Текущий цвет на выход RGB лент
color transition_colors[3]; // При переходе последний цвет копируется сюда
boolean transition_started; // Метод инициализации режима прошел?

byte cur_mode_flags[32]; // 32 байта флагов
uint32_t sys_animation_flag;
uint32_t mode_time_flag; // Это должен быть один из флагов массива выше, но мне лень его реализовывать
uint32_t mode_time_flags[3];

uint32_t last_animation_tick; // Время последнего рендера анимации
boolean animation_is_rendereing = false; //

void loadAnimation() {
  if (CUR_WIFI_MODE == SSID_NOT_SPECIFIED) {
    SYSTEM_ANIMATION = FIRST_INITIALIZATION;
  }
}

void tickTimerAnimation() {
  uint32_t cur = millis();
  if (cur < last_animation_tick + 20 || animation_is_rendereing) {
    return;
  }

  #ifdef DEBUG
    Serial.print("Timer ");
    Serial.println(last_animation_tick);
  #endif
  
  tickAnimation();
}

void tickLoopAnimation() {
  if (last_animation_tick + 19 <= millis()) {
    animation_is_rendereing = true;
    tickAnimation();
    animation_is_rendereing = false;
  }
}

void tickAnimation() {
  last_animation_tick = millis();
  
  render();

  // Можно конечно повесить ифки для проверки изменения состояния, чтобы не
  // перезаписывать лишний раз, но думаю это не очень необходимо.
  analogWrite(R_LED, colors[0] * config.brightness / 255);
  analogWrite(G_LED, colors[1] * config.brightness / 255);
  analogWrite(B_LED, colors[2] * config.brightness / 255);
}

void transition(byte* result, byte* from, byte* to, uint16_t delta_time, uint16_t duration) {
  for (byte i = 0; i < 3; i++) {
    result[i] = from[i] + (to[i] - from[i]) * delta_time / duration;
  }
}

AnimationType add_to_sys_anim;
void addSysAnimationToQueue() {
  SYSTEM_ANIMATIONS_QUEUE[SYSTEM_ANIMATIONS_QSIZE] = add_to_sys_anim;
  SYSTEM_ANIMATIONS_QSIZE += 1;
  
  #ifdef DEBUG
    Serial.println(F("Added animation to queue"));
  #endif
}

void render() {
  if (!SYSTEM_ANIMATION && SYSTEM_ANIMATIONS_QSIZE > 0) {
    SYSTEM_ANIMATION = SYSTEM_ANIMATIONS_QUEUE[0];
    sys_animation_flag = 0;
    
    AnimationType newQueue[5];
    newQueue[0] = SYSTEM_ANIMATIONS_QUEUE[1];
    newQueue[1] = SYSTEM_ANIMATIONS_QUEUE[2];
    newQueue[2] = SYSTEM_ANIMATIONS_QUEUE[3];
    newQueue[3] = SYSTEM_ANIMATIONS_QUEUE[4];

    memcpy(SYSTEM_ANIMATIONS_QUEUE, newQueue, sizeof(newQueue));
    
    SYSTEM_ANIMATIONS_QSIZE -= 1;

    #ifdef DEBUG
      Serial.println(F("Played animation from queue"));
    #endif
  }
  
  if (SYSTEM_ANIMATION) {
    switch (SYSTEM_ANIMATION) {
      case FIRST_INITIALIZATION: renderFirstInit();
        break;
      case CONNECTING_TO_WIFI: renderWifiConnecting();
        break;
      case CONNECTED_TO_WIFI: renderWifiConnected();
        break;
      case WIFI_WRONG_PASSWORD: renderWifiWrongPass();
        break;
      //case WIFI_IS_OFFLINE: renderWifiOffline();
      //  break;
    }
    
    return;
  }
  
  if (NEXT_MODE) {
    ANIMATION_MODE = NEXT_MODE;
  }

  switch (ANIMATION_MODE) {
    case CUSTOM: renderCustom();
      break;
    case STATIC_RGB: memcpy(colors, config.static_rgb, 3);
      break;
    case FADING_STATIC_RGB: renderFading();
      break;
    case PULSE: renderPulse();
      break;
    case RAINBOW: renderRainbow();
      break;
    case SMOOTH_RAINBOW: renderSmoothRainbow();
      break;
    case FADE_RAINBOW: renderFadeRainbow();
      break;
    case FADING_RAINBOW: renderSmoothFadeRainbow();
      break;
    case NONE: 
      colors[0] = 0;
      colors[1] = 0;
      colors[2] = 0;
      break;
    case OFF: 
      colors[0] = 0;
      colors[1] = 0;
      colors[2] = 0;
      break;
  }
  
  if (NEXT_MODE) {
    // Если переход только что запустили, то копируем последние цвета
    if (!transition_started) {
      memcpy(transition_colors, colors, 3);
      NEXT_MODE_STARTED_AT = millis();
      transition_started = true;
    }

    uint16_t i = millis() - NEXT_MODE_STARTED_AT;

    // Если время перехода истекло
    if (i >= config.mode_transition_time) {
      ANIMATION_MODE = NEXT_MODE;
      NEXT_MODE = NONE;
      i = config.mode_transition_time;
    }

    color result[3];
    transition(result, transition_colors, colors, i, config.mode_transition_time);

    // Получаем цвета для рендера перехода
    memcpy(colors, result, 3);
  }
}
