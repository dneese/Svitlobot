#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Введіть свої дані мережі
const char* ssid = "ім'я мережі вайфай";
const char* password = "пароль вайфай";



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
    Serial.println("Виконується HTTP-запит...");

    HTTPClient http;
    String url = "http://api.svitlobot.in.ua/channelPing?channel_key=КЛЮЧ"; // Введіть свій канальний ключ
    http.begin(url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Отримана відповідь:");
      Serial.println(payload);
    } else {
      Serial.print("Помилка HTTP-запиту: ");
      Serial.println(http.errorToString(httpCode).c_str());
    }
    
    http.end();
  } else {
    Serial.println("Немає з'єднання з WiFi");
  }
}
}
