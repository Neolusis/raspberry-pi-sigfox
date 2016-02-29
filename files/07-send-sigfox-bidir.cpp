#include "cookingClasses.h"
#include "cookingSigfox.h"
#include "cookingUART.h"
#include "cookingUtils.h"
#include "arduPi.h"
#include "SeeedDHT.h"

//////////////////////////////////////////////
//Asign to UART0
//////////////////////////////////////////////
uint8_t sock = SOCKET0;

#define DHT_PIN 8;
#define DHT_TYPE 22;
#define DHT_COUNT 16;
DHT dht(DHT_PIN, DHT_TYPE, DHT_COUNT);

#define BUTTON_PIN = 7;
// Button
int buttonState = 0;

#define LUM_PIN 0

#define RED_LED_PIN 2
#define GREEN_LED_PIN 3
#define BLUE_LED_PIN 4

// LED
#define RED_LED 1
#define GREEN_LED (1 << 1)
#define BLUE_LED (1 << 2)
#define LED_IS_ON(led_id,state) (state & (1 << (led_id-1)))

int ledsPin[3];

// Sigfox
uint8_t dataSigfox[12];
uint8_t size;
union
{
  uint8_t  value1[4];
  float    value2;
}t_union;
union
{
  uint8_t  value1[2];
  uint16_t value2;
}l_union;
uint8_t status;

// Luminance
float Vout[] = {0.0011498, 0.0033908, 0.011498, 0.15199, 0.53367, 1.3689, 1.9068, 2.3};
float Lux[] = {1.0108, 3.1201, 9.8051, 27.43, 69.545, 232.67, 645.11, 73.52, 1000};


float read_V_out(uint8_t apin) {
  float MeasuredVout = analogRead(0) * (3.0 / 1023.0);
  return MeasuredVout ;
}

float FmultiMap(float val, float * _in, float * _out, uint8_t size) {
  if (val <= _in[0]) return _out[0];
  if (val >= _in[size-1]) return _out[size-1];
  uint8_t pos = 1;
  while (val > _in[pos]) pos++;
  if (val == _in[pos]) return _out[pos];
  return (val - _in[pos-1]) * (_out[pos] - _out[pos-1]) / (_in[pos] - _in[pos-1]) + _out[pos-1];
}

float read_luminance(uint8_t apin) {
  float MeasureVout = analogRead(0) * (3.0 /1023.0);
  float Luminance = FmultiMap(MeasureVout, Vout, Lux, 9);
  return Luminance;
}

void setup() {
	pinMode(BUTTON_PIN, INPUT);
    pinMode(LUM_PIN, INPUT);
    pinMode(DHT_PIN, INPUT);

    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);

    ledsPin[RED_LED] = RED_LED_PIN;
    ledsPin[GREEN_LED] = GREEN_LED_PIN;
    ledsPin[BLUE_LED_PIN] = BLUE_LED_PIN;

    printf("*** Waiting for user action (push button) ***\n");
}

void loop()
{
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    float h = dht.TemperatureHumidityRead(DHT_PIN, 'H');
    float t = dht.TemperatureHumidityRead(DHT_PIN, 'T');
    float l = readLuminance(LUM_PIN);

    printf("***  Measures   ***\n");
    printf("Luminance : %f\n", l);
    printf("Humidify : %f\n", h);
    printf("Temperature : %f\n", t);
    printf("Send to Sigfox\n");

    // fill data array
    t_union.value2 = t;
    dataSigfox[0] = t_union.value1[3];
    dataSigfox[1] = t_union.value1[2];
    dataSigfox[2] = t_union.value1[1];
    dataSigfox[3] = t_union.value1[0];

    dataSigfox[4] = (uint8_t) h;

    l_union.value2 = (uint16_t) l;
    dataSigfox[5] = l_union.value1[1];
    dataSigfox[6] = l_union.value1[0];
    size = 7;

    // Final Frame to send in "data"
    printf("Final Frame to send: 0x%X\n", dataSigfox);

    // Sending packet to Sigfox
    status = Sigfox.send(dataSigfox,size);

    // Check sending status
    if( status == 0 )
    {
      printf("Sigfox packet sent OK\n");

      // TODO Check DEBUG
      int led_state = (int)strtol(reinterpret_cast<const char*>(Sigfox._ackData), NULL, 16);

      int i = 0;
      for(;i < 24; i++) {
        printf("%c", Sigfox._ackData[i]);
      }

      printf("\nLed state : %d", led_state);

      i = 0;
      for(;i <= 3; i++) {
        if(LED_IS_ON(i, led_state)) {
          printf("led %d is on\n", i);
          digitalWrite(ledsPin[i], HIGH);
        } else {
          printf("%d pin is off\n", ledsPin[i]);
          digitalWrite(ledsPin[i], LOW);
          }
      }

      printf("Back-End response: 0x%X\n", Sigfox._buffer);
    }
    printf("/// End of action ///\n");
    delay(1000);
  }
}

int main (){
	setup();
	while(1){
		loop();
	}
	return (0);
}
