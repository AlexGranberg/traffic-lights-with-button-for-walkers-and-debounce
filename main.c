#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "millis.h"

#define BIT_SET(a, b) ((a) |= (1 << (b)))
#define BIT_CLEAR(a, b) ((a) &= ~(1 << (b)))
#define BIT_FLIP(a, b) ((a) ^= (1 << (b)))
#define BIT_CHECK(a, b) (!!((a) & (1 << (b))))

#define YELLOW 2
#define RED 3
#define GREEN 1
#define BUTTON_INPUT 4
#define GREEN_WALK 7
#define RED_WALK 6

#define DEBOUNCE_DELAY 50       // Debounce time in milliseconds
#define YELLOW_DURATION 5000    // Yellow state duration in milliseconds
#define RED_DURATION 12000      // Red state duration in milliseconds

enum ButtonState {
  BUTTON_IDLE,
  BUTTON_PRESSED,
  BUTTON_RELEASED
};

enum State {
  STATE_GREEN,
  STATE_YELLOW,
  STATE_RED,
  STATE_YELLOW_TO_GREEN
};

int main(void) {
  millis_init();
  sei();
  bool buttonState = false;
  bool lastButtonState = false;
  unsigned long debounceTime = 0;  // Timestamp to track debounce delay
  unsigned long stateStartTime = 0;  // Timestamp to track state start time
  enum State state = STATE_GREEN;
  enum ButtonState button = BUTTON_IDLE;

  BIT_SET(DDRB, YELLOW);
  BIT_SET(DDRB, RED);
  BIT_SET(DDRB, GREEN);
  BIT_SET(DDRD, GREEN_WALK);
  BIT_SET(DDRD, RED_WALK);
  BIT_CLEAR(DDRD, BUTTON_INPUT);
  BIT_SET(PORTD, BUTTON_INPUT);
  BIT_SET(PORTD, RED_WALK);
  BIT_SET(PORTB, GREEN);

  while (1) {
    unsigned long current_millis = millis_get();
    unsigned long elapsed_millis = current_millis - stateStartTime;

    // Read the current state of the button
    buttonState = !(PIND & (1 << BUTTON_INPUT));

    // Button state
    switch (button) {
      case BUTTON_IDLE:
        if (state == STATE_GREEN && buttonState && !lastButtonState) {
          button = BUTTON_PRESSED;
          debounceTime = current_millis;
        }
        break;
      case BUTTON_PRESSED:
        if (!buttonState && current_millis - debounceTime >= DEBOUNCE_DELAY) {
          button = BUTTON_RELEASED;
          debounceTime = current_millis;
        }
        break;
      case BUTTON_RELEASED:
        if (current_millis - debounceTime >= DEBOUNCE_DELAY) {
          button = BUTTON_IDLE;

          if (state == STATE_GREEN) {
            state = STATE_YELLOW;
            stateStartTime = current_millis;
          }
        }
        break;
    }

    // Update the last button state
    lastButtonState = buttonState;

    if (state == STATE_GREEN) {
      BIT_SET(PORTB, GREEN);
      BIT_CLEAR(PORTB, YELLOW);
      BIT_CLEAR(PORTB, RED);
      BIT_CLEAR(PORTD, GREEN_WALK);
      BIT_SET(PORTD, RED_WALK);
    } else if (state == STATE_YELLOW) {
      BIT_SET(PORTB, YELLOW);
      BIT_CLEAR(PORTB, RED);
      BIT_CLEAR(PORTB, GREEN);
      BIT_CLEAR(PORTD, GREEN_WALK);
      BIT_SET(PORTD, RED_WALK);
        // Check if yellow state duration has elapsed
        if (stateStartTime == 0) {
            // Transition from green to yellow
            state = STATE_YELLOW;
            stateStartTime = current_millis;
        }

        if (elapsed_millis >= YELLOW_DURATION && stateStartTime > 0) {
            if (stateStartTime == current_millis - YELLOW_DURATION) {
            // Transition from yellow to red
            state = STATE_RED;
            stateStartTime = current_millis;
            }
        }
    } else if (state == STATE_RED) {
      BIT_CLEAR(PORTB, YELLOW);
      BIT_SET(PORTB, RED);
      BIT_CLEAR(PORTB, GREEN);
      BIT_SET(PORTD, GREEN_WALK);
      BIT_CLEAR(PORTD, RED_WALK);

        // Check if red state duration has elapsed
        if (elapsed_millis >= RED_DURATION) {
            if (stateStartTime + RED_DURATION <= current_millis) {
            // Transition from red to yellow
            state = STATE_YELLOW_TO_GREEN;
            stateStartTime = current_millis;
            }
        }
    } else if (state == STATE_YELLOW_TO_GREEN) {
      BIT_SET(PORTB, YELLOW);
      BIT_CLEAR(PORTB, RED);
      BIT_CLEAR(PORTB, GREEN);
      BIT_CLEAR(PORTD, GREEN_WALK);
      BIT_SET(PORTD, RED_WALK);

        // Check if yellow state duration has elapsed
        if (elapsed_millis >= YELLOW_DURATION) {
            if (stateStartTime + YELLOW_DURATION <= current_millis) {
            // Transition from yellow to green
            state = STATE_GREEN;
            stateStartTime = 0;
            }
        }
    }
}

return 0;
}
