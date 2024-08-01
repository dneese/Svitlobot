#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>

// Клас для керування підключенням до WiFi
class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password) {
        _ssid = ssid;
        _password = password;
    }

    void connect() {
        WiFi.begin(_ssid, _password);
        while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Підключення до WiFi...");
        }
        Serial.println("Підключено до WiFi");
        Serial.println("IP адреса: ");
        Serial.println(WiFi.localIP());
    }

    bool isConnected() {
        return WiFi.status() == WL_CONNECTED;
    }

private:
    const char* _ssid;
    const char* _password;
};

// Клас для виконання HTTP-запитів
class HTTPClientWrapper {
public:
    HTTPClientWrapper(const String& url) {
        _url = url;
    }

    int get() {
        HTTPClient http;
        http.begin(_url);
        int httpCode = http.GET();
        if (httpCode > 0) {
            String payload = http.getString();
            Serial.println(payload);
        }
        http.end();
        return httpCode;
    }

private:
    String _url;
};

// Введіть назву мережі WiFi, пароль і ключ тут
const char* ssid = "your_ssid";
const char* password = "your_password";
const String channelKey = "your_channel_key";

WiFiManager wifiManager(ssid, password);
HTTPClientWrapper httpClient("https://api.svitlobot.in.ua/channelPing?channel_key=" + channelKey);

Ticker wifiCheckTimer;
Ticker httpRequestTimer;

void setup() {
    Serial.begin(9600);
    wifiManager.connect();
    wifiCheckTimer.attach(10, checkWiFi); // Встановлює таймер для перевірки WiFi кожні 10 секунд
    httpRequestTimer.attach(240, makeHTTPRequest); // Встановлює таймер для виконання HTTP-запитів кожні 240 секунд
}

void loop() {
    // Не робить нічого
}

// Функція для відображення повідомлень
void displayMessage(const String& message) {
    Serial.println(message);
}

// Функція для перевірки стану підключення до WiFi та перепідключення, якщо потрібно
void checkWiFi() {
    if (!wifiManager.isConnected()) {
        displayMessage("Втрачено з'єднання з WiFi, перепідключення...");
        wifiManager.connect();
    }
}

// Функція для виконання HTTP GET-запиту до вказаного URL, якщо ESP8266 підключений до WiFi
void makeHTTPRequest() {
    if (wifiManager.isConnected()) {
        int httpCode = httpClient.get();
        if (httpCode > 0) {
            displayMessage("HTTP-запит успішний, код: " + String(httpCode));
        } else {
            displayMessage("Помилка HTTP-запиту, код: " + String(httpCode));
        }
    } else {
        displayMessage("Не підключено до WiFi");
    }
}
