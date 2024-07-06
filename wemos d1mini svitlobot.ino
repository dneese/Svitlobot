#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Введіть свої дані мережі
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Введіть свій канальний ключ
const char* channelKey = "ВАШКЛЮЧ";

// NTP-клієнт для отримання часу
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // Оновлення часу кожні 60 секунд

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Підключення до WiFi...");
  }

  Serial.println("Підключено до WiFi");
  
  timeClient.begin();
}

void loop() {
  timeClient.update();
  
  // Виконувати кожні 4 хвилини
  if (timeClient.getMinutes() % 4 == 0 && timeClient.getSeconds() == 0) {
    makeHTTPRequest();
    delay(1000);  // Затримка для уникнення кількох запитів протягом тієї ж секунди
  }
  
  delay(1000);  // Перевірка кожну секунду
}

void makeHTTPRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.svitlobot.in.ua/channelPing?channel_key=" + String(channelKey);
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
