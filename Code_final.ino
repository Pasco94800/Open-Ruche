#include <SigFox.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <HX711.h>
#include <OneWire.h>

#define LOADCELL_DOUT_PIN  2 // pour le capteur de poids
#define LOADCELL_SCK_PIN  3 // pour le capteur de poids
#define DHTPIN 4     // Pin digital connecte au DHT sensor
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
int DEBUG = 1; //0 pour debug et 1 pour autonomie
const byte BROCHE_ONEWIRE = 5; /* Broche du bus 1-Wire */
int analogPin = A3; // potentiometer wiper (middle terminal) connected to analog pin 3

/* Adresses des capteurs de température */
const byte SENSOR_ADDRESS_1[8] = {  0x28, 0x46, 0xC8, 0x31, 0xC, 0x0, 0x0, 0xB9 };
const byte SENSOR_ADDRESS_2[8] = { 0x28, 0x46, 0xC8, 0x31, 0xC, 0x0, 0x0, 0xB9 };
const byte SENSOR_ADDRESS_3[8] = { 0x28, 0x75, 0x91, 0x16, 0xA8, 0x1, 0x3C, 0x2C };

DHT dht(DHTPIN, DHTTYPE);
OneWire ds(BROCHE_ONEWIRE);
HX711 scale;



float calibration_factor = 21067; // pour 16,6 kg -> 21067 and for ~93,5 kg -> 21387, on a actuellement une marge de 300g
uint32_t delayMS;
float val = 0;  // variable to store the value read
float resultat= 0; // le resultat en volt
float pourcentage=0; //la charge de la batterie en pourcentage

//Struct to send values to Sigfox __attribute__ ((packed)) sigfox_message 
typedef struct {

  int8_t pourcentage; //la charge de la batterie en pourcentage
  int8_t tempDHT;
  int8_t Humidite;
  int16_t temperature1;
  int16_t temperature2;
  int16_t temperature3;
  int16_t poids;

} SigfoxMessage;

SigfoxMessage ValeursCapteurs;

void reboot() {
 NVIC_SystemReset();
 while (1);
}

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

void getCapteurDHT() {
  ValeursCapteurs.tempDHT = dht.readTemperature(); // temperature degrees
  ValeursCapteurs.Humidite = dht.readHumidity(); //humidite percent

  if (isnan(ValeursCapteurs.tempDHT)) {
    if(DEBUG == 0){
      Serial.println(F("Error reading temperature!"));
      }
      }
      
  if (isnan(ValeursCapteurs.Humidite)) {
    if(DEBUG == 0){
      Serial.println(F("Error reading humidity!"));
      }
    }

  if(DEBUG == 0){
  Serial.print(F("Temperature: "));
  Serial.print(ValeursCapteurs.tempDHT);
  Serial.println(F("°C"));
  Serial.print(F("Humidity: "));
  Serial.print(ValeursCapteurs.Humidite);
  Serial.println(F("%"));
  }
}

void getPoids(){
    //Get weight 
    scale.set_scale(calibration_factor);
    ValeursCapteurs.poids = (int16_t) (scale.get_units() * 10);

    if(DEBUG == 0){
    Serial.print("Reading: ");
    Serial.print(ValeursCapteurs.poids, 1);
    Serial.print(" kg ");
    Serial.print(" calibration_factor: ");
    Serial.print(calibration_factor);
    Serial.println();
  
    if(Serial.available())
    {
      char tamp = Serial.read();
      if(tamp == '+' || tamp == 'a')
        calibration_factor += 10;
      else if(tamp == '-' || tamp == 'z')
        calibration_factor -= 10;
    }
    }
  }

void getTempInterne(){
   
  /* Lit les températures des trois capteurs */
  ValeursCapteurs.temperature1 = (int16_t)(getTemperature(SENSOR_ADDRESS_1)*10);
  ValeursCapteurs.temperature2 = (int16_t)(getTemperature(SENSOR_ADDRESS_2)*10);
  ValeursCapteurs.temperature3 = (int16_t)(getTemperature(SENSOR_ADDRESS_3)*10);


  if(DEBUG == 0){
      /* Affiche les températures */
      Serial.print(F("\n------------------------------------\n"));
      Serial.print(F("Temperatures internes : \n"));
      Serial.print(F("Temperature du capteur 1 : "));
      Serial.print(ValeursCapteurs.temperature1);
      Serial.print(F("° C, "));
      Serial.print(F("\nTemperature du capteur 2 : "));
      Serial.print(ValeursCapteurs.temperature2);
      Serial.print(F("° C, "));
      Serial.print(F("\nTemperature du capteur 3 : "));
      Serial.print(ValeursCapteurs.temperature3);
      Serial.println("° C");
      Serial.print(F("------------------------------------\n"));
  }

}

void getBaterie(){
  val = analogRead(analogPin);  // read the input pin
  ValeursCapteurs.pourcentage = map(val,764,1023,0,100);

  if(DEBUG == 0){
    Serial.println("Pourcentage batterie :");          // debug value
    Serial.println(ValeursCapteurs.pourcentage);
  }

}

void envoi(){ //protocole pour envoyer un message à Sigfox
 SigFox.begin();
 SigFox.beginPacket();
 SigFox.write((uint8_t*)&ValeursCapteurs, sizeof(SigfoxMessage));
   if(DEBUG == 0){
     Serial.print("Status: ");
     Serial.println(SigFox.endPacket());
   }
  SigFox.endPacket();
 SigFox.end();
}

void setup() {
  if(DEBUG == 0){
      Serial.begin(9600);
  }

  dht.begin();
    if(DEBUG == 0){
         while (!Serial);
    }

   if (!SigFox.begin()) {
       if(DEBUG == 0){
           Serial.println("SigFox error, rebooting");
       }

   reboot();
   }
   delay(100); // Wait at least 30ms after first configuration
   // Enable debug prints and LED indication
   SigFox.debug();

  //Set weight sensor
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0 195727
  

         if(DEBUG == 0){
            long zero_factor = scale.read_average(); //Get a baseline reading
            Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
            Serial.println(zero_factor);
         }

  pinMode(analogPin, INPUT); //For the battery, configuration of ADC

   SigFox.status();
}



void loop() {

  envoi();
  //Serial.print(F("------------------------------------------------------------------------------------------------------------\n"));
  getCapteurDHT();
  getPoids();
  getTempInterne();
  getBaterie();
  //Serial.print(F("------------------------------------------------------------------------------------------------------------\n"));
  delay(720000);
  }
