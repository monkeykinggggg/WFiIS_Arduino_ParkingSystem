#include "pitches.h"
#include <Servo.h>
#define TRIG_LEFT 13
#define ECHO_LEFT 11

#define TRIG_CENTER 6
#define ECHO_CENTER 10

#define TRIG_RIGHT 5
#define ECHO_RIGHT 9

#define LIMIT 2
#define BUZZER 7

#define SERVO_PIN 3

int mario_melody[] = {
    NOTE_E5, NOTE_E5, REST, NOTE_E5, REST, NOTE_C5, NOTE_E5,
    NOTE_G5, REST, NOTE_G4, REST,
    NOTE_C5, NOTE_G4, REST, NOTE_E4,
    NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
    NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5
};

int mario_durations[] = {
    8, 8, 8, 8, 8, 8, 8,
    4, 4, 8, 4,
    4, 8, 4, 4,
    4, 4, 8, 4,
    8, 8, 8, 4, 8, 8
};

int gameover_melody[] = {
    NOTE_C5, NOTE_G4, NOTE_E4,
    NOTE_A4, NOTE_B4, NOTE_A4, NOTE_GS4, NOTE_AS4, NOTE_GS4,
    NOTE_G4, NOTE_D4, NOTE_E4
};

int gameover_durations[] = {
    4, 4, 4,
    8, 8, 8, 8, 8, 8,
    8, 8, 2
};


int pacman_melody[] = {
    NOTE_B4, NOTE_B5, NOTE_FS5, NOTE_DS5,
    NOTE_B5, NOTE_FS5, NOTE_DS5, NOTE_C5,
    NOTE_C6, NOTE_G6, NOTE_E6, NOTE_C6, NOTE_G6, NOTE_E6,

    NOTE_B4, NOTE_B5, NOTE_FS5, NOTE_DS5, NOTE_B5,
    NOTE_FS5, NOTE_DS5, NOTE_DS5, NOTE_E5, NOTE_F5,
    NOTE_F5, NOTE_FS5, NOTE_G5, NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_B5
};

int pacman_durations[] = {
    16, 16, 16, 16,
    32, 16, 8, 16,
    16, 16, 16, 32, 16, 8,

    16, 16, 16, 16, 32,
    16, 8, 32, 32, 32,
    32, 32, 32, 32, 32, 16, 8
};


int squidgame_melody[] = {
    NOTE_F4, NOTE_F4, NOTE_F4, NOTE_D4, NOTE_DS4, NOTE_F4, REST,
    NOTE_F4, NOTE_F4, NOTE_F4, NOTE_D4, NOTE_DS4, NOTE_F4, REST,
    NOTE_G4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_AS4, NOTE_AS4, NOTE_A4, NOTE_G4,
    NOTE_F4, NOTE_F4, NOTE_F4, NOTE_G4, NOTE_F4, REST,

    NOTE_F4, NOTE_F4, NOTE_F4, NOTE_D4, NOTE_DS4, NOTE_F4, REST,
    NOTE_F4, NOTE_F4, NOTE_F4, NOTE_D4, NOTE_DS4, NOTE_F4, REST,
    NOTE_G4, NOTE_G4, NOTE_G4, NOTE_C5, NOTE_AS4, NOTE_A4, NOTE_G4, NOTE_A4, NOTE_AS4, NOTE_AS4, NOTE_AS4,

    REST
};

int squidgame_durations[] = {
    6, 6, 6, 4, 6, 2, 3,
    6, 6, 6, 4, 6, 2, 3,
    4, 6, 4, 6, 4, 6, 4, 6,
    4, 6, 4, 6, 2, 3,

    6, 6, 6, 4, 6, 2, 3,
    6, 6, 6, 4, 6, 2, 3,
    4, 6, 4, 6, 4, 6, 4, 6, 2, 2, 2,

    1
};

int tokyo_drift_melody[] = {
    NOTE_AS4, REST, NOTE_AS4, REST, NOTE_AS4, REST, NOTE_AS4, REST,
    NOTE_AS4, NOTE_B4, NOTE_DS5,
    NOTE_AS4, REST, NOTE_AS4, REST,
    NOTE_AS4, NOTE_B4, NOTE_DS5,
    NOTE_AS4, REST, NOTE_AS4, REST,
    NOTE_AS4, NOTE_B4, NOTE_DS5,
    NOTE_AS4, REST, NOTE_AS4, REST,
    NOTE_AS4, NOTE_B4, NOTE_DS5,
    NOTE_F5, REST, NOTE_F5, REST,
    NOTE_GS5, NOTE_FS5, NOTE_F5,
    NOTE_AS4, REST, NOTE_AS4, REST,
    NOTE_GS5, NOTE_FS5, NOTE_F5,
    NOTE_AS4, REST, NOTE_AS4, REST,
    NOTE_AS4, NOTE_B4, NOTE_DS5,
    NOTE_AS4, REST, NOTE_AS4, REST,
    NOTE_AS4, NOTE_B4, NOTE_DS5,
    NOTE_AS4, REST, NOTE_AS4, REST,
    REST
};

int tokyo_drift_durations[] = {
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 4,
    4, 4, 4, 4,
    3, 3, 4,
    4, 4, 4, 4,
    3, 3, 4,
    4, 4, 4, 4,
    3, 3, 4,
    4, 4, 4, 4,
    3, 3, 4,
    4, 4, 4, 4,
    3, 3, 4,
    4, 4, 4, 4,
    3, 3, 4,
    4, 4, 4, 4,
    3, 3, 4,
    4, 4, 4, 4,
    1
};

int* melody = mario_melody;
int* durations = mario_durations;
int melodyLength = sizeof(mario_durations) / sizeof(mario_durations[0]);

 bool streaming = true;

Servo servoMotor;
int servoAngle = 90;
bool controlEnabled = false;
int setpoint_cm = 30;
float Kp = 1.0;
int deadband_cm = 2;
int max_step_deg = 8;

long readUltrasonic(int trigPin, int echoPin) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    long duration = pulseIn(echoPin, HIGH, 25000); // timeout ~25ms (max ~4m)
    long distance = duration * 0.034 / 2;
    if (distance <= 0 || distance > 400) distance = 400;
    return distance;
}

void beepDistance(long minDistance, bool latchPressed) {
    if (minDistance < 5) {
        if (latchPressed) {
            for (int note = 0; note < melodyLength; note++) {
                int duration = 1000 / durations[note];
                tone(BUZZER, melody[note], duration);
                int pauseBetweenNotes = duration * 1.30;
                delay(pauseBetweenNotes);
                noTone(BUZZER);
            }
        } else {
            tone(BUZZER, 1000);
        }
    } else if (minDistance < 15) {
        tone(BUZZER, 1000);
        delay(100);
        noTone(BUZZER);
        delay(100);
    } else if (minDistance < 25) {
        tone(BUZZER, 800);
        delay(100);
        noTone(BUZZER);
        delay(300);
    } else if( minDistance < 35){
        tone(BUZZER, 600);
        delay(100);
        noTone(BUZZER);
        delay(500);
    } else if( minDistance < 45){
        tone(BUZZER, 400);
        delay(200);
        noTone(BUZZER);
        delay(700);
    }
    else {
        noTone(BUZZER);
    }
}

void handleSerial() {
    if (Serial.available() > 0) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        cmd.toUpperCase();

        if (cmd == "START") {
            streaming = true;
        } else if (cmd == "STOP") {
            streaming = false;
        } else if (cmd == "MARIO") {
            melody = mario_melody;
            durations = mario_durations;
            melodyLength = sizeof(mario_durations) / sizeof(mario_durations[0]);
        } else if (cmd == "GAMEOVER") {
            melody = gameover_melody;
            durations = gameover_durations;
            melodyLength = sizeof(gameover_durations) / sizeof(gameover_durations[0]);
        } else if (cmd == "PACMAN") {
            melody = pacman_melody;
            durations = pacman_durations;
            melodyLength = sizeof(pacman_durations) / sizeof(pacman_durations[0]);
        } else if (cmd == "SQUIDGAME") {
            melody = squidgame_melody;
            durations = squidgame_durations;
            melodyLength = sizeof(squidgame_durations) / sizeof(squidgame_durations[0]);
        } else if (cmd == "TOKYO_DRIFT") {
            melody = tokyo_drift_melody;
            durations = tokyo_drift_durations;
            melodyLength = sizeof(tokyo_drift_durations) / sizeof(tokyo_drift_durations[0]);
        } else if (cmd == "MEASURE") {
            long distLeft = readUltrasonic(TRIG_LEFT, ECHO_LEFT);
            long distCenter = readUltrasonic(TRIG_CENTER, ECHO_CENTER);
            long distRight = readUltrasonic(TRIG_RIGHT, ECHO_RIGHT);
            bool colFront = digitalRead(LIMIT) == LOW;
            Serial.println(String(distCenter) + "," + String(distLeft) + "," + String(distRight) + "," + String(colFront));
        } else if (cmd.startsWith("SET_SERVO")) {
            String arg = cmd.substring(9);
            arg.trim();
            if (arg.length() > 0) {
                int a = arg.toInt();
                if (a < 0) a = 0; if (a > 180) a = 180;
                servoAngle = a;
                servoMotor.write(servoAngle);
            }
        } else if (cmd.startsWith("SETPOINT")) {
            String arg = cmd.substring(8);
            arg.trim();
            if (arg.equals("OFF")) {
                controlEnabled = false;
            } else {
                if (arg.length() > 0) {
                    int sp = arg.toInt();
                    if (sp < 2) sp = 2; if (sp > 200) sp = 200;
                    setpoint_cm = sp;
                    controlEnabled = true;
                }
            }
        } else if (cmd == "CONTROL ON") {
            controlEnabled = true;
        } else if (cmd == "CONTROL OFF") {
            controlEnabled = false;
        }
    }
}

void setup() {
    Serial.begin(9600);

    pinMode(TRIG_LEFT, OUTPUT);
    pinMode(ECHO_LEFT, INPUT);
    pinMode(TRIG_CENTER, OUTPUT);
    pinMode(ECHO_CENTER, INPUT);
    pinMode(TRIG_RIGHT, OUTPUT);
    pinMode(ECHO_RIGHT, INPUT);

    pinMode(LIMIT, INPUT_PULLUP);

    pinMode(BUZZER, OUTPUT);

    //Serial.println("distance_left,distance_center,distance_right,collision");

    servoMotor.attach(SERVO_PIN);
    servoMotor.write(servoAngle);
}

void loop() {
    handleSerial();
    long distLeft = readUltrasonic(TRIG_LEFT, ECHO_LEFT);
    long distCenter = readUltrasonic(TRIG_CENTER, ECHO_CENTER);
    long distRight = readUltrasonic(TRIG_RIGHT, ECHO_RIGHT);

    bool colFront = digitalRead(LIMIT) == LOW;

    long minDist = min(distLeft, min(distCenter, distRight));
    beepDistance(minDist, colFront);

    if (controlEnabled) {
        long error = setpoint_cm - distCenter;
        if (abs(error) > deadband_cm) {
            float delta = Kp * (float)error;
            if (delta > max_step_deg) delta = max_step_deg;
            if (delta < -max_step_deg) delta = -max_step_deg;
            int newAngle = servoAngle + (int)delta;
            if (newAngle < 0) newAngle = 0;
            if (newAngle > 180) newAngle = 180;
            if (newAngle != servoAngle) {
                servoAngle = newAngle;
                servoMotor.write(servoAngle);
            }
        }
    }

    if (streaming) {
        Serial.println(String(distCenter) + "," + String(distLeft) + "," + String(distRight) + "," + String(colFront));
    }
    delay(200); // ~5 samples per second
}
