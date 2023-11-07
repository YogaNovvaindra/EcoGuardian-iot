#include <Adafruit_ADS1X15.h>
#include "DHTesp.h"

DHTesp dht;
Adafruit_ADS1115 ads; /* Use this for the 16-bit version */

void setup(void)
{
    Serial.begin(9600);

    dht.setup(16, DHTesp::DHT11); // DHT Sensor
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
    // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
    // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
    // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
    // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
    if (!ads.begin())
    {
        Serial.println("Failed to initialize ADS.");
        while (1)
            ;
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

    int sensorValue = analogRead(A0);
    float voltage = sensorValue * (5.0 / 1023.0);

    Serial.println("-----------------------------------------------------------");
    Serial.print("analog: ");
    Serial.print(analogRead(A0));
    Serial.print("  ");
    Serial.print(voltage);
    Serial.println("V");
    Serial.print("AIN0: ");
    Serial.print(adc0);
    Serial.print("  ");
    Serial.print(volts0);
    Serial.println("V");
    Serial.print("AIN1: ");
    Serial.print(adc1);
    Serial.print("  ");
    Serial.print(volts1);
    Serial.println("V");
    Serial.print("AIN2: ");
    Serial.print(adc2);
    Serial.print("  ");
    Serial.print(volts2);
    Serial.println("V");
    Serial.print("AIN3: ");
    Serial.print(adc3);
    Serial.print("  ");
    Serial.print(volts3);
    Serial.println("V");

    // adc0 is set to be the MQ2 sensor
    // calculate the resistance of the MQ2 sensor from volts0
    float sensor_volt = volts0;
    float RS_gas = (5.0 - sensor_volt) / sensor_volt; // omit *RL
    Serial.print("RS_gas: ");
    Serial.println(RS_gas);
    // calculate the ratio RS_gas/RS_air
    // RS/R0 = 9.8 for MQ2 gas sensor.
    float ratio = RS_gas / 9.8; // ratio = RS/R0
    Serial.print("ratio: ");
    Serial.println(ratio);
    // calculate smoke concentration in ppm
    // using spec sheet curve
    float slope_m = log(0.6/3.5) / log(10000/200);
    float slope_b = log(0.93) - (slope_m)*log(5000);
    // float ppm_x = 10 ^ {[log(y) - slope_b] / slope_m};
    // calculate ppm
    float ppm = pow(10, ((log(ratio) - slope_b) / slope_m));

    Serial.print("MQ2: ");
    Serial.println(ppm);
}
