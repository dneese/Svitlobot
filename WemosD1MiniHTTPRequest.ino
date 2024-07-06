#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define EEPROM_SIZE 96

// Веб-сервер на порті 80
ESP8266WebServer server(80);

// NTP-клієнт для отримання часу
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // Оновлення часу кожні 60 секунд

String ssid = "";
String password = "";
String channelKey = "";

void setup() {
  Serial.begin(115200);

  // Ініціалізація EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Зчитування збережених налаштувань WiFi та канального ключа
  ssid = readEEPROMString(0, 32);
  password = readEEPROMString(32, 32);
  channelKey = readEEPROMString(64, 32);

  // Якщо SSID не збережений, відкрити веб-інтерфейс налаштувань
  if (ssid == "") {
    startConfigPortal();
  } else {
    connectWiFi();
    timeClient.begin();
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    return;
  }

  timeClient.update();
  
  // Виконувати кожні 4 хвилини
  if (timeClient.getMinutes() % 4 == 0 && timeClient.getSeconds() == 0) {
    makeHTTPRequest();
    delay(1000);  // Затримка для уникнення кількох запитів протягом тієї ж секунди
  }

  server.handleClient();
  delay(1000);  // Перевірка кожну секунду
}

void startConfigPortal() {
  WiFi.softAP("Wemos_Config");
  Serial.println("Відкрито точку доступу: Wemos_Config");

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", R"(
      <form action="/save" method="post">
        SSID: <input type="text" name="ssid"><br>
        Password: <input type="text" name="password"><br>
        Channel Key: <input type="text" name="channelKey"><br>
        <input type="submit" value="Save">
      </form>
    )");
  });

  server.on("/save", HTTP_POST, []() {
    ssid = server.arg("ssid");
    password = server.arg("password");
    channelKey = server.arg("channelKey");

    writeEEPROMString(0, 32, ssid);
    writeEEPROMString(32, 32, password);
    writeEEPROMString(64, 32, channelKey);
    EEPROM.commit();

    server.send(200, "text/html", "Saved! Restarting...");
    delay(2000);
    ESP.restart();
  });

  server.begin();
  Serial.println("Веб-сервер запущено, перейдіть на 192.168.4.1");
}

void connectWiFi() {
  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Підключення до WiFi...");
  }

  Serial.println("Підключено до WiFi");
}

void makeHTTPRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.svitlobot.in.ua/channelPing?channel_key=" + channelKey;
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.println("Помилка HTTP-запиту");
    }
    
    http.end();
  }
}

String readEEPROMString(int start, int len) {
  String data = "";
  for (int i = start; i < start + len; i++) {
    char ch = EEPROM.read(i);
    if (ch == 0) break;
    data += ch;
  }
  return data;
}

void writeEEPROMString(int start, int len, String data) {
  for (int i = start; i < start + len; i++) {
    if (i - start < data.length()) {
      EEPROM.write(i, data[i - start]);
    } else {
      EEPROM.write(i, 0);
    }
  }
}
