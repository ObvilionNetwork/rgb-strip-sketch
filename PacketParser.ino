enum Packet {
  // Входящие пакеты
  GET_TOKEN,            // Запрос на получение токена
  GET_VERSION,          // Запрос на получение версии
  GET_WIFI_INFO,        // Запрос на получение информации о WiFi
  GET_MODE,             // Запрос на получение текущего режима
  SET_MODE,             // Устанавливает режим с параметрами
  SET_TOKEN,            // Устанавливает новый токен устройству
  SET_VISUALIZER_COLOR, // Устанавливает цвет визуализатора
  SET_WIFI_DATA,        // Устанавливает новые данные WiFi
  PLAY_ANIMATION,       // Проиграывает анимацию

  // Исходящие пакеты
  TOKEN   = 128, // Отдает токен
  VERSION = 129, // Отдает версию
  WARNING = 130, // Отправляет ошибку на клиент
  MODE    = 131  // Отправляет информацию о режиме
};

byte PACKET_INDEX[TCP_SERVER_MAX_CLIENTS];

void readPacket(WiFiClient client, byte i) {
  while (client.available() > 0) {
    byte r = client.read();
    if (r == TCP_SERVER_FIRST_BYTES[PACKET_INDEX[i]]) {
      PACKET_INDEX[i]++;

      if (PACKET_INDEX[i] == 3) {
        parcePacket(client);
        return;
      }
    } else {
      if (r == TCP_SERVER_FIRST_BYTES[0]) {
        PACKET_INDEX[i] = 1;
      } else {
        PACKET_INDEX[i] = 0;
      }
    }
  }
}


// Структура пакета:
// 222 111 222 - первые три байта
// xxx         - тип пакета
void parcePacket(WiFiClient client) {  
  Packet ptype = (Packet) client.read();

  #ifdef DEBUG
    Serial.print(F("Packet parce: "));
    Serial.println(ptype);
  #endif
  
  switch (ptype) {
    case SET_VISUALIZER_COLOR:
      if (ANIMATION_MODE != VIRTUALIZER) {
        return;
      }

      // TODO: set color

      break;
    case SET_MODE:
      NEXT_MODE = (AnimationType) client.read();
      
      // TODO: animation props
      
      break;
    case GET_MODE:
      client.write(Packet::MODE);
      client.write(ANIMATION_MODE);

      // TODO
      
      break;
    case PLAY_ANIMATION:
      SYSTEM_ANIMATION = (AnimationType) client.read();

      // TODO
      
      break;
    case GET_WIFI_INFO:
      {
        client.write(CUR_WIFI_MODE);
        Long2Bytes l2b; 

        // Название сети WiFi
        client.write(config.ssid, 32);
        
        // Уровень сигнала
        l2b.value = WiFi.RSSI();
        client.write(l2b.bytes, 4);

        // Локальный IP адресс
        l2b.value = (int32_t) WiFi.localIP();
        client.write(l2b.bytes, 4);

        // Мак адрес устройства
        byte mac_address[6];
        WiFi.macAddress(mac_address); 
        client.write(mac_address, 6);

        // Мак адрес роутера
        client.write(WiFi.BSSID(), 6);
      }
      
      break;
    case GET_VERSION:
      {
        char device[] = OBV_HOME_DEVICE;
        int bSize = sizeof(device) / sizeof(char);
        
        client.write(bSize);
        client.write(device, bSize);

        Int2Bytes version;
        version.value = OBV_HOME_DEVICE_VERSION;

        client.write(version.bytes, 2);
      }
      
      break;
    case SET_WIFI_DATA:
      if (client.available() < 64) {
        return;
      }

      for (byte i = 0; i < 32; i++) {
        config.ssid[i] = client.read();
      }

      for (byte i = 0; i < 32; i++) {
        config.pass[i] = client.read();
      }

      break;
    case GET_TOKEN:
      {
        client.write(Packet::TOKEN);
  
        byte data[40]; 
        memcpy(data, config.token, 40);
        
        byte out[80];
  
        for (byte i = 0; i < 2; i++) {
          int32_t keys[2];
          
          Long2Bytes convert;
          
          for (byte c = 0; c < 4; c++) {
            convert.bytes[c] = client.read();
          }
          keys[0] = convert.value;
  
          for (byte c = 0; c < 4; c++) {
            convert.bytes[c] = client.read();
          }
          keys[1] = convert.value;
          
          //encrypt(data, 40 * (i * 4 + 1), out, keys);
  
          if (i == 0) {
            memcpy(data, out, 40 * 4);
          }
        }
  
        client.write(data, 40 * 16);
      }
      
      break;
    case SET_TOKEN:
      if (client.available() < 40) {
        return;
      } 

      for (byte i = 0; i < 40; i++) {
        config.token[i] = client.read();
      }

      break;
  }
}
