#include <WiFi.h>
#include <map>
#include <functional>
#include <ArduinoJson.h>

static int TCP_PORT = 3333;

using Handler = std::function<void(StaticJsonDocument<512>&)>;
std::map<String, Handler> handlers;

void scanWifiNetworks() {
  Serial.println("Iniciando scan de redes WiFi...");
  int n = WiFi.scanNetworks();

  if (n == 0) {
    Serial.println("Nenhuma rede encontrada.");
  } else {
    Serial.print(n);
    Serial.println(" redes encontradas:");
    for (int i = 0; i < n; ++i) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
}

static void createWifiAP(const char* ssid, const char* password) {
  if (!WiFi.softAP(ssid, password)) {
    Serial.println("Soft AP creation failed.");
    while (true);
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("Server started");
}

void connectToAWifi(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
    if (millis() - start > 20000) { // timeout 20s
      Serial.println("\nTimeout tentando conectar. Verifique a senha ou se o seu wifi se encontra abaixo:");
      scanWifiNetworks();
      break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Falha na conexão.");
    Serial.printf("Wifi Status: %d; Bloqueando a execução...", WiFi.status());
    while(true);
  }
}

static void TASK_tcpServer(void *parameter) {
  WiFiServer server(TCP_PORT);
  server.begin();
  Serial.printf("[TCP] Servidor iniciado na porta %d\n", TCP_PORT);

  for (;;) {
    WiFiClient client = server.available();
    client.setTimeout(10000);

    if (client) {
      Serial.println("[TCP] Cliente conectado");

      String data = "";
      unsigned long startTime = millis();
      bool iddleTimeout = false;
      while (client.connected() && !data.endsWith("\n")) {
        while (client.available()) {
          char c = client.read();
          data += c;
        }
        if(millis() - startTime > 10000) {
          iddleTimeout = true;
          break;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
      }

      if(iddleTimeout) {
        client.println("{\"status\":\"IDDLE TIMEOUT\"}");
      } else if (data.length() > 0) {
        Serial.println("[TCP] Dados recebidos:");
        Serial.println(data);

        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, data);
        if (err) {
          Serial.print("[ERRO] JSON inválido: ");
          Serial.println(err.c_str());
          client.println("{\"status\":\"ERROR\",\"reason\":\"INVALID JSON\"}");
        } else {
          String tipo = doc["type"].as<String>();

          auto requestHandler = handlers.find(tipo);
          if(requestHandler != handlers.end()) {
            Serial.println("Handler encontrado, executando...");
            
            try {
              (requestHandler->second)(doc);
              client.println("{\"status\":\"OK\"}");
            } catch (const std::exception &e) {
              Serial.print("[ERRO] Exceção geral: ");
              Serial.println(e.what());
              client.println("{\"status\":\"INTERNAL_ERROR\"}");
            } catch (...) {
              Serial.println("[ERRO] Exceção desconhecida geral");
              client.println("{\"status\":\"INTERNAL_ERROR\"}");
            }
          } else {
            Serial.print("Handler não encontrado para: ");
            Serial.println(tipo);
            client.println("{\"status\":\"NOT FOUND\"}");
          }
        }
      } else {
        client.println("{\"status\":\"NO DATA RECEIVED\"}");
      }

      client.stop();
      Serial.println("[TCP] Cliente desconectado");
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

static int preciseDistance = 10;
void andar(int cm) {
  Serial.printf("Carrinho andou para frente em cm: ");
  Serial.println(cm);
}

void girar(int direction) {
  Serial.printf("Carrinho girou na direção:");
  Serial.println(direction);
}

void handleInstructions(StaticJsonDocument<512> payload) {
  JsonArray instructions = payload["data"].as<JsonArray>();
  for(auto instruction : instructions) {
    String action = instruction["action"].as<String>();
    int value = instruction["value"].as<int>();  

    if(action.equals("move")) {
      while(value > 0) {
        andar(min(preciseDistance, value));
        value -= preciseDistance;
      }
    } else {
      girar(value);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // createWifiAP("ESP32_AP_TULIO", "12345678");
  connectToAWifi("Letícia ", "21052000");

  handlers["instructions"] = handleInstructions;
  xTaskCreate(TASK_tcpServer,"TCP Server",4096,NULL,1,NULL);
}


void loop() {
  
}
