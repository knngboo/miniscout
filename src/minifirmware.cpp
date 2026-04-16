//// --- minifirmware.cpp --- ////
//// //////////////////////// ////
// --- Includes --- //
// //////////////// //
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- ESP32 Pin Names --- //
// /////////////////////// //
// GPIO PINS: I2C & Multiplexer
#define MUX_RST 4 //RST -> Pin 4 [Multiplexer Reset]

// I2C COMMUNICATION: Control hand over to internal I2C Hardware Controller
#define I2C_SDA 21 //SDA -> Pin 21 [Serial Data]
#define I2C_SCL 22 //SCL -> Pin 22 [Serial Clock]

// CHANNELS: Multiplexer channels
#define MUX_CH_LEFT 0 // (sc0 / sd0)
#define MUX_CH_CENTER 1 // (sc1 / sd1)
#define MUX_CH_RIGHT 2 // (sc2 / sd2)

// GPIO PINS: left motor driver channel b
#define MTR_LPWM 15 //pwmb -> pin 15 [pwm for speed control]
#define MTR_L_IN1 16 //bin1 -> pin rx2 [direction control 1]
#define MTR_L_IN2 17 //BIN2 -> Pin TX2 [Direction Control 2]
#define MTR_L_ENC_A 33 //C1 -> Pin 33 [Speed/Position A] *Input Only*
#define MTR_L_ENC_B 32 //C2 -> Pin 32 [Speed/Position B] *Input Only*

// GPIO PINS: Right Motor Driver Channel A
#define MTR_RPWM 5 //PWMA -> Pin 5 [PWM for Speed Control]
#define MTR_R_IN1 18 //AIN1 -> Pin 18 [Direction Control 1]
#define MTR_R_IN2 19 //AIN2 -> Pin 19 [Direction Control 2]
#define MTR_R_ENC_A 35 //C1 -> Pin 35 [Speed/Position A] *Input Only*
#define MTR_R_ENC_B 34 //C2 -> Pin 34 [Speed/Position B] *Input Only*

// GPIO PINS: Steering Servo
#define SERVO_STEER 13 //PWM -> Pin 13 [PWM for Steering Control]

// GPIO PINS: Buzzer
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
    pinMode(MTR_L_ENC_A, INPUT); //Set Left Encoder A as an input to listen for rotation pulses
    pinMode(MTR_L_ENC_B, INPUT); //Set Left Encoder B as an input to listen for rotation pulses
    pinMode(MTR_R_ENC_A, INPUT); //Set Right Encoder A as an input to listen for rotation pulses
    pinMode(MTR_R_ENC_B, INPUT); //Set Right Encoder B as an input to listen for rotation pulses

    // Servo
    pinMode(SERVO_STEER, OUTPUT); //Set Servo Pin as an output to send angle position

    //Buzzer
    pinMode(BUZZER_PIN, OUTPUT); //Set Buzzer Pin as an output to trigger audio

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