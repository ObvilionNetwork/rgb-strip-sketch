WiFiServer TCP_SERVER(TCP_SERVER_PORT);
WiFiClient TCP_SERVER_CLIENTS[TCP_SERVER_MAX_CLIENTS];

void loadTCPServer() {
  TCP_SERVER.begin();

  #ifdef DEBUG
    Serial.println(F("TCP Server started!"));
  #endif
}

void tickTCPServer() {
  WiFiClient client = TCP_SERVER.available();
  
  if (client) {
    boolean added = false;

    for (byte i = 0; i < TCP_SERVER_MAX_CLIENTS; i++) {
      if (!TCP_SERVER_CLIENTS[i]) {
        #ifdef DEBUG
          Serial.print(F("Client "));
          Serial.print(i + 1);
          Serial.println(F(" added!"));
        #endif
    
        TCP_SERVER_CLIENTS[i] = client;
        added = true;
        break;
      }
    }

    #ifdef DEBUG
      if (!added) {
        Serial.println(F("Client limit extended!"));
      }
    #endif
  }

  for (byte i = 0; i < TCP_SERVER_MAX_CLIENTS; i++) {
    if (!TCP_SERVER_CLIENTS[i]) {
      continue;
    }

    readPacket(TCP_SERVER_CLIENTS[i], i);
  }
}
