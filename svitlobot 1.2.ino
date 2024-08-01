#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Ticker.h>
#include <ESP8266Ping.h>
#include <EEPROM.h>

const int EEPROM_SIZE = 512;
const int MAX_RETRIES = 3;
const int RETRY_DELAY = 5000; // 5 секунд

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

    void setSSID(const char* ssid) {
        _ssid = ssid;
    }

    void setPassword(const char* password) {
        _password = password;
    }

private:
    const char* _ssid;
    const char* _password;
};

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

    void setChannelKey(const String& channelKey) {
        _url = "https://api.svitlobot.in.ua/channelPing?channel_key=" + channelKey;
    }

private:
    String _url;
};

WiFiManager wifiManager("your_ssid", "your_password");
HTTPClientWrapper httpClient("https://api.svitlobot.in.ua/channelPing?channel_key=your_channel_key");

Ticker wifiCheckTimer;
Ticker httpRequestTimer;

void setup() {
    Serial.begin(9600);
    EEPROM.begin(EEPROM_SIZE);
    loadSettings();
    printWelcomeMessage();
    wifiManager.connect();
    wifiCheckTimer.attach(10, checkWiFi); // Встановлює таймер для перевірки WiFi кожні 10 секунд
    httpRequestTimer.attach(240, makeHTTPRequest); // Встановлює таймер для виконання HTTP-запитів кожні 240 секунд
}

void loop() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');
        handleCommand(command);
    }
}

void handleCommand(String command) {
    command.trim();
    if (command.startsWith("set ssid ")) {
        String ssid = command.substring(9);
        setSSID(ssid);
    } else if (command.startsWith("set password ")) {
        String password = command.substring(13);
        setPassword(password);
    } else if (command.startsWith("set channelkey ")) {
        String channelKey = command.substring(15);
        setChannelKey(channelKey);
    } else if (command == "show settings") {
        showSettings();
    } else {
        Serial.println("Невідома команда");
    }
}

void setSSID(String ssid) {
    ssid.toCharArray(ssid, 32);
    EEPROM.put(0, ssid);
    EEPROM.commit();
    wifiManager.setSSID(ssid.c_str());
    Serial.println("SSID збережено: " + ssid);
}

void setPassword(String password) {
    password.toCharArray(password, 64);
    EEPROM.put(32, password);
    EEPROM.commit();
    wifiManager.setPassword(password.c_str());
    Serial.println("Пароль збережено");
}

void setChannelKey(String channelKey) {
    channelKey.toCharArray(channelKey, 64);
    EEPROM.put(96, channelKey);
    EEPROM.commit();
    httpClient.setChannelKey(channelKey);
    Serial.println("Канальний ключ збережено: " + channelKey);
}

void showSettings() {
    char ssid[32];
    char password[64];
    char channelKey[64];
    EEPROM.get(0, ssid);
    EEPROM.get(32, password);
    EEPROM.get(96, channelKey);
    Serial.println("SSID: " + String(ssid));
    Serial.println("Пароль: " + String(password));
    Serial.println("Канальний ключ: " + String(channelKey));
}

void loadSettings() {
    char ssid[32];
    char password[64];
    char channelKey[64];
    EEPROM.get(0, ssid);
    EEPROM.get(32, password);
    EEPROM.get(96, channelKey);
    wifiManager.setSSID(ssid);
    wifiManager.setPassword(password);
    httpClient.setChannelKey(channelKey);
    Serial.println("Налаштування завантажено:");
    Serial.println("SSID: " + String(ssid));
    Serial.println("Пароль: " + String(password));
    Serial.println("Канальний ключ: " + String(channelKey));
}

void printWelcomeMessage() {
    Serial.println("Ласкаво просимо до конфігуратора ESP8266!");
    Serial.println("Використовуйте наступні команди для налаштування:");
    Serial.println("set ssid your_ssid");
    Serial.println("set password your_password");
    Serial.println("set channelkey your_channel_key");
    Serial.println("show settings");
}

void checkWiFi() {
    if (!wifiManager.isConnected()) {
        Serial.println("Втрачено з'єднання з WiFi, перепідключення...");
        wifiManager.connect();
        if (!wifiManager.isConnected()) {
            handleWiFiError();
        }
    } else if (!checkInternetConnection()) {
        Serial.println("Інтернет недоступний, перепідключення...");
        WiFi.disconnect();
        wifiManager.connect();
    }
}

bool checkInternetConnection() {
    const IPAddress remoteIp(8, 8, 8, 8); // Google DNS сервер
    return Ping.ping(remoteIp);
}

void makeHTTPRequest() {
    int retries = 0;
    while (retries < MAX_RETRIES) {
        int httpCode = httpClient.get();
        if (httpCode > 0) {
            Serial.println("HTTP-запит успішний, код: " + String(httpCode));
            break;
        } else {
            Serial.println("Помилка HTTP-запиту, код: " + String(httpCode));
            if (httpCode == 404) {
                Serial.println("Ресурс не знайдено");
            } else if (httpCode == 500) {
                Serial.println("Помилка сервера");
            }
            retries++;
            if (retries < MAX_RETRIES) {
                delay(RETRY_DELAY);
            }
        }
    }
    if (retries == MAX_RETRIES) {
        Serial.println("Не вдалося виконати HTTP-запит після " + String(MAX_RETRIES) + " спроб");
    }
}

void handleWiFiError() {
    Serial.println("Помилка підключення до WiFi. Перевірте налаштування мережі.");
    ESP.restart();
}
