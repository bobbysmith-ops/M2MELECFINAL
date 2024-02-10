
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>

#include <Wire.h>
#include "Adafruit_VCNL4010.h"


//setting up the UUIDs for the service and characteristics
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define FSR_CHARACTERISTIC_UUID "b4a3a889-cf5f-40f9-a1f2-ac515643ba81"
#define CALFLAG_CHARACTERISTIC_UUID "608e2c81-0afe-4019-aeb6-a5af9d0fe141"


//calibration variables
//uint8_t *calFlag = 0;
//char calFlag = "b";
//String calFlag = "no";
uint8_t *calFlag = 0;
int fingerPress = 0;
//int maxPress = 0;
int maxPress = 1;
int fingerSpread = 0;
//int maxSpread = 1;
int maxSpread = 65535;


//Proximity Sensor Variables
int proxVal=0;
int proxVal1;
uint32_t proxVal2;
Adafruit_VCNL4010 vcnl; //declare proximitySensor object


//FSR (force sensitive resistor) sensor variables
const int FSR_PIN = 2;
int fsrADC=0;
int fsrADC1;
uint32_t fsrADC2;


//Flex Sensor Variables
const int FLEX_PIN = 15;//test
const float VCC = 4.98; // Measured voltage of Ardunio 5V line
const float R_DIV = 47500.0; // Measured resistance of 3.3k resistor
int flexADC=0;
int flexADC1;
uint32_t flexADC2;


//Extra test variables
uint32_t sensorVal = 0;
uint16_t testVal4 = 3;
uint8_t testVal2 = 14;
float testVal7 = 5.2;



//Creating the BLE characteristics (it's important to declare these w/ global scope, ie. outside setup or loop)
BLECharacteristic pCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);//Proximity sensor characteristic
BLECharacteristic FSRCharacteristic(FSR_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);//FSR characteristic
BLECharacteristic calFlagCharacteristic(CALFLAG_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);//calibration flag characteristic

  
//TODO: Possibly implement later, would require using the new arduino bluetooth library I think
//BLEFloatCharacteristic pCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
//BLEDescriptor
//BLEDescriptor pDescriptor(BLEUUID((uint16_t)0x2902));
  

void setup() {
  
  Serial.begin(115200);
  vcnl.begin();//call begin method from proximity sensor object
  Serial.println("Starting BLE work!");

  
  //create the BLE device
  BLEDevice::init("MyESP32");

  //create the BLE server
  BLEServer *pServer = BLEDevice::createServer();

  //create the BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  //adding the BLEcharacteristics I made to my BLEservice
  pService->addCharacteristic(&pCharacteristic); 
  pService->addCharacteristic(&FSRCharacteristic);
  pService->addCharacteristic(&calFlagCharacteristic);
  
  
  pService->start();//start the BLEservice
  

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising(); //Create advertising object for our BLE device
  
  pAdvertising->addServiceUUID(SERVICE_UUID); //Attach our Service UUID to the advertising object

  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  
  BLEDevice::startAdvertising();//Get our BLE device to start advertising
  
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // put your main code here, to run repeatedly:

  //calFlag = calFlagCharacteristic.getValue();
  calFlag = calFlagCharacteristic.getData();//the getData() method is returning a pointer to a uint8_t aka it returns uint8_t*
  //Serial.println(calFlag);
  uint8_t receivedCalFlag = *calFlag; //dereference the pointer and store in a new uint8_t variable
  //Serial.println(receivedCalFlag);
  Serial.print("Calibration flag value: ");
  //Serial.println(*calFlag); //can also just dereference and print in the same step
  Serial.println(receivedCalFlag);

  

  while (receivedCalFlag == 1) {
    fingerPress = analogRead(FSR_PIN);
    if (fingerPress > maxPress)
      maxPress = fingerPress;

    fingerSpread = vcnl.readProximity();
    if (fingerSpread < maxSpread)
      maxSpread = fingerSpread;
      
   delay(100);
   Serial.print("Current maxPress: "); Serial.println(maxPress);
   Serial.println(fingerPress);

   Serial.print("Current maxSpread: "); Serial.println(maxSpread);
   Serial.println(fingerSpread);
   
   Serial.print("Calibration Flag: "); Serial.println(receivedCalFlag);
   
   calFlag = calFlagCharacteristic.getData();
   receivedCalFlag = *calFlag;
  }

  Serial.print("maxPress: "); Serial.println(maxPress);
  Serial.print("maxSpread: "); Serial.println(maxSpread);
  

  //-----PROXIMITY SENSOR------
  
  proxVal = vcnl.readProximity();
  //proxVal1 = map(proxVal,2170,65535,0,100);
  proxVal1 = map(proxVal,maxSpread,65535,0,100);
  proxVal2 =  static_cast<uint32_t>(proxVal1);
  //Serial.println(proxVal1);
  //Serial.println(proxVal);
  Serial.println(proxVal2);

   
  //------FLEX SENSOR-----
   //flexADC = analogRead(FLEX_PIN);
   //rrflexADC1 = flexADC/20;
   //flexADC2 = static_cast<uint32_t>(flexADC1);
   //Serial.println(flexADC2);

  //------FSR------
   fsrADC = analogRead(FSR_PIN);
   //fsrADC1 = fsrADC/10;
   //fsrADC1 = map(fsrADC,0,3300,0,100);
   fsrADC1 = map(fsrADC,0,maxPress,0,100);
   fsrADC2 = static_cast<uint32_t>(fsrADC1);
   Serial.println(fsrADC2);


//sensorVal = 85; //random test variable

delay(100);
pCharacteristic.setValue(proxVal2);//Update the value for the characteristic with our sensor value

delay(100);
FSRCharacteristic.setValue(fsrADC2);//Update the value for the FSR characteristic with our sensor value


//TODO: Implement flex sensors in future
//delay(100);
//pCharacteristic.setValue(flexADC2);



}
