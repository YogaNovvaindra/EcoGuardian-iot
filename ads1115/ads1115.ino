#include <Adafruit_ADS1X15.h>
#include "DHTesp.h"

DHTesp dht;
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
//Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */
const float MQ2_RL = 10.0;   // Load resistance in ohms
const float MQ2_R0 = 10.0;   // Sensor resistance at clean air in ohms

// Define the constants for calibration (MQ2_LPG, MQ2_CO, MQ2_SMOKE)
#define MQ2_LPG 0.42 // LPG curve slope
#define MQ2_CO 0.77 // CO curve slope
#define MQ2_SMOKE 0.42 // Smoke curve slope

#define MQ7_RL 10.0 // Load resistance in ohms
#define MQ7_R0_CLEAN_AIR 20.0 // Sensor resistance at clean air for CO in kohms
#define MQ7_CO 1.0 // CO curve slope

// Define the constants for MQ-135 sensor calibration
#define MQ135_RL 10.0 // Load resistance in ohms
#define MQ135_R0_CLEAN_AIR 76.63 // Sensor resistance at clean air for CO2 in kohms
#define MQ135_ALPHA 0.99 // Steinhart-Hart alpha value for CO2

// Define the constants for MQ-135 CO2, CO, and NH4 calibration
#define MQ135_CO2 0.358 // CO2 curve slope
#define MQ135_CO 1.178 // CO curve slope
#define MQ135_NH4 0.47 // NH4 curve slope

void setup(void)
{
  Serial.begin(9600);

  dht.setup(16, DHTesp::DHT11); //DHT Sensor
  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
 
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  if (!ads.begin())
  {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
}
 
void loop(void)
{
  int16_t adc0, adc1, adc2, adc3;
  float volts0, volts1, volts2, volts3;
 
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);
 
  volts0 = ads.computeVolts(adc0);
  volts1 = ads.computeVolts(adc1);
  volts2 = ads.computeVolts(adc2);
  volts3 = ads.computeVolts(adc3);
 
  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0); Serial.println("V");
  Serial.print("AIN1: "); Serial.print(adc1); Serial.print("  "); Serial.print(volts1); Serial.println("V");
  Serial.print("AIN2: "); Serial.print(adc2); Serial.print("  "); Serial.print(volts2); Serial.println("V");
  // Serial.print("AIN3: "); Serial.print(adc3); Serial.print("  "); Serial.print(volts3); Serial.println("V");

  // Calculate resistance of the MQ-2 sensor
  float mq2_sensorResistance = (MQ2_RL * volts0) / (5.0 - volts0);

  // Calculate ppm values for MQ-2 gases using calibration formulas
  float mq2_lpg = pow(10, ((log10(mq2_sensorResistance) - log10(MQ2_LPG)) / (-MQ2_LPG)) + log10(MQ2_LPG));
  float mq2_co = pow(10, ((log10(mq2_sensorResistance) - log10(MQ2_CO)) / (-MQ2_CO)) + log10(MQ2_CO));
  float mq2_smoke = pow(10, ((log10(mq2_sensorResistance) - log10(MQ2_SMOKE)) / (-MQ2_SMOKE)) + log10(MQ2_SMOKE));

  // Calculate resistance of the MQ-7 sensor
  float mq7_sensorResistance = (MQ7_RL * volts1) / (5.0 - volts1);

  // Calculate CO ppm value for MQ-7 sensor using calibration formula
  float mq7_co = pow(10, ((log10(mq7_sensorResistance) - log10(MQ7_CO)) / (-MQ7_CO)) + log10(MQ7_CO));

  // Calculate resistance of the MQ-135 sensor
  float mq135_sensorResistance = (MQ135_RL * volts2) / (5.0 - volts2);

  float mq135_co2 = pow(10, ((log10(mq135_sensorResistance) - log10(MQ135_CO2)) / (-MQ135_CO2)) + log10(MQ135_CO2));
  float mq135_co = pow(10, ((log10(mq135_sensorResistance) - log10(MQ135_CO)) / (-MQ135_CO)) + log10(MQ135_CO));
  float mq135_nh4 = pow(10, ((log10(mq135_sensorResistance) - log10(MQ135_NH4)) / (-MQ135_NH4)) + log10(MQ135_NH4));

  Serial.println("-----------------------------------------------------------");
  Serial.print("MQ-2 LPG PPM: "); Serial.println(mq2_lpg);
  Serial.print("MQ-2 CO PPM: "); Serial.println(mq2_co);
  Serial.print("MQ-2 Smoke PPM: "); Serial.println(mq2_smoke);
  Serial.println("-----------------------------------------------------------");
  Serial.print("MQ-7 CO PPM: "); Serial.println(mq7_co);
  Serial.println("-----------------------------------------------------------");
  Serial.print("MQ-135 CO2 PPM: "); Serial.println(mq135_co2);
  Serial.print("MQ-135 CO PPM: "); Serial.println(mq135_co);
  Serial.print("MQ-135 NH4 PPM: "); Serial.println(mq135_nh4);
  Serial.println("-----------------------------------------------------------");

  //dht22
    float h = dht.getHumidity();
  float t = dht.getTemperature();
  Serial.print("{\" Kelembaban\" : ");
  Serial.print(h);
  Serial.print("%, \"Temperatur\" : ");
  Serial.print(t);
  Serial.print("°C }\n");
  //end dht
  delay(2000);
}
