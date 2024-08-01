#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

// WiFi налаштування
const char* ssid = "ВАШ_ССІД";
const char* password = "ВАШ_ПАРОЛЬ";

// Інтервал між запитами в секундах (4 хвилини)
const unsigned long interval = 240; // в секундах
const String channelKey = "ВАШКЛЮЧ";
const String serverURL = "https://api.svitlobot.in.ua/channelPing?channel_key=" + channelKey;
const String internetCheckURL = "http://www.google.com"; // URL для перевірки доступності інтернету

Ticker wifiCheckTimer;
Ticker httpRequestTimer;

void setup() {
  Serial.begin(9600);                  // Ініціалізація серійного зв'язку
  WiFi.begin(ssid, password);          // Підключення до WiFi

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Підключення до WiFi...");
  }

  Serial.println("Підключено до WiFi");
  Serial.println("IP адреса: ");
  Serial.println(WiFi.localIP());

  wifiCheckTimer.attach(10, checkWiFi); // Перевірка статусу WiFi кожні 10 секунд
  httpRequestTimer.attach(interval, makeHTTPRequest); // Встановлення таймера для HTTP-запитів
}

void loop() {
  // Порожня функція loop
}

void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Спроба перепідключення до WiFi...");
    WiFi.begin(ssid, password);
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Підключено до WiFi");
      Serial.println("IP адреса: ");
      Serial.println(WiFi.localIP());
    }
  }
}

void makeHTTPRequest() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!checkInternetConnection()) {
      Serial.println("Інтернет недоступний, спроба перепідключення...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      return;
    }

    HTTPClient http;

    Serial.print("Відправка HTTP-запиту на: ");
    Serial.println(serverURL);

    http.begin(serverURL);           // Встановлення URL для запиту
    int httpCode = http.GET();       // Виконання GET-запиту

    if (httpCode > 0) { // Перевірка статусу відповіді
      String payload = http.getString();
      Serial.println(httpCode);      // Виведення статусного коду
      Serial.println(payload);       // Виведення відповіді
    } else {
      Serial.print("Помилка HTTP-запиту: ");
      Serial.println(http.errorToString(httpCode).c_str()); // Виведення тексту помилки
    }

    http.end(); // Завершення запиту
  } else {
    Serial.println("Не підключено до WiFi");
  }
}

bool checkInternetConnection() {
  HTTPClient http;
  http.begin(internetCheckURL);
  int httpCode = http.GET();
  http.end();
  return httpCode == HTTP_CODE_OK;
}
