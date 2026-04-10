#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "vortex";;
const char* password = "xxxplaystation";

AsyncWebServer server(80);
AsyncWebServer ws("/ws");

void onEvent(AsyncWebSocket *server, AsynceWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        String message = "";
        for (size_t i = 0; i < len; i++) {
            message += (char)data[i];
        }

        Serial.print("Command Received: ");
        Serial.println(message);

        if (message == "SCOUT") {
            Serial.println("Scouting Started...");
        } else if (message == "STOP") {
            Serial.println("Scouting Stopped...");
        } else if (message == "RESET") {
            Serial.println("Map Reset...");
        }
    }
}

void setup() {}