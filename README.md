
# Wemos D1 Mini HTTP Request

Цей проект використовує Wemos D1 Mini для періодичного виконання HTTP-запитів кожні 4 хвилини.

## Вимоги

- Arduino IDE
- Wemos D1 Mini
- WiFi мережа
- Канальний ключ для сервісу [SvitloBot](https://api.svitlobot.in.ua)

## Налаштування

1. Завантажте та встановіть [Arduino IDE](https://www.arduino.cc/en/software).
2. Додайте менеджер плат ESP8266:
   - Перейдіть у **Файл > Параметри** (File > Preferences) і додайте наступну URL-адресу в поле "Додаткові URL-адреси менеджера плат" (Additional Board Manager URLs): 
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Встановіть пакет плат ESP8266, перейшовши в **Інструменти > Плата > Менеджер плат** (Tools > Board > Boards Manager), знайдіть "ESP8266" і встановіть його.
3. Встановіть необхідні бібліотеки:
   - Перейдіть у **Sketch > Include Library > Manage Libraries** і встановіть наступні бібліотеки:
     - `ESP8266HTTPClient`
     - `NTPClient`
     - `WiFi`

## Використання
Відкрийте файл WemosD1MiniHTTPRequest.ino в Arduino IDE.

Введіть свої дані WiFi і канальний ключ у відповідні поля:

const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
const char* channelKey = "ВАШКЛЮЧ";

Завантажте скетч на ваш Wemos D1 Mini.Підключіть серійний монітор, щоб побачити результати.


1. Клонувати цей репозиторій:
   ```sh
   git clone https://github.com/yourusername/Wemos-D1-Mini-HTTP-Request.git
