#include <HID-Project.h>


class Encoder {
  private:
    int pinA;
    int pinB;
    int pressDebounce;
    int releaseDebounce;
    int countA;
    int countB;
    int rawA;
    int rawB;
    uint8_t prevNextCode = 0;
    uint16_t store=0;

  public:
    boolean prevA = false;
    boolean prevB = false;
    boolean currA = false;
    boolean currB = false;

    Encoder(int _pinA, int _pinB) {
      pinA = _pinA;
      pinB = _pinB;
      pressDebounce = 3;
      releaseDebounce = 3;
    }

    void setup() {
      pinMode(pinA, INPUT_PULLUP);
      pinMode(pinB, INPUT_PULLUP);
      countA = 0;
      countB = 0;
    }

    void poll() {

      int current = read_rotary();

      if (current == 1){
        up();
      } else if (current == -1){
        down();
      }
      
      
//      prevA = currA;
//      prevB = currB;
//      
//      rawA = digitalRead(pinA);
//      rawB = digitalRead(pinB);
//      
//
//
//
//      if (prevA && !currA) {
//        if (!currB) {
//          up();
//        } else {
//          down();
//        }
//      }
//      else if (!prevA && currA) {
//        if (currB) {
//          up();
//        } else {
//          down();
//        }
//      }
      //    else if (prevB && !currB){
      //      if (!currA){
      //        up();
      //      }else{
      //        down();
      //      }
      //    }
      //    else if (!prevB && currB){
      //      if (currA){
      //        up();
      //      }else{
      //        down();
      //      }
      //    }
    }

    // A vald CW or  CCW move returns 1, invalid returns 0.
    int8_t read_rotary() {
      static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
    
      prevNextCode <<= 2;
      if (digitalRead(pinA)) prevNextCode |= 0x02;
      if (digitalRead(pinB)) prevNextCode |= 0x01;
      prevNextCode &= 0x0f;
    
       // If valid then store as 16 bit data.
       if  (rot_enc_table[prevNextCode] ) {
          store <<= 4;
          store |= prevNextCode;
          //if (store==0xd42b) return 1;
          //if (store==0xe817) return -1;
          if ((store&0xff)==0x2b) return -1;
          if ((store&0xff)==0x17) return 1;
       }
       return 0;
    }

    void up() {
      Consumer.write(MEDIA_VOL_UP);
    }
    void down() {
      Consumer.write(MEDIA_VOL_DOWN);
    }

};

class Keyswitch {
  private:
    int pin;
    int ledPin;
    char key;
    int pressDebounce;
    int releaseDebounce;
    int count;
    int LEDCount;
    int LEDPin;
    float LEDLevel = 0;
    float LEDFadeRate = 0.1;
    boolean LEDFade = true;

  public:
    boolean pressed;
    boolean pinState;
    boolean LEDActive;


    Keyswitch(int _pin, char _key, int _pressDebounce, int _releaseDebounce) {
      pin = _pin;
      key = _key;
      pressDebounce = _pressDebounce;
      releaseDebounce = _releaseDebounce;
      LEDActive = false;
    }

    Keyswitch(int _pin, char _key, int _pressDebounce, int _releaseDebounce, int _LEDPin) {
      pin = _pin;
      key = _key;
      pressDebounce = _pressDebounce;
      releaseDebounce = _releaseDebounce;
      LEDPin = _LEDPin;
      LEDActive = true;
      LEDLevel = 0;
    }

    void setup() {
      pinMode(pin, INPUT_PULLUP);

      if (LEDActive) {
        pinMode(LEDPin, OUTPUT);
      }
    }

    void poll() {
      pinState = digitalRead(pin); // LOW is pressed, HIGH is released

      if (!pressed) {
        if (!pinState) {
          count ++;
        } else {
          count = 0;
        }
        if (count > pressDebounce) {
          pressKey();
        }
      } else {
        if (!pinState) {
          count = 0;
        } else {
          count ++;
        }
        if (count > releaseDebounce) {
          releaseKey();
        }
      }


    }

    void LEDUpdate() {
      if (LEDActive) {
        if (!pressed) {
          LEDLevel -= LEDLevel * LEDFadeRate;
        }
        if (LEDLevel <= 1) {
          LEDLevel = 0;
          LEDFade = false;
        }
        analogWrite(LEDPin, round(LEDLevel));
      }
    }

    void pressKey() {
      if (!pressed) {
        Keyboard.press(key);
        pressed = true;
        LEDLevel = 255;
        LEDFade = false;
      }
    }
    void releaseKey() {
      if (pressed) {
        Keyboard.release(key);
        pressed = false;
        LEDFade = true;
      }
    }

}; // END KEYSWITCH CLASS

int pressDebounce = 0;  // number of consecutive polls
int releaseDebounce = 20; // number of consecutive polls

Keyswitch SW1(0, 'x', pressDebounce, releaseDebounce, 7); // switch pin, character, press debounce, release debounce, LED pin)
Keyswitch SW2(2, 'z', pressDebounce, releaseDebounce, 8);
Keyswitch SW3(1, 'c', pressDebounce, releaseDebounce, 9);
Keyswitch SW4(3, '`', pressDebounce, releaseDebounce, 10);

Encoder vol(5,4);

unsigned long currentTime = 0;

unsigned long prevTime = 0;
unsigned long pollingInterval = 1000; //Microseconds``zz`
unsigned long elapsedTime = 0;

unsigned long prevTimeLED = 0;
unsigned long pollingIntervalLED = 5000; //Microseconds
unsigned long elapsedTimeLED = 0;

void setup() {

  pinMode(PIN_LED2, INPUT);
  pinMode(PIN_LED3, INPUT);

  SW1.setup();
  SW2.setup();
  SW3.setup();
  SW4.setup();
  vol.setup();

  Keyboard.begin();
  Consumer.begin();
}

void loop() {
  // Button Poll
  currentTime = micros();
  elapsedTime = currentTime - prevTime;
  if (elapsedTime >= pollingInterval) {
    prevTime = currentTime - (elapsedTime - pollingInterval);
    SW1.poll();
    SW2.poll();
    SW3.poll();
    SW4.poll();
    vol.poll();
  }

  // LED Poll
  elapsedTimeLED = currentTime - prevTimeLED;
  if (elapsedTimeLED >= pollingIntervalLED) {
    prevTimeLED = currentTime - (elapsedTimeLED - pollingIntervalLED);
    SW1.LEDUpdate();
    SW2.LEDUpdate();
    SW3.LEDUpdate();
    SW4.LEDUpdate();
  }

}
