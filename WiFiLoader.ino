uint32_t wifi_ticker;

void loadWiFi() {
  if (strcmp(config.ssid, "") == 0) {
    runAP();
    CUR_WIFI_MODE = SSID_NOT_SPECIFIED;
  } else {
    //SYSTEM_ANIMATION = CONNECTING_TO_WIFI;
    runAC();
  }
}

void tickWiFi() {
  // Пропускать только если неверный пароль или нет сети
  if (CUR_WIFI_MODE < 4) {
    return;
  }

  // Ждем 10 секунд и пытаемся подключиться снова
  if (millis() - wifi_ticker >= 20000) {
    wifi_ticker = millis();

    runAC();
  }
}

void runAP() {
  WiFi.softAPConfig(AP_IP, 
        IPAddress(192, 168, 4, 1), 
        IPAddress(255, 255, 255, 0));
  
  WiFi.softAP(AP_SSID, AP_PASS);
}

void runAC() {
  WiFi.persistent(false);
  WiFi.hostname(AC_HOSTNAME);
  //WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.pass);

  CUR_WIFI_MODE = CONNECTING;
  
  add_to_sys_anim = CONNECTING_TO_WIFI;
  addSysAnimationToQueue();

  // Создаём таймер подключения к сети WiFi
  Timer tmr(15000);

  #ifdef DEBUG
    Serial.println(F("Connecting to WiFi..."));
  #endif

  digitalWrite(I_LED, 0);

  // Ожидаем подключения к WiFi
  while (WiFi.status() != WL_CONNECTED) {
    // Вставить логику обновления сюда
    tickConfig();
    tickTCPServer();

    // Если неверный пароль, то запускаем точку доступа
    if (WiFi.status() == WL_WRONG_PASSWORD) {
      CUR_WIFI_MODE = WRONG_PASSWORD;

      add_to_sys_anim = WIFI_WRONG_PASSWORD;
      addSysAnimationToQueue();
      
      #ifdef DEBUG
        Serial.println(F("Wrong WiFi password"));
      #endif

      WiFi.disconnect();
      
      runAP();
 
      return;
    }

    // Если время подключения вышло, то запускаем точку доступа
    if (tmr.period()) {
      WiFi.disconnect();
      runAP();

      CUR_WIFI_MODE = AC_IS_OFFLINE;
      
      //add_to_sys_anim = WIFI_IS_OFFLINE;
      //addSysAnimationToQueue();
      
      #ifdef DEBUG
        Serial.println(F("WiFi is offline"));
      #endif
      
      return;
    }
  }

  digitalWrite(I_LED, 1);

  
  // Подключено к WiFi
  CUR_WIFI_MODE = CONNECTED;
  
  add_to_sys_anim = CONNECTED_TO_WIFI;
  addSysAnimationToQueue();

  WiFi.hostname(AC_HOSTNAME);

  #ifdef DEBUG
     Serial.print(F("Connected to wifi. IP: "));
     Serial.print(WiFi.localIP());
     Serial.print(F(" MAC: "));
     Serial.println(WiFi.macAddress());
  #endif
        
  // Вырубаем точку доступа
  WiFi.mode(WIFI_STA);

  NEXT_MODE = FADE_RAINBOW;
}
