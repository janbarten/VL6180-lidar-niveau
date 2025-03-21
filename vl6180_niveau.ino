#include <Wire.h>              //Nodig voor protocol om met de lidar te praten
#include "Adafruit_VL6180X.h"  //bevat commando's om de data op te halen
#include "RTClib.h"            //Nodig voor een real time clock

Adafruit_VL6180X vl = Adafruit_VL6180X();  //De lidar krijgt zijn eigen naam
RTC_DS3231 rtc;                            //De klok krijgt zijn eigen naam

/*

De VL6188X metingen veschillen tot wel 5 mm bij een gelijkblijvende afstand. Door een lopend gemiddelde te berekenen
krijgen we een stabiele meting
*/
const int numReadings = 30;  // aantal metingen waarover het lopende gemiddelde wordt berekend. Evt hier aanpassen.
int readings[numReadings];   // the readings from the sensor
int readIndex = 0;           // the index of the current reading
int total = 0;               // the running total
int meting = 0;              // the average

//variabelen voor de klok
int uur = 0;      // variabele om de uren in op te slaan
int minuut = 0;   // variabele om de uren in op te slaan
int seconde = 0;  // variabele om de seconden in op te slaan

//variabelen voor de tijdstippen waarop water geven moet worden:
int uur1 = 9;         // water geven om 9.00 uur
int uur2 = 16;        // water geven om 16.00 uur
int startMinuut = 0;  //variabele om de het sluitmoment van de vulkraan op te slaanzodat je de bewewateringsduur kunt timen

// variabelen voor het watergeef regime
int regime = 0;  // variabele om het watergeven op te slaan (waterhoogte en standtijd)
int stand = 0;
int duur = 0;
int hoog = 10;  // waterhoogte in millimeter
int midden = 7;
int laag = 5;
int lang = 10;  // standtijd van het water in minuten
int medium = 7;
int kort = 5;
bool vulStatus = 0;       // boolean om de vulstatus van het watergeven aan te geven 1 = vullen is actief
bool leegloopStatus = 0;  // boolean om de leegloopstatus van het watergeven aan te geven 1 = leeglopen is actief
bool natStatus = 0;       // boolean om vocht status in de tray op te slaan 0 = geen water 1 = gevuld met water



void setup() {
  Serial.begin(9600);
  rtc.begin();

  while (!Serial) {  // wait for serial port to open on native usb devices
    delay(1);
  }

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  Serial.println("Adafruit VL6180x test!");
  if (!vl.begin()) {
    Serial.println("Failed to find sensor");
    while (1)
      ;
  }
  Serial.println("Sensor found!");
  vl.setOffset(1);  // correctie voor misaanwijzing
}

void loop() {
  DateTime now = rtc.now();  // lezez van de RTC
  uur = now.hour();          // uur opslaan in de variabele
  minuut = now.minute();     // minuut opslaan in variabele

  uint8_t range = vl.readRange();         // afstand meten
  uint8_t status = vl.readRangeStatus();  // Checken op foutmeldingen van de lidar

  total = total - readings[readIndex];  // subtract the last reading
  readings[readIndex] = range;          // read from the sensor
  total = total + readings[readIndex];  // add the reading to the total
  readIndex = readIndex + 1;            // advance to the next position in the array
  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  meting = total / numReadings;  // calculate the average

  if (status == VL6180X_ERROR_NONE) {  // check for errors in the lidar
    Serial.print(meting);              // print to the computer serial monitor
    Serial.println(" mm");
  }

  // If some error occurred, print it out!
  if ((status >= VL6180X_ERROR_SYSERR_1) && (status <= VL6180X_ERROR_SYSERR_5)) {
    Serial.println("System error");
  } else if (status == VL6180X_ERROR_ECEFAIL) {
    Serial.println("ECE failure");
  } else if (status == VL6180X_ERROR_NOCONVERGE) {
    Serial.println("No convergence");
  } else if (status == VL6180X_ERROR_RANGEIGNORE) {
    Serial.println("Ignoring range");
  } else if (status == VL6180X_ERROR_SNR) {
    Serial.println("Signal/Noise error");
  } else if (status == VL6180X_ERROR_RAWUFLOW) {
    Serial.println("Raw reading underflow");
  } else if (status == VL6180X_ERROR_RAWOFLOW) {
    Serial.println("Raw reading overflow");
  } else if (status == VL6180X_ERROR_RANGEUFLOW) {
    Serial.println("Range reading underflow");
  } else if (status == VL6180X_ERROR_RANGEOFLOW) {
    Serial.println("Range reading overflow");
  }

  switch (regime) {  // definieren van de verschillende regimes
    case 1:
      stand = hoog;
      duur = lang;
    case 2:
      stand = hoog;
      duur = medium;
    case 3:
      stand = hoog;
      duur = kort;
    case 4:
      stand = midden;
      duur = lang;
    case 5:
      stand = midden;
      duur = medium;
    case 6:
      stand = midden;
      duur = kort;
  }

  if (uur == uur1 && stand <= meting && vulStatus == 0 && natStatus == 0) {  // controleren of het tijd is om te vullen
    vulStatus = 1;                                         // declaren dat we gaan vullen
  }

  if (vulStatus == 1) {
    //vullen aan                                            // code om het vullen te start
  }

  if (vulStatus == 1 && stand >= meting) {                  // controleren of het gewenste niveau bereikt is
    vulStatus = 0;                                          // declareren dat het vullen is gestupt
    // vullen uit                                           // hier de code om het vullen te stoppen
    natStatus = 1;                                          // declareren dat de bak vol water zit
    startMinuut = minuut;                                   // timer starten om de bewateringsduur te kunnen timen
  }

  if (natStatus == 1 && (minuut - startMinuut >= duur)) {   // controleren of de gewenste "natte" periode is bereikt
    // leegloop aan                                         // hier de code om de leegloop te starten
    natStatus = 0;                                          // declareren dat de bak leeg is
  }

  delay(20);
}
