#include <CapacitiveSensor.h>


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

/* Red, Yellow, Green LED Pin on/off booleans */
int led_r_13_On = false;
int led_y_12_On = false;
int led_g_11_On = false;

/* Variables for calculating touch sensor button hold time */
long btn_hold_start = 0;
long btn_hold_duration = 0;

/* Variables for calculating weight/capacitor timeout duration */
long timeout_startTime = 0;
long timeout_duration = 0;
int timeout_cap_threshold = 1000;

/* Max weight variable and creation of CapacitorSensor objects */
int max_supported_weight = 200000; // 800g (unit measure in grams)
CapacitiveSensor   button_cap = CapacitiveSensor(3,1);  
CapacitiveSensor   weight_cap = CapacitiveSensor(4,2);




/* Initilisation */
void setup()                    
{
  /* Opens serial port, configure pins 13, 12 and 11 for output. */
   Serial.begin(9600);
   pinMode(13, OUTPUT);
   pinMode(12, OUTPUT);
   pinMode(11, OUTPUT);
   Serial.println("Measure Cup Prototype - Initialised.");
}







/* Utility functions */

void printWeight(long weight_cap) 
{
  /*  Converts the cap  value to grams at 1 decimal place and print result. */
  weight_grams = weight_cap / 250;
  weight_grams = round(weight_grams*10)/10.0;
  Serial.println(String(weight_grams) + " grams.");
}

void activeLEDAnimation() 
{
  /* TODO: Activate each LED individually for 500ms and turns all off after 1 second. */
   digitalWrite(13, true); /* Red LED */
   delay(500);
   digitalWrite(12, true); /* Yellow LED */
   delay(500);
   digitalWrite(11, true); /* Green LED */
   delay(1000);
   /* Turn R, Y, G LEDs off after final delay */
   digitalWrite(13, false);
   digitalWrite(12, false);
   digitalWrite(11, false);
}

void deactiveLEDAnimation()
{
  /* Flashes Red LED Twice when measuring cup has been set to inactive */
  digitalWrite(13, true);
  delay(500);
  digitalWrite(13, false);
  delay(200);
  digitalWrite(13, true);
  delay(500);
  digitalWrite(13, false);
}

void flashLED(pin, ms) {
    /* Flashes an LED at specified pin for N milliseconds. */
    digitalWrite(pin, true);
    delay(ms);
    digitalWrite(pin, false);
}







/* Main loop function */

void loop()                    
{
     
    long btn_cap =  button_cap.capacitiveSensor(30);
    long w_cap = weight_cap.capacitiveSensor(30);

    led_r_13_On = false;
    led_y_12_On = false;
    led_g_11_On = false;



    /* If weight sensor is inactive and the touch button sensor has been pressed, record the sensor touch duration. If */
    if (not active && btn_cap >= 1000 && btn_hold_start = 0) {
      btn_hold_start = millis();
    
    } else if (not active && btn_cap >= 1000 && btn_hold_start > 0) {
      btn_hold_duration = millis() - btn_hold_start();
    
    } else if (btn_cap >= 1000 && not active && btn_hold_duration > 2000) {
      /* If the weight prototype is inactive and button sensor is being touched for longer than 2 seconds: activate */
      Serial.println("Starting weight measurer...");
      active = true;
      btn_hold_start = 0;
      btn_hold_duration = 0;
      activeLEDAnimation();
    
    } else {
      /* if btn_cap is less than 1000/isnt being pressed, reset the button press duration timer */
      btn_hold_start = 0;
      btn_hold_duration = 0;
    }


    
        


    if (active) {
      
      
      if (timeout_startTime = 0 and active and w_cap < timeout_cap_threshold) {
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
        active = false;
        timeout_startTime = 0;
        timeout_duration = 0;
        deactiveLEDAnimation();
      
      } else {
      
        if (w_cap < max_supported_weight) {
          
          if (w_cap < 1000) {
            /* if capacitor is less than 1000, do nothing. */
            continue;
          }
  
          else if (w_cap >= 1000 && w_cap < 15000) {
            /* if capacitor is between 1000 and 15000, flash GREEN LED every 500ms. */
            flashLED(11, 500);
          }
          
          else if (w_cap >= 15000 && w_cap < 40000) {
            /* if capacitor is between 15000 and 40000, turn GREEN LED on. */
            led_g_11_On = true;
          } 
  
          else if (w_cap >= 40000 && w_cap < 80000) {
            /* if capacitor is between 40000 and 80000, flash YELLOW LED every 500ms. */
            flashLED(12, 500);
          }
          
          else if (w_cap >= 80000 && w_cap < 100000) {
            /* if capacitor is between 80000 and 100000, turn YELLOW LED on. */
            led_y_12_On = true;
          }
  
          else if (w_cap >= 100000 && w_cap < 150000) {
            /* if capacitor is between 100000 and 150000, flash RED LED every 500ms. */
            flashLED(13, 500);
          }
          
          else {
            /* if capacitor is between 150000 and 200000, turn RED LED on. */
            led_r_13_On = true;
          }
        
        } else {
          /* if capacitor is greater than 200000, rapidly flash RED LED every 200ms. */
          Serial.println("Warning: exceeding the maximum supported weight!");
          flashLED(13, 200)
        }
  
        /* print the weight and update each LED pin with boolean values */
        printWeight(w_cap);
        digitalWrite(13, led_r_13_On);
        digitalWrite(12, led_y_12_On);
        digitalWrite(11, led_g_11_On);
      }
            
    }



}

