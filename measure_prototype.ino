#include <CapacitiveSensor.h>
#include <LiquidCrystal.h>

/* CS1Q HCI Measure Mug Prototype
 * Authored by 2086380A
 * 
 * Touch sensor button implimented through capacitive sensor (using pins 3 and 1 and 1 MOhm Resistor)
 * Mock weight sensor pad implimented through capacitive sensor (using pins 6 and 8 and 1 MOhm Resistor)
 * Red, Yellow, Green and Blue LEDs are connected to pins 5, 4, 3 and 2.
 * Remaining pins are used with LiquidCrystal to impliment a functioning LCD screen.
 */

CapacitiveSensor weight_cap = CapacitiveSensor(7, 6);
LiquidCrystal lcd(8,9,10,11,12,13);

bool active = false;
bool usingLCD = true;
bool led_r_On = false;
bool led_y_On = false;
bool led_g_On = false;
bool led_b_On = false;

int led_r_pin = 5;
int led_y_pin = 4;
int led_g_pin = 3;
int led_b_pin = 2;
int buttonPin = 1;

long max_supported_weight = 200000.00;
long btn_hold_start = 0;
long btn_hold_duration = 0;
long btn_hold_aim_duration = 2000;
long timeout_startTime = 0;
long timeout_duration = 0;
int timeout_cap_threshold = 1000;

void setup() {
  
   Serial.begin(9600);
   pinMode(buttonPin, INPUT_PULLUP);  
   pinMode(led_r_pin, OUTPUT);
   pinMode(led_y_pin, OUTPUT);
   pinMode(led_g_pin, OUTPUT);
   pinMode(led_b_pin, OUTPUT);
   Serial.println("Measure Cup Prototype - Initialised.");
   
   if (usingLCD) {
     lcd.clear();
     lcd.begin(16,2);
     lcd.setCursor(0,0);
     lcd.print("Measure Cup");
     lcd.setCursor(0,1);
     lcd.print("Prototype");
   }
   turnOnAnimation();
}






void loop() {
    
    long w_cap = weight_cap.capacitiveSensor(30);
    led_r_On = false;
    led_y_On = false;
    led_g_On = false;
    led_b_On = false;
    checkButton();

    if (active) {

      led_b_On = true;
      
      checkTimeout(w_cap);

      /* If w_cap has been < 1000 for over 15 seconds, deactivate the measuring system. */
      if (timeout_duration > 15000) {
        processTimeout();
      
      } else {
        
        Serial.println("Weight Capacitor: "+String(w_cap));
        
        if (w_cap < max_supported_weight) {
          if (w_cap < 1000) {
            /* do nothing */
          } else if (w_cap >= 1000 && w_cap < 15000) {
            flashLED(led_g_pin, 500);
          } else if (w_cap >= 15000 && w_cap < 40000) {
            led_g_On = true;
          } else if (w_cap >= 40000 && w_cap < 80000) {
            flashLED(led_y_pin, 500);
          } else if (w_cap >= 80000 && w_cap < 100000) {
            led_y_On = true;
          } else if (w_cap >= 100000 && w_cap < 150000) {
            flashLED(led_r_pin, 500);
          } else {
            led_r_On = true;
          }
        } else {
          flashLED(led_r_pin, 100);
        }
  
        /* print the weight and update each LED pin with boolean values */
        printWeight(w_cap);
        
        digitalWrite(led_r_pin, led_r_On);
        digitalWrite(led_y_pin, led_y_On);
        digitalWrite(led_g_pin, led_g_On);
        digitalWrite(led_b_pin, led_b_On);
      }
            
    } else {
      disableLEDs();
    }
}








/* Utility functions */

void checkButton() {
    if (not active && digitalRead(buttonPin) == LOW) {
    
    if (btn_hold_start == 0) {
      btn_hold_start = millis();
      Serial.println("Button Pressed! Starting hold count at: "+String(btn_hold_start));
    
    } else if (btn_hold_duration < btn_hold_aim_duration) {
      btn_hold_duration = millis() - btn_hold_start;
      Serial.println(btn_hold_duration);
    
    } else {
      Serial.println("Starting weight measurer...");
      active = true;
      btn_hold_start = 0;
      btn_hold_duration = 0;
      activeLEDAnimation();
    }
    
  } else {
    btn_hold_start = 0;
    btn_hold_duration = 0;
  }
}

void checkTimeout(long w_cap) {
  if (timeout_startTime == 0 and active and w_cap < timeout_cap_threshold) {
    timeout_startTime = millis();
  
  } else if (timeout_startTime > 0 and active and w_cap < timeout_cap_threshold) {
    timeout_duration = millis() - timeout_startTime;
  
  } else {
    /* If the w_cap value is higher than the timeout_cap_threshold, cancel timeout. */
    timeout_startTime = 0;
    timeout_duration = 0;
  }
}

void printWeight(long weight_cap) {
  /*  Converts the cap  value to grams at 1 decimal place and print result. */
  float weight_grams = weight_cap / 250;
  weight_grams = round(weight_grams*10)/10.0;
  Serial.println(String(weight_grams) + " grams.");

  if (usingLCD) {
    lcd.clear();
    lcd.setCursor(0,1);
    if (weight_cap < max_supported_weight) {
      lcd.print(String(weight_grams) + " grams.");
    } else {
      Serial.println("Warning: exceeding the maximum supported weight!");
      lcd.setCursor(0,0);
      lcd.print(String(weight_grams) + " grams.");
      lcd.setCursor(0,1);
      lcd.print("Exceeding max weight!");
    }
  }
}

void disableLEDs() {
  digitalWrite(led_r_pin, false);
  digitalWrite(led_y_pin, false);
  digitalWrite(led_g_pin, false);
  digitalWrite(led_b_pin, false);
}

void processTimeout() {
  Serial.print("Timeout occurred, deactivating");
  active = false;
  timeout_startTime = 0;
  timeout_duration = 0;
  flashLED(led_b_pin, 300);
  flashLED(led_b_pin, 300);

  if (usingLCD) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Timeout due to:");
    lcd.setCursor(0,1);
    lcd.print("Scale Inacvitiy.");
    delay(3000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Hold button for");
    lcd.setCursor(0,1);
    lcd.print("3s to start!");
  }
}

void turnOnAnimation() {
  flashLED(led_r_pin, 100);
  flashLED(led_y_pin, 100);
  flashLED(led_g_pin, 100);
  flashLED(led_b_pin, 200);
  flashLED(led_g_pin, 100);
  flashLED(led_y_pin, 100);
  flashLED(led_r_pin, 200);
}

void activeLEDAnimation() {
   digitalWrite(led_r_pin, true);
   delay(200);
   digitalWrite(led_y_pin, true);
   delay(200);
   digitalWrite(led_g_pin, true);
   delay(200);
   digitalWrite(led_b_pin, true);
   delay(500);
   
   digitalWrite(led_r_pin, false);
   digitalWrite(led_y_pin, false);
   digitalWrite(led_g_pin, false);
   digitalWrite(led_b_pin, false);
}

void flashLED(int pin, int ms) {
    /* Flashes an LED at specified pin for N milliseconds. */
    digitalWrite(pin, true);
    delay(ms);
    digitalWrite(pin, false);
}

