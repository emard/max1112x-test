/*
*/

// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin
const int pin = 32+20; // first audio pin

const int SPI_MISO = 20;
const int SPI_MOSI = 8+18;
const int SPI_CLK = 8+17;
const int SPI_CSN = 8+16;

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

const uint16_t config_setup =
          (0x10 << 11) |  // CONFIG_SETUP
          (1 << 10)    |  // REFSEL external single ended (vs REF-).
          (0 << 9)     |  // AVGON averaging turned off.
          (0 << 7)     |  // NAVG 1 conversion per result.
          (0 << 5)     |  // NSCAN scans N and returns 4 results.
          (0 << 3)     |  // SPM all circuitry powered all the time.
          (1 << 2)     ;  // ECHO 1-enabled 0-disabled.

const uint16_t mode_control =
          (0 << 15) |  // REG_CNTL select ADC mode control.
          (3 << 11) |  // standard_int scan 0-N internal clock w/avg.
          (7 << 7)  |  // CHSEL scan up to channel 7.
          (0 << 5)  |  // RESET the FIFO only.
          (0 << 3)  |  // PM power management normal.
          (0 << 2)  |  // CHAN_ID set to 1 in ext mode to get chan id.
          (1 << 1)  ;  // SWCNV 1 (rising edge of CS)

const uint16_t sample_set =
          (B10110 << 11 ) |  // SMPL_SET select ADC sample set
               (4 << 3)   ;  // SEQ length

uint16_t SPIRW(uint16_t d)
{
  uint16_t r = 0, b = 1<<15;
  uint8_t miso;
  for(int i = 0; i < 16; i++)
  {
    digitalWrite(SPI_CLK, LOW);
    //digitalWrite(9, LOW);
    digitalWrite(SPI_MOSI, (d & b) != 0 ? HIGH : LOW);
    //digitalWrite(10, (d & b) != 0 ? HIGH : LOW);
    //delay(1);
    digitalWrite(SPI_CLK, HIGH);
    //digitalWrite(9, HIGH);
    // read now
    if(digitalRead(SPI_MISO))
    {
      r |= b;
      //digitalWrite(11, HIGH);
    }
    else
    {
      //digitalWrite(11, LOW);
    }
    b >>= 1;
    //delay(1);
  }
  return r;
}

uint16_t SPISend(const unsigned int adc_cbase, uint16_t d)
{
  digitalWrite(SPI_CSN, LOW);
  //digitalWrite(8, LOW);
  uint16_t retval = SPIRW(d);
  digitalWrite(SPI_CSN, HIGH);
  //digitalWrite(8, HIGH);
  return retval;
}

void SPIdriveCSN(int cs)
{
  digitalWrite(SPI_CSN, cs);
  //digitalWrite(8, cs);
}

void SPIstart()
{
  // first clock with CSN HI
  digitalWrite(SPI_CSN, HIGH); // make sure it's high
  //delay(1);
  digitalWrite(SPI_CLK, LOW);
  //delay(1);
  digitalWrite(SPI_CLK, HIGH);
  //delay(1);
  SPISend(0,mode_control); // to the rest 16 bit send normal mode control command
}

void configureMax1112x()
{
  const int adc_cbase = 0;
  // Config: Reset.
  SPISend(adc_cbase, (2 << 5)); // RESET reset all registers to defaults.

  // Config: ADC configuration.
  SPISend(adc_cbase, config_setup);

  // Config: Unipolar.
  SPISend(adc_cbase, (0x12 << 11));  // All channels unipolar.
  SPISend(adc_cbase, (0x11 << 11) |  // All channels unipolar.
                        (1 << 2));      // Reference all channels to REF-.

  // Config: ADC mode control.
  SPISend(adc_cbase, mode_control);

  // Defaults are good for:
  // Config: Bipolar.
  // Config: RANGE.
  // Config: Custom scan 0.
  // Config: Custom scan 1.
  // Config: Sample set.
}


void setup() {
  for(int j = 0; j < 8; j++)
  {
    pinMode(8+j, OUTPUT);
    digitalWrite(8+j, LOW);
  }
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);
  configureMax1112x();
}

void loop() {
  // here is where you'd put code that needs to be running all the time.

  // check to see if it's time to blink the LED; that is, if the difference
  // between the current time and last time you blinked the LED is bigger than
  // the interval at which you want to blink the LED.
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
    SPIstart();
    delay(1); // wait conversion
    for(int i = 0; i < 9; i++)
    {
      Serial.print(SPISend(0,0), HEX);
      Serial.print(" ");
    }
    Serial.println("<");
    // set the GPIO shared with ADC
    pinMode(32+14, OUTPUT);
    digitalWrite(32+14, ledState);
  }
}
