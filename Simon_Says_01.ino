#include <Bounce.h>
#include "./pitches.h"

const int STARTING_LEVEL = 1;
const int MAX_LEVEL = 100;
const int STARTING_VELOCITY = 500; // ms
const int MIN_VELOCITY = 50;  // ms
const int BUTTON_COUNT = 4;

int sequence[MAX_LEVEL];
int level = STARTING_LEVEL;
int velocity = STARTING_VELOCITY;

// define all the pins

const int LED1_PIN = 13;
const int LED2_PIN = 12;
const int LED3_PIN = 11;
const int LED4_PIN = 10;

const int BUTTON1_PIN = 9;
const int BUTTON2_PIN = 8;
const int BUTTON3_PIN = 7;
const int BUTTON4_PIN = 6;

const int SPEAKER_PIN = 3;

// use the Bounce library to debounce the buttons
Bounce bounces[] = {
  Bounce(BUTTON1_PIN, 10),
  Bounce(BUTTON2_PIN, 10),
  Bounce(BUTTON3_PIN, 10),
  Bounce(BUTTON4_PIN, 10)
};

// define a struct to represent a button/LED combo
typedef struct {
  int buttonPin;  // which pin is the button connected to?
  int ledPin;     // which pin is the LED connected to?
  int note;       // which musical note does the button play?
} button;

// define the array of button structs
const button BUTTONS[] = {
  {BUTTON1_PIN, LED1_PIN, NOTE_C4},
  {BUTTON2_PIN, LED2_PIN, NOTE_D4},
  {BUTTON3_PIN, LED3_PIN, NOTE_E4},
  {BUTTON4_PIN, LED4_PIN, NOTE_F4}
};

void setup() {
  Serial.begin(9600);
  for (int i=0; i < BUTTON_COUNT; i++) {
    pinMode(BUTTONS[i].buttonPin, INPUT_PULLUP);
    pinMode(BUTTONS[i].ledPin, OUTPUT);
  }
  pinMode(SPEAKER_PIN, OUTPUT);
}

void loop() {
  bool waitingToStart = (level == STARTING_LEVEL);

  if (waitingToStart) {
    // check if the player has pushed the start button (button 0) yet
    if (bounces[0].update() && bounces[0].fallingEdge()) {
      waitingToStart = false;
    }
  }

  if (waitingToStart) {
    // keep generating random sequences and flashing the lights
    generateSequence();
    flashEachLight();
  } else {
    // show the next sequence and wait for the player to copy it
    showSequence();
    getSequence();
  }
}

// fill the sequence array with a random button sequence
void generateSequence() {
  randomSeed(millis());

  for (int i = 0; i < MAX_LEVEL; i++) {
    sequence[i] = random(0, BUTTON_COUNT);
  }
}

void showSequence() {
  button b;
  lightsOff();

  Serial.print("Q: ");

  for (int i = 0; i < level; i++) {
    Serial.print(sequence[i]);
    Serial.print(" ");
    
    // load up the next button in the sequence
    b = BUTTONS[sequence[i]];
    flashLightPlayNote(b);
    delay(200);
  }

  Serial.println("");
}

void getSequence() {
  bool buttonPressed;
  int bi;   // the index of the button that was pressed
  button b; // the corresponding button struct

  Serial.print("A: ");

  for (int i = 0; i < level; i++) {
    buttonPressed = false;

    // wait for a button press
    while (!buttonPressed) {
      for (int i = 0; i < BUTTON_COUNT; i++) {
        if (bounces[i].update() && bounces[i].fallingEdge()) {
          buttonPressed = true;
          bi = i;
          b = BUTTONS[bi];
          break;
        }
      }
    }

    Serial.print(bi);
    Serial.print(" ");
    flashLightPlayNote(b);

    // if the wrong button was pressed we’re done, go to the bad place
    if (bi != sequence[i]) {
      wrongSequence();
      Serial.println("");
      return;
    }
  }

  // if we got here, the player got the sequence right, go to the happy place
  rightSequence();
  Serial.println("");
}

void rightSequence() {
  Serial.print(" YES!");
  lightsOff();
  delay(250);

  lightsOn();
  playHappyNoise();
  maybePlayNazareth(level);

  lightsOff();
  delay(500);

  // level up!
  if (level < MAX_LEVEL) {
    level++;
    velocity -= 50;  // increases difficulty
    if (velocity < MIN_VELOCITY) {
      velocity = MIN_VELOCITY;
    }
  }
}

void wrongSequence() {
  Serial.print(" NO.");

  for (int i = 0; i < 3; i++) {
    lightsOn();
    tone(SPEAKER_PIN, 233);
    delay(250);

    noTone(SPEAKER_PIN);
    delay(250);
  }

  // back to the beginning
  level = STARTING_VELOCITY;
  velocity = STARTING_VELOCITY;
}

void flashLightPlayNote(button b) {
  digitalWrite(b.ledPin, HIGH);
  tone(SPEAKER_PIN, b.note);
  delay(velocity);
  digitalWrite(b.ledPin, LOW);
  noTone(SPEAKER_PIN);
}

// lights

void lightsOn(void) {
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  digitalWrite(LED3_PIN, HIGH);
  digitalWrite(LED4_PIN, HIGH);
}

void lightsOff(void) {
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
}

// flash all the LEDs in a sequence
void flashEachLight(void) {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    digitalWrite(BUTTONS[i].ledPin, HIGH);
    // TODO(ben): replace this delay (and all the others) with elapsedMillis
    // timers to make the controls more responsive?
    delay(60);
    digitalWrite(BUTTONS[i].ledPin, LOW);
  }
}

// music

void playHappyNoise(void) {
  tone(SPEAKER_PIN, NOTE_C4);
  delay(50);
  tone(SPEAKER_PIN, NOTE_E4);
  delay(50);
  tone(SPEAKER_PIN, NOTE_G4);
  delay(50);
  tone(SPEAKER_PIN, NOTE_C5);
  delay(100);
  noTone(SPEAKER_PIN);
}

void maybePlayNazareth(int level) {
  if (level % 4 == 0) {
    delay(200);
    // things are really cool
    tone(SPEAKER_PIN, NOTE_C4);
    delay(150);
    noTone(SPEAKER_PIN);
    delay(50);
    tone(SPEAKER_PIN, NOTE_C4);
    delay(200);
    tone(SPEAKER_PIN, NOTE_E4);
    delay(200);
    tone(SPEAKER_PIN, NOTE_G4);
    delay(200);
    noTone(SPEAKER_PIN);
    delay(200);
    tone(SPEAKER_PIN, NOTE_B4);
    delay(1200);
    noTone(SPEAKER_PIN);
    if (level >= 8) {
      delay(200);
      // in Nazareth
      tone(SPEAKER_PIN, NOTE_E4);
      delay(400);
      tone(SPEAKER_PIN, NOTE_D4);
      delay(400);
      tone(SPEAKER_PIN, NOTE_C4);
      delay(150);
      noTone(SPEAKER_PIN);
      delay(50);
      tone(SPEAKER_PIN, NOTE_C4);
      delay(800);
      noTone(SPEAKER_PIN);
    }
    if (level >= 12) {
      delay(800);
      // the city is full of joy
      tone(SPEAKER_PIN, NOTE_E4);
      delay(150);
      noTone(SPEAKER_PIN);
      delay(50);
      tone(SPEAKER_PIN, NOTE_E4);
      delay(200);
      tone(SPEAKER_PIN, NOTE_D4);
      delay(200);
      tone(SPEAKER_PIN, NOTE_C4);
      delay(400);
      tone(SPEAKER_PIN, NOTE_D4);
      delay(800);
      tone(SPEAKER_PIN, NOTE_E4);
      delay(600);
      tone(SPEAKER_PIN, NOTE_C4);
      delay(800);
      noTone(SPEAKER_PIN);
    }
    if (level >= 16) {
      delay(200);
      // and this is where girl meets boy
      tone(SPEAKER_PIN, NOTE_E4);
      delay(150);
      noTone(SPEAKER_PIN);
      delay(50);
      tone(SPEAKER_PIN, NOTE_E4);
      delay(200);
      tone(SPEAKER_PIN, NOTE_D4);
      delay(200);
      tone(SPEAKER_PIN, NOTE_C4);
      delay(400);
      tone(SPEAKER_PIN, NOTE_A4);
      delay(600);
      tone(SPEAKER_PIN, NOTE_E4);
      delay(600);
      tone(SPEAKER_PIN, NOTE_G4);
      delay(1000);
    }
  }
}
