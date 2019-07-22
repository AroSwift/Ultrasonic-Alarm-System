#include <NewPing.h> // By Tim Eckel
#include <LiquidCrystal.h> // By Arduino

// Pins
const int trigger_pin = 6;
const int echo_pin = 5;
//const int btn_pins[] = { 2, 3, 4, 13 };
const int btn_pins[] = { 4, 3, 2, 13 };

// Button variables
const int btn_unlock_indices[] = { 0, 1, 3 };
int btn_user_indices[3];
int btn_user_indice = 0;
int btn_iterator = 0;
boolean btn_state;

// Distance variables
const int max_distance = 150;
const int distance_lvl_1 = max_distance;
const int distance_lvl_2 = 60;
const int distance_lvl_3 = 25;
const int margin_error_distance = 3;
unsigned long current_distance = 0;
int current_lvl = 1;
boolean changed_lvl = true;

// State of alarm
unsigned long last_update;
boolean locked = false;
boolean armed = true;
boolean arming = false;
const int lock_time = 10000;
const int rearm_time = 5000;

// NewPing setup of pins and maximum distance
NewPing sonar(trigger_pin, echo_pin, max_distance);

// Initialize LiquidCrystal with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  // Note: this is the only baud rate that will return results
  Serial.begin(9600);
  // Setup the LCD's number of columns and rows
  lcd.begin(16, 2);
  // Set default last update
  last_update = millis();
  // Set the button pins to input
  for(int i = 0; i < sizeof(btn_pins)/sizeof(int); i++) {
    pinMode(btn_pins[i], INPUT_PULLUP);
  }
}

void loop() {
  // When the alarm is armed and not locked
  if(armed && !locked) {
    active_alarm();
  // When the alarm is armed and locked
  } else if(armed && locked) {
    locked_alarm();
  }

  // Constantly check if the alarm should be disarmed
  check_disarm();
}

void active_alarm() {
  // Note: 29ms should be the shortest delay between pings
  delay(50); // Arbitrarily chose 50 milliseconds
  current_distance = sonar.ping_cm();

  // Get the type of alarm level
  if(current_distance <= margin_error_distance) {
    check_lvl_change(1);
  } else if(current_distance <= distance_lvl_3) {
    check_lvl_change(3);
  } else if(current_distance <= distance_lvl_2) {
    check_lvl_change(2);
  } else if(current_distance <= distance_lvl_1) {
    check_lvl_change(1);
  }

  // When the alarm level changed
  if(changed_lvl) {
    // Reset the LCD state
    lcd.setCursor(0, 0);
    lcd.clear();

    // Determine and set the new alarm level
    switch(current_lvl) {
      case 1:
        alarm_lvl_1();
        break;
      case 2:
        alarm_lvl_2();
        break;
      case 3:
        alarm_lvl_3();
        break;
      default:
        alarm_lvl_1();
        break;
    }
    // The level has been changed
    changed_lvl = false;
  }
}

void locked_alarm() {
  // Wait for the designated amount of time before unlocking alarm
  if(millis() - last_update > lock_time) {
    locked = false;
  }
}

void check_disarm() {
  // When we are getting ready to re-arm alarm
  if(arming) {
    // Wait for the designated amount of time before re-arming
    if(millis() - last_update > rearm_time) {
      rearm_alarm();
      reset_disarm();
    }
  }

  // When the user has entered the number of button presses needed to unlock the alarm
  if(btn_user_indice >= sizeof(btn_unlock_indices)/sizeof(int)) {
    // disarm alarm by default
    armed = false;

    // Iterate over the button indices
    for(int i = 0; i < sizeof(btn_user_indices)/sizeof(int); i++) {
      // When the incorrectly entered a button, keep the alarm armed
      if(btn_user_indices[i] != btn_unlock_indices[i]) {
        armed = true;
      }
    }

    // Regardless of result, reset disarm variables
    reset_disarm();

    // When the user re-enters pass combination
    if(arming && !armed) {
      rearm_alarm();
    } else if(!armed) { // Alarm disarmed
      // Inform user alarm is disarmed
      lcd.setCursor(0, 0);
      lcd.clear();
      lcd.print("Alarm disarmed");
      Serial.println("Alarm disarmed");
      arming = true;
      last_update = millis();
    }
  } else { // Button sequence not complete

    // Read a button's state
    btn_state = digitalRead(btn_pins[btn_iterator]);
    
    // When the button connection is closed (it is always on)
    if (btn_state == LOW) {
      // Capture the user's button indice 
      btn_user_indices[btn_user_indice] = btn_iterator;
      btn_user_indice++;
      // Prevent double button capture
      delay(300);
    }

    // When we have iterated over all the button pins
    if(btn_iterator >= sizeof(btn_pins)/sizeof(int)-1) {
      // Reset the button iterator
      btn_iterator = 0;
    } else {
      // Go to next button pin
      btn_iterator++;
    }
  }
}

void reset_disarm() {
  // Reset the current button indice
  btn_iterator = 0;
  // Reset the user's entered button indices
  memset(btn_user_indices, 0, sizeof(btn_user_indices));
  btn_user_indice = 0;
  // And reset the alarm level
  current_lvl = 1;
  changed_lvl = true;
}

void rearm_alarm() {
  // Reset disarm variables
  locked = false;
  armed = true;
  arming = false;
  // Inform user that alarm is re-armed
  lcd.setCursor(0, 0);
  lcd.clear();
  lcd.print("Alarm re-armed");
  Serial.println("Alarm re-armed");
  delay(1000);
}

void check_lvl_change(int new_lvl) {
  // When the level has changed
  if(current_lvl != new_lvl) {
    changed_lvl = true;
  }
  // Set the new level
  current_lvl = new_lvl;
}

void alarm_lvl_1() {
  lcd.print("Stay back!");
  Serial.println("Stay back!");
}

void alarm_lvl_2() {
  lcd.print("Final warning!");
  Serial.println("Final warning!");
}

void alarm_lvl_3() {
  lcd.print("Alarm Triggered!");
  Serial.println("Alarm Triggered!");
  // Lock the alarm down
  locked = true;
  last_update = millis();
}

