#include "pitches.h"
#include <Servo.h>
#define TRIG_LEFT 13
#define ECHO_LEFT 11

#define TRIG_CENTER 6
#define ECHO_CENTER 9

#define TRIG_RIGHT 5
#define ECHO_RIGHT 10

#define LIMIT 2
#define BUZZER 7

#define SERVO_LEFT_PIN 3
#define SERVO_RIGHT_PIN 4

const int SERVO_STOP = 90;
const int SERVO_DRIVE = 25;
const bool SERVO_LEFT_REVERSED = false;
const bool SERVO_RIGHT_REVERSED = true;

Servo servoLeft;
Servo servoRight;

bool autoDriveActive = false;
int targetDistanceCm = 0;
const int TARGET_TOL = 2;

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
    1
};

int* melody = mario_melody;
int* durations = mario_durations;
int melodyLength = sizeof(mario_durations) / sizeof(mario_durations[0]);

bool streaming = true;

void writeServoDir(Servo &s, bool reversed, int dir) {
  int val = SERVO_STOP + (reversed ? -1 : 1) * dir * SERVO_DRIVE;
  s.write(val);
}

void driveStop() {
  writeServoDir(servoLeft, SERVO_LEFT_REVERSED, 0);
  writeServoDir(servoRight, SERVO_RIGHT_REVERSED, 0);
}

void driveForward() {
  writeServoDir(servoLeft, SERVO_LEFT_REVERSED, +1);
  writeServoDir(servoRight, SERVO_RIGHT_REVERSED, +1);
}

void driveBackward() {
  writeServoDir(servoLeft, SERVO_LEFT_REVERSED, -1);
  writeServoDir(servoRight, SERVO_RIGHT_REVERSED, -1);
}

void updateAutoDrive(long distCenter, long minDist, bool colFront) {
  if (!autoDriveActive) return;
  if (minDist < 5 || colFront) {
    driveStop();
    autoDriveActive = false;
    return;
  }
  if (distCenter > targetDistanceCm + TARGET_TOL) {
    driveBackward();
  } else if (distCenter < targetDistanceCm - TARGET_TOL) {
    driveForward();
  } else {
    driveStop();
    autoDriveActive = false;
  }
}

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
  if (!streaming) {
    noTone(BUZZER);
    return;
  }
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
            autoDriveActive = false;
            driveStop();
        } else if (cmd.startsWith("GOTO ")) {
            int value = cmd.substring(5).toInt();
            if (value < 5) value = 5;
            if (value > 200) value = 200;
            targetDistanceCm = value;
            autoDriveActive = true;
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
            Serial.println(String(distLeft) + "," + String(distCenter) + "," + String(distRight) + "," + String(colFront));
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

    servoLeft.attach(SERVO_LEFT_PIN);
    servoRight.attach(SERVO_RIGHT_PIN);
    driveStop();

    //Serial.println("distance_left,distance_center,distance_right,collision");
}

void loop() {
    handleSerial();
    long distLeft = readUltrasonic(TRIG_LEFT, ECHO_LEFT);
    long distCenter = readUltrasonic(TRIG_CENTER, ECHO_CENTER);
    long distRight = readUltrasonic(TRIG_RIGHT, ECHO_RIGHT);

    bool colFront = digitalRead(LIMIT) == LOW;

    long minDist = min(distLeft, min(distCenter, distRight));
    beepDistance(minDist, colFront);

    updateAutoDrive(distCenter, minDist, colFront);

    if (streaming) {
        Serial.println(String(distLeft) + "," + String(distCenter) + "," + String(distRight) + "," + String(colFront));
    }
    delay(200); // ~5 samples per second
}