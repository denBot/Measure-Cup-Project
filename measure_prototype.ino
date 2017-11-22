#include <CapacitiveSensor.h>
#include <LiquidCrystal.h>

/* CS1Q HCI Measure Mug Prototype
 * Authored by 2086380A
 * 
 * Touch sensor button implimented through capacitive sensor (using pins 3 and 1 and 1 MOhm Resistor)
 * Mock weight sensor pad implimented through capacitive sensor (using pins 4 and 2 and 1 MOhm Resistor)
 * Red, Yellow and Green LEDs are connected to pins 13, 12 and 11. Each colour represents the closeness to the maximum weight the cup can hold.
 * Serial Monitor will be used as a mock LCD screen, therefore Serial.Print will be used to print data as an output.
 * 
 * Pin 13, 12, 11: On 
 */




/* How the software works:
 * 
 * By default, the measuring system is inactive/off. When a user presses on the 'power button' touch sensor for more than 2 seconds, the measuring system will start up.
 * When it starts up, all 3 Red, Yellow and Green LEDs will turn on in one by one and then turn off.
 * 
 * Since we are using a 2nd touch sensor to represent our weight sensor that we would ideally use in the final prototype, the harder the touch sensor is pressed, the more the LEDs change.
 * If the weight capacitor is less than 1000, no LEDs will light up.
 * If its greater than 1000 and less than 15000 it will flash green.
 * Greater than 15000 and less than 40000, it will light without flashing.
 * Greater than 40000 and less than 80000, it will flash yellow... and so on.
 * 
 * Each weight capacitor value will then be converted to grams by dividing the value by 250 (not sure what the actual maths would be) and then printed out.
 * 
 * If the capacitor reaches a value of over 200,000, the RED led will flash rapidly and a message will be printed stating that the cup has exceeded its maximum weight.
 * 
 * Id the weight capacitor isnt being used/is less than 1000 for over 15 seconds, the weight system will deactivate and will flash the RED led twice.
 * 
 */







/* Variables */

/*  Measure cup active status: false by default. */
boolean active = false;
boolean usingLCD = true;
long max_supported_weight = 200000;

/* Red, Yellow, Green LED Pin on/off booleans */
bool led_r_On = false;
bool led_y_On = false;
bool led_g_On = false;
bool led_b_On = false;
int led_r_pin = 5;
int led_y_pin = 4;
int led_g_pin = 3;
int led_b_pin = 2;
int buttonPin = 1;

/* Variables for calculating weight/capacitor timeout duration */
long btn_hold_start = 0;
long btn_hold_duration = 0;
long btn_hold_aim_duration = 2000;
long timeout_startTime = 0;
long timeout_duration = 0;
int timeout_cap_threshold = 1000;

CapacitiveSensor weight_cap = CapacitiveSensor(6,8);
LiquidCrystal lcd(7,9,10,11,12,13);





void setup()                    
{
  /* Opens serial port, configure pins 3, 4, 5 and 6 for RYGB LEDs, configures button. */
  
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






void loop()                    
{
    long w_cap = weight_cap.capacitiveSensor(30);

    led_r_On = false;
    led_y_On = false;
    led_g_On = false;
    led_b_On = false;

    /* If weight sensor is inactive and the touch button sensor has been pressed, record the sensor touch duration. If */
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


    if (active) {

      led_b_On = true;
      
      if (timeout_startTime == 0 and active and w_cap < timeout_cap_threshold) {
        timeout_startTime = millis();
      
      } else if (timeout_startTime > 0 and active and w_cap < timeout_cap_threshold) {
        timeout_duration = millis() - timeout_startTime;
      
      } else {
        /* If the w_cap value is higher than the timeout_cap_threshold, cancel timeout. */
        timeout_startTime = 0;
        timeout_duration = 0;
      }

      /* If w_cap has been < 1000 for over 15 seconds, deactivate the measuring system. */
      if (timeout_duration > 15000) {
        processTimeout();
      } else {
      
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
          Serial.println("Warning: exceeding the maximum supported weight!");
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

void printWeight(long weight_cap) 
{
  /*  Converts the cap  value to grams at 1 decimal place and print result. */
  float weight_grams = weight_cap / 250;
  weight_grams = round(weight_grams*10)/10.0;
  Serial.println(String(weight_grams) + " grams.");

  if (usingLCD)
  {
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(String(weight_grams) + " grams.");
  }
  
}

void disableLEDs()
{
  digitalWrite(led_r_pin, false);
  digitalWrite(led_y_pin, false);
  digitalWrite(led_g_pin, false);
  digitalWrite(led_b_pin, false);
}

void processTimeout()
{
  Serial.print("Timeout occurred, deactivating");
  active = false;
  timeout_startTime = 0;
  timeout_duration = 0;
  flashLED(led_b_pin, 300);
  flashLED(led_b_pin, 300);

  if (usingLCD)
  {
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

void turnOnAnimation()
{
  flashLED(led_r_pin, 100);
  flashLED(led_y_pin, 100);
  flashLED(led_g_pin, 100);
  flashLED(led_b_pin, 200);
  flashLED(led_g_pin, 100);
  flashLED(led_y_pin, 100);
  flashLED(led_r_pin, 200);
}


void activeLEDAnimation() 
{
  /* TODO: Activate each LED individually for 500ms and turns all off after 1 second. */
   digitalWrite(led_r_pin, true); /* Red LED */
   delay(200);
   digitalWrite(led_y_pin, true); /* Yellow LED */
   delay(200);
   digitalWrite(led_g_pin, true); /* Green LED */
   delay(200);
   digitalWrite(led_b_pin, true); /* Yellow LED */
   delay(500);
   /* Turn R, Y, G LEDs off after final delay */
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

