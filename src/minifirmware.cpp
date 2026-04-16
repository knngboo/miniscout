//// --- minifirmware.cpp --- ////
//// //////////////////////// ////

// --- Includes --- //
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h> //Multiplexer Library
#include <Adafruit_VL53L0X.h> //Sensors Library

// --- Global Object -- //
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// --- ESP32 Pin Names --- //
// I2C & Multiplexer
#define MUX_RST 4 // Multiplexer Reset
#define I2C_SDA 21 // Serial Data
#define I2C_SCL 22 // Serial Clock

// Multiplexer Channels
#define MUX_CH_LEFT 0 // (sc0 / sd0)
#define MUX_CH_CENTER 1 // (sc1 / sd1)
#define MUX_CH_RIGHT 2 // (sc2 / sd2)

// Left Motor Driver (Channel B)
#define MTR_LPWM 15 // PWM for Speed Control
#define MTR_L_IN1 16 // Direction Control 1
#define MTR_L_IN2 17 // Direction Control 2
#define MTR_L_C1 33 //C1 -> Pin 33 [Speed/Position A] *Input Only*
#define MTR_L_C2 32 //C2 -> Pin 32 [Speed/Position B] *Input Only*

// Right Motor Driver (Channel A)
#define MTR_RPWM 5 // PWM for Speed Control
#define MTR_R_IN1 18 // Direction Control 1
#define MTR_R_IN2 19 // Direction Control 2
#define MTR_R_C1 35 //C1 -> Pin 35 [Speed/Position A] *Input Only*
#define MTR_R_C2 34 //C2 -> Pin 34 [Speed/Position B] *Input Only*

// GPIO PINS: Steering Servo
#define SERVO_PWM 13 //PWM -> Pin 13 [PWM for Steering Control]

// GPIO PINS: Buzzer
#define BUZZER_PWM 27 //PWM -> Pin 27 [PWM for Buzzer Control]

// --- WiFi Credentials --- //
// //////////////////////// //
const char* ssid = "vortex";;
const char* password = "xxxplaystation";

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

//// --- Void Setup --- ////
//// ////////////////// ////
void setup() {

    // --- Declare Pin Modes --- //
    // Multiplexer Control
    pinMode(MUX_RST, OUTPUT); //Set Reset Pin as an output

    // Right Motor Control
    pinMode(MTR_RPWM, OUTPUT); //Set RPWM as an output to control speed
    pinMode(MTR_R_IN1, OUTPUT); //Set Right Direction 1 as an output
    pinMode(MTR_R_IN2, OUTPUT); //Set Right Direction 2 as an output

    // Left Motor Control
    pinMode(MTR_LPWM, OUTPUT); //Set LPWM as an output to control speed
    pinMode(MTR_L_IN1, OUTPUT); //Set Left Direction 1 as an output
    pinMode(MTR_L_IN2, OUTPUT); //Set Left Direction 2 as an output

    // Motor Encoders
    pinMode(MTR_L_C1, INPUT); //Set Left Encoder A as an input to listen for rotation pulses
    pinMode(MTR_L_C2, INPUT); //Set Left Encoder B as an input to listen for rotation pulses
    pinMode(MTR_R_C1, INPUT); //Set Right Encoder A as an input to listen for rotation pulses
    pinMode(MTR_R_C2, INPUT); //Set Right Encoder B as an input to listen for rotation pulses

    // Servo
    pinMode(SERVO_PWM, OUTPUT); //Set Servo Pin as an output to send angle position

    //Buzzer
    pinMode(BUZZER_PWM, OUTPUT); //Set Buzzer Pin as an output to trigger audio

    // --- Perform a Hrad Reset on I2C --- //
    digitalWrite(MUX_RST, LOW);
    delay(10);
    digitalWrite(MUX_RST, HIGH);
    delay(10);

    Serial.begin(115200); //Starts serial communication to computer for debugging

    // --- Start the WiFi Access Point --- //
    WiFi.softAP(ssid, password);
    Serial.println("AP Started: " + String(ssid)); //{serial debugging}
    Serial.print("IP Address: "); //{serial debugging}
    Serial.println(WiFi.softAPIP()); //{serial debugging}


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