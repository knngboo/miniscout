//// --- minifirmware.cpp --- ////
//// //////////////////////// ////

// --- Includes --- //
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h> // Multiplexer Library
#include <Adafruit_VL53L0X.h> // Sensors Library

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
#define MTR_L_C1 33 // Speed/Position A *Input Only*
#define MTR_L_C2 32 // Speed/Position B *Input Only*

// Right Motor Driver (Channel A)
#define MTR_RPWM 5 // PWM for Speed Control
#define MTR_R_IN1 18 // Direction Control 1
#define MTR_R_IN2 19 // Direction Control 2
#define MTR_R_C1 35 // Speed/Position A *Input Only*
#define MTR_R_C2 34 // Speed/Position B *Input Only*

// GPIO PINS: Steering Servo
#define SERVO_PWM 13 // PWM for Steering Control

// GPIO PINS: Buzzer
#define BUZZER_PWM 27 // PWM for Buzzer Control

// --- WiFi Credentials --- //
// //////////////////////// //
const char* ssid = "vortex";;
const char* password = "xxxplaystation";

// --- The Digital World (HTML UI) --- //
// Stored in PROGMEM (Flash Memory)
// /////////////////////////////////// //
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <title>Map</title>
        <style>
            body {
                font-family: sans-serif; background: #1a1a1a; color: white; text-align: center;
            }
            canvas {
                background: #000; border: 2px solid #444; margin-top: 20px; cursor: crosshair;
            }
            .controls {
                margin-top: 20px;
            }
            #scout {
                background: #2ecc71; color: white;
            }
            #stop {
                background: #e74c3c; color: white;
            }
            #reset {
                background: #3498db; color: white;
            }
        </style>
    </head>
    <body>
        <h1>MiniScout Digital World</h1>
        <div id="status">Status: Disconnected</div>

        <canvas id="mapCanvas" width="600" height="400"></canvas>

        <div class="controls">
            <button id="scout">SCOUT</button>
            <button id="stop">STOP</button>
            <button id="reset">RESET MAP</button>
        </div>
        
        <script>
            const canvas = document.getElementById('mapCanvas');
            const ctx =canvas.getContext('2d');
            const status = document.getElementById('status');

            const gateway = `ws://${window.location.hostname}/ws`;
            let websocket;

            function initWebSocket() {
                websocket = new WebSocket(gateway);
                websocket.onopen = () => status.innerText = 'Status: Connected to Mini Scout';
                websocket.onclose = () => status.innerText = 'Status: Disconnected';
                websocket.onmessage = (event) => {
                    console.log("Data from Mini Scout:", event.data);
                };
            }

            document.getElementById('scout').onclick = () => websocket.send("SCOUT");
            document.getElementById('stop').onclick = () => websocket.send("STOP");
            document.getElementById('reset').onclick = () => {
                websocket.send("RESET");
                ctx.clearRect(0, 0, canvas.width, canvas.height);
            };

            window.onload = initWebSocket;
        </script>
    </body>
</html>)rawliteral";

// --- WebSocket Event Handler --- //
// /////////////////////////////// //
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    // --- Filter: Listen for Data only --- //
    if (type == WS_EVT_DATA) {
        String message = ""; // Create Empty Container
        // --- Reconstruct Message --- //
        // Converts raw bits into readable text characters
        for (size_t i = 0; i < len; i++) {
            message += (char)data[i]; // {Data Reassembly}
        }
        Serial.print("Command Received: "); // {Debug: Received}
        Serial.println(message); // {Debug: Command}

        // --- Command Switchboard --- //
        if (message == "SCOUT") {
            Serial.println("Scouting Started..."); // [Trigger Drive Logic]
        } else if (message == "STOP") {
            Serial.println("Scouting Stopped..."); // [Kill Motors]
        } else if (message == "RESET") {
            Serial.println("Map Reset..."); // [Refresh Web UI]
        }
    }
}

//// --- Void Setup --- ////
//// ////////////////// ////
void setup() {

    // --- Declare Pin Modes --- //
    // Multiplexer Control
    pinMode(MUX_RST, OUTPUT); // Set Reset Pin

    // Right Motor Control
    pinMode(MTR_RPWM, OUTPUT); // Set RPWM
    pinMode(MTR_R_IN1, OUTPUT); // Set Right Direction 1
    pinMode(MTR_R_IN2, OUTPUT); // Set Right Direction 2

    // Left Motor Control
    pinMode(MTR_LPWM, OUTPUT); // Set LPWM
    pinMode(MTR_L_IN1, OUTPUT); // Set Left Direction 1
    pinMode(MTR_L_IN2, OUTPUT); // Set Left Direction 2

    // Right Motor Encoders
    pinMode(MTR_R_C1, INPUT); //Set Right Encoder A
    pinMode(MTR_R_C2, INPUT); //Set Right Encoder B 
    
    // Left Motor Encoders
    pinMode(MTR_L_C1, INPUT); // Set Left Encoder A
    pinMode(MTR_L_C2, INPUT); //Set Left Encoder B          

    // Servo
    pinMode(SERVO_PWM, OUTPUT); //Set Servo Pin

    //Buzzer
    pinMode(BUZZER_PWM, OUTPUT); //Set Buzzer Pin

    // --- Perform a Hard Reset on I2C --- //
    digitalWrite(MUX_RST, LOW); // Off
    delay(10);
    digitalWrite(MUX_RST, HIGH); // On
    delay(10);

    // --- Serial Monitor Debugging --- //
    Serial.begin(115200); // Start Data Link [115200]

    // --- Network Hub --- //
    WiFi.softAP(ssid, password); // Boot "vortex" WiFi
    Serial.print("AP Started: "); // {debug}
    Serial.println(ssid); // {debug}
    Serial.print("IP Address: "); // {debug}
    Serial.println(WiFi.softAPIP()); // {debug}

    // --- System Communication --- //
    ws.onEvent(onEvent); // Bind Socket Logic
    server.addHandler(&ws); // Enable WebSocket Support

    // --- Web Routes Lambda Function --- //
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", index_html); // Root Response [Send PROGMEM HTML]
    });
    server.begin(); // Launch Server
}

void loop() {
    ws.cleanupClients(); // Keep Socket Stable
}