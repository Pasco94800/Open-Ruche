/**
 * Exemple de code pour lire plusieurs capteurs DS18B20 sur un même bus 1-Wire via leur adresse unique.
 */
 
/* Dépendance pour le bus 1-Wire */
#include <OneWire.h>
 
 
/* Broche du bus 1-Wire */
const byte BROCHE_ONEWIRE = 5;

/* Adresses des capteurs de température */
const byte SENSOR_ADDRESS_1[] = {  0x28, 0x46, 0xC8, 0x31, 0xC, 0x0, 0x0, 0xB9 };
const byte SENSOR_ADDRESS_2[] = { 0x28, 0x46, 0xC8, 0x31, 0xC, 0x0, 0x0, 0xB9 };
const byte SENSOR_ADDRESS_3[] = { 0x28, 0x75, 0x91, 0x16, 0xA8, 0x1, 0x3C, 0x2C };


/* Création de l'objet OneWire pour manipuler le bus 1-Wire */
OneWire ds(BROCHE_ONEWIRE);

 
/**
 * Fonction de lecture de la température via un capteur DS18B20.
 */
float getTemperature(const byte addr[]) {
  byte data[9];
  // data[] : Données lues depuis le scratchpad
  // addr[] : Adresse du module 1-Wire détecté
  
  /* Reset le bus 1-Wire et sélectionne le capteur */
  ds.reset();
  ds.select(addr);
  
  /* Lance une prise de mesure de température et attend la fin de la mesure */
  ds.write(0x44, 1);
  delay(800);
  
  /* Reset le bus 1-Wire, sélectionne le capteur et envoie une demande de lecture du scratchpad */
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);
 
 /* Lecture du scratchpad */
  for (byte i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
   
  /* Calcul de la température en degré Celsius */
  return (int16_t) ((data[1] << 8) | data[0]) * 0.0625; 
}
 
 
/** Fonction setup() **/
void setup() {

  /* Initialisation du port série */
  Serial.begin(9600);
}
 
 
/** Fonction loop() **/
void loop() {
  float temperature[3];
   
  /* Lit les températures des trois capteurs */
  temperature[0] = getTemperature(SENSOR_ADDRESS_1);
  temperature[1] = getTemperature(SENSOR_ADDRESS_2);
  temperature[2] = getTemperature(SENSOR_ADDRESS_3);
  
  /* Affiche les températures */
  Serial.print(F("\n------------------------------------\n"));
  Serial.print(F("Temperatures internes : \n"));
  Serial.print(F("Temperature du capteur 1 : "));
  Serial.print(temperature[0], 2);
  Serial.print(F("° C, "));
  Serial.print(F("\nTemperature du capteur 2 : "));
  Serial.print(temperature[1], 2);
  Serial.print(F("° C, "));
  Serial.print(F("\nTemperature du capteur 3 : "));
  Serial.print(temperature[2], 2);
  Serial.println("° C");
  Serial.print(F("------------------------------------\n"));
}
