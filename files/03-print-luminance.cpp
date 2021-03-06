#include "cookingUtils.h"
#include "cookingClasses.h"
#include "arduPi.h"

#define LUM_PIN 0

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

void loop() {
    float l = read_luminance(LUM_PIN);

    printf("***  Measures   ***\n");
    printf("Luminance : %f lux\n", l);
    delay(1000);
}

int main (){
   while(1){
      loop();
   }
   return (0);
}