// É necessário baixar as libs: 
// ArduinoJson         by Benoit Blanchon
// Async TCP           by ESP32Async
// ESP Async WebServer by ESP32Async

#include <WiFi.h>
#include <functional>

// #include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

void andar(int cm) {
  Serial.printf("Carrinho andou para frente em cm: ");
  Serial.println(cm);
}

void girar(int direction) {
  Serial.printf("Carrinho girou na direção:");
  Serial.println(direction);
}

static int preciseDistance = 10;
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

AsyncWebServer server(80);
using Handler = std::function<void(StaticJsonDocument<512>&)>;
using WebHandler = std::function<void(AsyncWebServerRequest*, uint8_t*, size_t, size_t, size_t)>;
WebHandler mapHandlerToWebHandler(Handler handler) {
  return [handler](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    Serial.print("[HTTP] Rota encontrada: ");
    Serial.println(request->url());

    StaticJsonDocument<512> payload;
    DeserializationError err = deserializeJson(payload, data, len);
    if (err) {
      Serial.print("[ERRO] JSON inválido: ");
      Serial.println(err.c_str());
      request->send(422, "application/json", "{\"status\":422,\"message\":\"INVALID JSON\"}");
      return;
    }

    try {
      handler(payload);
      request->send(200, "application/json", "{\"status\":200,\"message\":\"OK\"}");
    } catch (const std::exception &e) {
      Serial.print("[ERRO] Exceção geral: ");
      Serial.println(e.what());
      request->send(500, "application/json", "{\"status\":500,\"message\":\"INTERNAL_SERVER_ERROR\"}");
    } catch (...) {
      Serial.println("[ERRO] Exceção desconhecida geral");
      request->send(500, "application/json", "{\"status\":500,\"message\":\"INTERNAL_SERVER_ERROR\"}");
    }
  };
}

void setupWebServer() {
  Serial.println("Configurando servidor web...");

  Serial.println("Configurando CORS...");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

  Serial.println("Definindo as rotas e os preflights");
  server.on(
    "/instructions",
    HTTP_POST,
    [](AsyncWebServerRequest *request){},
    NULL,
    mapHandlerToWebHandler(handleInstructions)
  );
  server.on("/instructions", HTTP_OPTIONS, [](AsyncWebServerRequest *request) {
    request->send(200);
  });

  Serial.println("Definindo not found...");
  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.printf("Rota não encontrada: %s\n", request->url().c_str());
    request->send(404, "application/json", "{\"status\":\"NOT FOUND\"}");
  });

  server.begin();
  Serial.println("Listening...");
}

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

void setup() {
  Serial.begin(115200);
  delay(1000);

  // createWifiAP("ESP32_AP_TULIO", "12345678");
  connectToAWifi("Letícia ", "21052000");
  setupWebServer();
}


void loop() {
  
}
