// --- minifirmware.cpp --- //

// --- Includes --- //
// //////////////// //
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- ESP32 Pin Names --- //
// /////////////////////// //

// I2C & Multiplexer
#define I2C_SDA 21 //SDA -> Pin 21 [Serial Data]
#define I2C_SCL 22 //SCL -> Pin 22 [Serial Clock]
#define MUX_RST 4 //RST -> Pin 4 [Multiplexer Reset]

// ToF Sensor Channels
#define MUX_CH_LEFT 0 // (SC0 / SD0)
#define MUX_CH_CENTER 1 // (SC1 / SD1)
#define MUX_CH_RIGHT 2 // (SC2 / SD2)

// Left Motor Driver Channel B
#define MTR_LPWM 15 //PWMB -> Pin 15 [PWM for Speed Control]
#define MTR_L_IN1 16 //BIN1 -> Pin RX2 [Direction Control 1]
#define MTR_L_IN2 17 //BIN2 -> Pin TX2 [Direction Control 2]
#define MTR_L_ENC_A 33 //C1 -> Pin 33 [Speed/Position A] *Input Only*
#define MTR_L_ENC_B 32 //C2 -> Pin 32 [Speed/Position B] *Input Only*

// Right Motor Driver Channel A
#define MTR_RPWM 5 //PWMA -> Pin 5 [PWM for Speed Control]
#define MTR_R_IN1 18 //AIN1 -> Pin 18 [Direction Control 1]
#define MTR_R_IN2 19 //AIN2 -> Pin 19 [Direction Control 2]
#define MTR_R_ENC_A 35 //C1 -> Pin 35 [Speed/Position A] *Input Only*
#define MTR_R_ENC_B 34 //C2 -> Pin 34 [Speed/Position B] *Input Only*

// Steering Servo
#define SERVO_STEER 13 //PWM -> Pin 13 [PWM for Steering Control]

// Buzzer
#define BUZZER_PIN 27 //PWM -> Pin 27 [PWM for Buzzer Control]

// --- WiFi Credentials --- //
// //////////////////////// //
const char* ssid = "vortex";;
const char* password = "xxxplaystation";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
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

void setup() {
    Serial.begin(115200);

    WiFi.softAP(ssid, password);
    Serial.println("AP Started: " + String(ssid));
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    ws.onEvent(onEvent);
    server.addHandler(&ws);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "MiniScout Server is Live. Open the HTML file on your PC.");
    });

    server.begin();
}

void loop() {
    ws.cleanupClients();
}