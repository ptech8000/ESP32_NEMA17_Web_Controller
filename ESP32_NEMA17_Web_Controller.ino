#include <WiFi.h>
#include <WebServer.h>

// =============================
// WiFi AP
// =============================
const char* AP_SSID = "P-TECH_MOTOR";
const char* AP_PASSWORD = "12345678";

// =============================
// A4988 pins
// =============================
#define STEP_PIN 25
#define DIR_PIN  26
#define EN_PIN   27

// =============================
// Web server
// =============================
WebServer server(80);

// =============================
// Motor variables
// =============================
long currentPosition = 0;

bool motorRunning = false;
bool directionForward = true;

long remainingSteps = 0;

unsigned long lastStepMicros = 0;

int stepSpeed = 500; // steps per second

// =============================
// HTML PAGE
// =============================
const char MAIN_PAGE[] PROGMEM = R"rawliteral(

<!DOCTYPE html>
<html>

<head>

<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>P-TECH Motor Control</title>

<style>

body {
    font-family: Arial;
    background: #111;
    color: white;
    text-align: center;
    margin: 0;
    padding: 20px;
}

.container {
    max-width: 500px;
    margin: auto;
    background: #222;
    padding: 25px;
    border-radius: 15px;
}

h1 {
    color: #00ff99;
}

input {
    width: 90%;
    padding: 15px;
    margin: 10px;
    font-size: 20px;
    border-radius: 8px;
    border: none;
}

button {
    padding: 15px 20px;
    margin: 8px;
    font-size: 17px;
    border: none;
    border-radius: 8px;
    cursor: pointer;
}

.forward {
    background: #00cc66;
    color: white;
}

.reverse {
    background: #ff9900;
    color: white;
}

.stop {
    background: #ff3333;
    color: white;
}

.reset {
    background: #555;
    color: white;
}

.status {
    margin-top: 20px;
    padding: 15px;
    background: #333;
    border-radius: 10px;
}

</style>

</head>

<body>

<div class="container">

<h1>P-TECH</h1>

<h2>NEMA 17 Motor Control</h2>

<label>Steps</label>

<input
    type="number"
    id="steps"
    value="200"
    min="1"
>

<label>Speed (steps/sec)</label>

<input
    type="number"
    id="speed"
    value="500"
    min="1"
    max="5000"
>

<br>

<button
    class="reverse"
    onclick="moveMotor('reverse')">
     REVERSE
</button>

<button
    class="forward"
    onclick="moveMotor('forward')">
    FORWARD 
</button>

<br>

<button
    class="stop"
    onclick="stopMotor()">
    STOP
</button>

<br>

<button
    class="reset"
    onclick="resetPosition()">
    RESET POSITION
</button>

<div class="status">

<h3>Status</h3>

<p>
Position:
<span id="position">0</span>
steps
</p>

<p>
Motor:
<span id="motorStatus">Stopped</span>
</p>

</div>

</div>

<script>

function moveMotor(direction) {

    let steps =
        document.getElementById("steps").value;

    let speed =
        document.getElementById("speed").value;

    fetch(
        "/move?steps=" +
        steps +
        "&direction=" +
        direction +
        "&speed=" +
        speed
    );
}


function stopMotor() {

    fetch("/stop");

}


function resetPosition() {

    fetch("/reset");

}


function updateStatus() {

    fetch("/status")
    .then(response => response.json())
    .then(data => {

        document.getElementById("position").innerText =
            data.position;

        document.getElementById("motorStatus").innerText =
            data.running ? "Running" : "Stopped";

    });

}


setInterval(updateStatus, 300);

</script>

</body>

</html>

)rawliteral";

// =============================
// MOVE MOTOR
// =============================
void handleMove() {

    if (!server.hasArg("steps")) {
        server.send(400, "text/plain", "Missing steps");
        return;
    }

    long steps = server.arg("steps").toInt();

    if (steps <= 0) {
        server.send(400, "text/plain", "Invalid steps");
        return;
    }

    if (server.hasArg("direction")) {

        String dir = server.arg("direction");

        if (dir == "forward") {
            directionForward = true;
            digitalWrite(DIR_PIN, HIGH);
        }
        else {
            directionForward = false;
            digitalWrite(DIR_PIN, LOW);
        }
    }

    if (server.hasArg("speed")) {

        stepSpeed =
            server.arg("speed").toInt();

        if (stepSpeed < 1)
            stepSpeed = 1;

        if (stepSpeed > 5000)
            stepSpeed = 5000;
    }

    remainingSteps = steps;

    motorRunning = true;

    digitalWrite(EN_PIN, LOW);

    server.send(
        200,
        "text/plain",
        "Motor started"
    );
}

// =============================
// STOP
// =============================
void handleStop() {

    motorRunning = false;

    remainingSteps = 0;

    digitalWrite(EN_PIN, HIGH);

    server.send(
        200,
        "text/plain",
        "Motor stopped"
    );
}

// =============================
// RESET POSITION
// =============================
void handleReset() {

    currentPosition = 0;

    server.send(
        200,
        "text/plain",
        "Position reset"
    );
}

// =============================
// STATUS
// =============================
void handleStatus() {

    String json = "{";

    json += "\"position\":";
    json += currentPosition;

    json += ",";

    json += "\"running\":";
    json += motorRunning ? "true" : "false";

    json += "}";

    server.send(
        200,
        "application/json",
        json
    );
}

// =============================
// MOTOR CONTROL
// =============================
void runMotor() {

    if (!motorRunning)
        return;

    if (remainingSteps <= 0) {

        motorRunning = false;

        digitalWrite(EN_PIN, HIGH);

        return;
    }

    unsigned long interval =
        1000000UL / stepSpeed;

    unsigned long now =
        micros();

    if (now - lastStepMicros >= interval) {

        lastStepMicros = now;

        // STEP HIGH
        digitalWrite(STEP_PIN, HIGH);

        delayMicroseconds(3);

        // STEP LOW
        digitalWrite(STEP_PIN, LOW);

        remainingSteps--;

        if (directionForward)
            currentPosition++;
        else
            currentPosition--;
    }
}

// =============================
// SETUP
// =============================
void setup() {

    Serial.begin(115200);

    pinMode(STEP_PIN, OUTPUT);
    pinMode(DIR_PIN, OUTPUT);
    pinMode(EN_PIN, OUTPUT);

    digitalWrite(STEP_PIN, LOW);
    digitalWrite(DIR_PIN, LOW);

    // Disable driver initially
    digitalWrite(EN_PIN, HIGH);

    // Start WiFi AP
    WiFi.softAP(
        AP_SSID,
        AP_PASSWORD
    );

    Serial.println();
    Serial.println("================================");
    Serial.println("P-TECH NEMA 17 CONTROLLER");
    Serial.println("================================");

    Serial.print("WiFi SSID: ");
    Serial.println(AP_SSID);

    Serial.print("Password: ");
    Serial.println(AP_PASSWORD);

    Serial.print("Open: http://");
    Serial.println(WiFi.softAPIP());

    // Routes
    server.on(
        "/",
        HTTP_GET,
        []() {

            server.send_P(
                200,
                "text/html",
                MAIN_PAGE
            );

        }
    );

    server.on(
        "/move",
        HTTP_GET,
        handleMove
    );

    server.on(
        "/stop",
        HTTP_GET,
        handleStop
    );

    server.on(
        "/reset",
        HTTP_GET,
        handleReset
    );

    server.on(
        "/status",
        HTTP_GET,
        handleStatus
    );

    server.begin();

    Serial.println(
        "Web server started."
    );
}

// =============================
// LOOP
// =============================
void loop() {

    server.handleClient();

    runMotor();
}
