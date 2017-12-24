#include <MoistureSensor.h>
#include <LiquidCrystal.h>

#define FLOAT_SWITCH 7
#define PUMP 6
#define LED 13

#define PUMP_ON_TIME 1000
#define FILLING_INTERVAL 300000
#define WAIT_AFTER_LOW_WATER 30000
#define WAIT_AFER_MOIST 120000

#define MOISTURE_PIN A0
#define MOISTURE_ENABLE 3
MoistureSensor moistureSensor(MOISTURE_PIN, MOISTURE_ENABLE);

const int rs = 4, en = 8, d4 = A2, d5 = A3, d6 = A4, d7 = A5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

long prevTimePumpOn = 0;
long prevTimeMoistureValuePrinted = -50000;
long prevTimeForTimeDisplay = -1000;
long prevTimePumpDelay = -1;
int prevMoistureLevel = -1;

int level = 0;

void setup() {
    pinMode(PUMP, OUTPUT);
    digitalWrite(LED, LOW);
    pumpOff();
    pinMode(FLOAT_SWITCH, INPUT_PULLUP);
    pinMode(PUMP, OUTPUT);
    pinMode(LED, OUTPUT);

    Serial.begin(9600);

    lcd.begin(20, 4);
    lcd.noAutoscroll();
    lcd.clear();
    lcd.print("Moisture: ");
    lcd.setCursor(0, 2);
    lcd.print("Watered ");
    
    moistureSensor.begin();
}

void loop() {
    if (millis() - prevTimeMoistureValuePrinted > 1000) {
        unsigned int moisture = moistureSensor.read();
        lcd.setCursor(17,0);
        if (moisture > 999) {
            lcd.print(" ");
        }
        lcd.print(moisture);
        Serial.print("Moisture: ");
        Serial.println(moisture);
        lcd.setCursor(0, 1);
        prevTimeMoistureValuePrinted = millis();

        // TODO: Maybe shouldn't be in this if statement
        level = moistureSensor.level();
        
        if (level != prevMoistureLevel) {
            lcd.setCursor(0,1);
            if (level == MOISTURE_DRY) {
                lcd.print("DRY  ");
                Serial.println("DRY");
            } else if (level == MOISTURE_OKAY) {
                lcd.print("IDEAL");
                Serial.println("OKAY");
            } else if (level == MOISTURE_WET) {
                lcd.print("WET  ");
                Serial.println("WET");
            }
            lcd.print("  "); // due to word of different lengths
            
            prevMoistureLevel = level;
        }
    }

    unsigned long diff = millis() - prevTimePumpOn;

    if (millis() - prevTimeForTimeDisplay > 1000) {
        lcd.setCursor(8, 2);
        unsigned long seconds = diff / 1000;
        if (seconds < 60) {
            lcd.print(seconds);
            lcd.print(" s ago   ");
            Serial.print(seconds);
            Serial.println(" s ago");
            lcd.noBlink();
        } else if (seconds < 7200) {
            unsigned long minutes = seconds / 60;
            lcd.print(minutes);
            lcd.print(" m ago   ");
            Serial.print(minutes);
            Serial.println(" m ago");
            lcd.blink();
        } else {
            unsigned long hours = seconds / 3600;
            lcd.print(hours);
            lcd.print(" h ago   ");
            Serial.print(hours);
            Serial.println(" h ago");
        }
        prevTimeForTimeDisplay = millis();
    }

    lcd.setCursor(0, 3);

    if (lowWater()) {
        pumpOff();
        digitalWrite(LED, HIGH);
        delay(WAIT_AFTER_LOW_WATER);
        digitalWrite(LED, LOW);
        lcd.print("Water supply: LOW   ");
        Serial.println("Water supply: LOW   ");
    } else if (level != MOISTURE_DRY) {
        pumpOff();
        lcd.print("Pump is off. Moist.");
        //Serial.println("Pump is off. Moist.");
        prevTimePumpDelay = millis();
    } else {
        if (diff > PUMP_ON_TIME && diff < FILLING_INTERVAL) {
            pumpOff();
            lcd.print("Pump is off         ");
        } else if (diff > FILLING_INTERVAL && millis() - prevTimePumpDelay > WAIT_AFER_MOIST && level == MOISTURE_DRY) {
            prevTimePumpOn = millis();
            prevTimePumpDelay = -1;
            pumpOn();
            lcd.print("Pump is ON!!!       ");
            Serial.println("Pump is on");
        }
    }

    delay(100);
}

void pumpOn() {
    digitalWrite(PUMP, LOW);
}

void pumpOff() {
    digitalWrite(PUMP, HIGH);
}

boolean lowWater() {
    return digitalRead(FLOAT_SWITCH) == LOW;
}

