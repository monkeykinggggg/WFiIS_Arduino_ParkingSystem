#include "pitches.h"
#define TRIG_FRONT 13
#define ECHO_FRONT 11


#define LIMIT_FRONT 2
#define BUZZER 7


int mario_melody[] = {
  NOTE_E5, NOTE_E5, REST, NOTE_E5, REST, NOTE_C5, NOTE_E5,
  NOTE_G5, REST, NOTE_G4, REST, 
  NOTE_C5, NOTE_G4, REST, NOTE_E4,
  NOTE_A4, NOTE_B4, NOTE_AS4, NOTE_A4,
  NOTE_G4, NOTE_E5, NOTE_G5, NOTE_A5, NOTE_F5, NOTE_G5,
};

int mario_durations[] = {
  8, 8, 8, 8, 8, 8, 8,
  4, 4, 8, 4, 
  4, 8, 4, 4,
  4, 4, 8, 4,
  8, 8, 8, 4, 8, 8,
  8, 4,8, 8, 4,
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

int melody[] = {
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

int durations[] = {
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
    if(latchPressed){
       int size = sizeof(durations) / sizeof(int); 
       for (int note = 0; note < size; note++) {
         int duration = 1000 / durations[note]; 
         tone(BUZZER, melody[note], duration);
         int pauseBetweenNotes = duration * 1.30; 
         delay(pauseBetweenNotes);
         noTone(BUZZER); 
      } 
    }else{
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

void setup() {
  Serial.begin(9600);
  
  pinMode(TRIG_FRONT, OUTPUT);
  pinMode(ECHO_FRONT, INPUT);
//  pinMode(TRIG_LEFT, OUTPUT);
//  pinMode(ECHO_LEFT, INPUT);
//  pinMode(TRIG_RIGHT, OUTPUT);
//  pinMode(ECHO_RIGHT, INPUT);
  
  pinMode(LIMIT_FRONT, INPUT_PULLUP);
//  pinMode(LIMIT_REAR, INPUT_PULLUP);
  
  pinMode(BUZZER, OUTPUT);
  
  //Serial.println("distance_front,distance_left,distance_right,collision_front,collision_rear");
  Serial.println("distance_front,collision_front");
}

void loop() {
  long distFront = readUltrasonic(TRIG_FRONT, ECHO_FRONT);
  
  bool colFront = digitalRead(LIMIT_FRONT) == LOW;
  
  long minDist = distFront;
  beepDistance(minDist, colFront);
  
  Serial.print(distFront);
  Serial.print(",");

  Serial.print(colFront);

  Serial.println("");
  delay(200); // ~5 samples per second
}
