/*
Copyright 2015 Pablo Miranda Carranza
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and limitations under the License.
*/

#include <Arduino.h>

#include <Arduino.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>

#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

//#include <mdns/mDNSNameResolution.h> //can be used if one wants mdns resolution
#include <basics/Buffer.h> //this code is included in the Avr-ArduinoActuatorCode folder
#include <basics/String.h>

//#define MAINDEBUG


bool pirSample(uint8_t pin){
  uint16_t sampleSize=8000;
  uint16_t ct=0;
  for(uint16_t i=0;i<sampleSize;++i){
    ct+=digitalRead(pin)? 0: 1;
  }
  return ct==sampleSize;
}

Adafruit_CC3000_Client connectToServer(Adafruit_CC3000& cc3000, uint32_t ip, uint16_t port){

#ifdef MAINDEBUG
    Serial.println(F("Connecting to server...")); //output the message
#endif

#ifdef MAINDEBUG
//Do a quick ping test
  Serial.print(F("\n\rPinging ")); 
  cc3000.printIPdotsRev(ip); 
  Serial.print("...");  
  uint8_t replies = cc3000.ping(ip, 5);

  Serial.print(replies); Serial.println(F(" replies"));
  if (replies)
    Serial.println(F("Ping successful!"));
#endif


  Adafruit_CC3000_Client tcpClient = cc3000.connectTCP(ip, port);
  basics::String messageType{"sensorcr"};
  if(static_cast<size_t>(tcpClient.write(messageType.data(),messageType.length()))!= messageType.length()){ //sings are not important
#ifdef MAINDEBUG
    Serial.println(F("Could not reach server.")); //output the message
#endif

  }else{
#ifdef MAINDEBUG
    Serial.println(F("message sent to server.")); //output the message
#endif

  }
  
  delay(1000); //wait for response

  static basics::Buffer<char,20> ibuf; //USED AS THE MAIN IO BUFFER

  ibuf.clear();
  while (tcpClient.available()){
    ibuf.push(tcpClient.read());
  }
  if(ibuf.size()){
    ibuf.push(0);//end of string...needs to be added
#ifdef MAINDEBUG
    Serial.println(ibuf.begin()); //output the message
#endif
  }else{
#ifdef MAINDEBUG
    Serial.println(F("Failed handshake with server.")); //output the message
#endif

  }

  return tcpClient;
}

void connetWireless(Adafruit_CC3000& cc3000){

 uint32_t t = millis();
  //Attempt to connect to an access point...
  while(!cc3000.connectSecure("YOUR WIFI NETWORK NAME","THE PASSWORD TO YOUR WIFI", WLAN_SEC_WPA2) && (millis()-t)<30000);  //30 sec timeout


  // Wait for DHCP to complete
  t = millis();
  while (!cc3000.checkDHCP() && (millis()-t)<30000);  //30 sec timeout

  if(!cc3000.checkDHCP()){

#ifdef MAINDEBUG
    Serial.println(F("DHCP request failed"));
#endif

  }

  if(!cc3000.checkConnected()){

#ifdef MAINDEBUG
    Serial.println(F("Could not connect to WIFI network"));
#endif

  }else{

#ifdef MAINDEBUG
    Serial.println(F("Connected to WIFI network"));
#endif

  }
}


int main()
{
  init();


  //Serial Init
  //---------------------------------------------------------------------------
#ifdef MAINDEBUG
  Serial.begin(115200);
  Serial.println(F("Initialising the CC3000."));
#endif
  //---------------------------------------------------------------------------

  //Pir sensor init:
  pinMode(2,INPUT);

  //LED For Test
  pinMode(4,OUTPUT);
  digitalWrite(4, 0);


  //Wireless init
  //---------------------------------------------------------------------------
  //IRQ 3, VBAT 5, CS  10 IRQ needs to be an interrupt pin. VBAT and IRQ can be any pins
  Adafruit_CC3000 cc3000 = Adafruit_CC3000(10, 3, 5, SPI_CLOCK_DIVIDER); // you can change this clock speed but DI

  if (!cc3000.begin()){

#ifdef MAINDEBUG
    Serial.println(F("Unable to initialise the CC3000. Check your wiring?"));
#endif

    while(true);
  }

  //This voodo seems to be necessary to make it work
  //with the current setup
  // cc3000.startSSIDscan();
  // cc3000.stopSSIDscan();
  //this also works...
  cc3000.scanSSIDs(10000);// the longer, the safer, it seems...
  cc3000.scanSSIDs(0);

  if (!cc3000.deleteProfiles()) {

#ifdef MAINDEBUG
    Serial.println(F("Failed deleting profiles."));
#endif

    while(true);
  }

    //IMPORTANT!!!
  //For disabling the timeout for sockets... as in:
  //http://e2e.ti.com/support/wireless_connectivity/f/851/t/290432.aspx
  unsigned long aucDHCP       = 14400;
  unsigned long aucARP        = 3600;
  unsigned long aucKeepalive  = 10;
  unsigned long aucInactivity = 0;

  netapp_timeout_values(&aucDHCP, &aucARP, &aucKeepalive, &aucInactivity);

  connetWireless(cc3000);


  //cc3000.getHostByName(WEBSITE, &ip); //if we would use an http address instead of mDNS...

   // using mdns
  // uint32_t ip;
  // t = millis();
  // do{
  //   ip=mdns::getIpFromHostName("my-macbook");
  // }while(ip==0 && (millis()-t)<30000);

  //Hard coded IP
  uint32_t  ip=cc3000.IP2U32(242, 177, 191, 201);
  uint16_t port = 8080;
  Adafruit_CC3000_Server server(port); //for listening to connections
  Adafruit_CC3000_Client tcpClient = connectToServer(cc3000,ip,port);


  //Now listen to sensor and emit events
  //---------------------------------------------------------------------------

  
  uint16_t ct=0;
  const uint16_t movementLim=50;
  //For re-connection. This prevents errors produced in the socket communication,
  //And also makes unnecesary to reconnect manually if the server is restarted,
  //which may be hard if the arduino is in a remote place
  const uint32_t reconnectInterval=15L*60L*1000L; //every 15 minutes
  //const uint32_t reconnectInterval=60L*1000L; //for testing every 1 minute
  uint32_t t = millis();

#ifdef MAINDEBUG
  basics::Buffer<char,20> ibuf; 
  uint32_t intervalCt=0;
#endif


  for(;;){

     ct+=pirSample(2)? 10: 0; //add 10 if movement detected
     ct=ct>=7?ct-7:0; //decrease every time...

     if(ct>movementLim){
  
      if(tcpClient.write("activity",8)!=8){ //-1 error, otherwise it should return the number of bytes transmitted
        //reconnect:

#ifdef MAINDEBUG
        Serial.println(F("Error transmitting. Resetting the tcpclient."));
#endif

        tcpClient.close();
        tcpClient = connectToServer(cc3000,ip,port);
      }

      //delay(100);  //wait a bit... (there are some problems otherwise)
#ifdef MAINDEBUG
      Serial.print(F("*"));
#endif

    }

     if(millis()-t > reconnectInterval){ //if using mdns it may be necessary to get the IP again...

#ifdef MAINDEBUG
       Serial.println();
       Serial.print(F("interval: "));
       Serial.print(intervalCt++);
       Serial.println();
#endif

       if(!cc3000.checkConnected()){

#ifdef MAINDEBUG
        Serial.println(F("WIFI disconnected! Trying to reconnect"));
#endif
        digitalWrite(4, 1); //set the led on...
        connetWireless(cc3000);
      }

      t=millis();
    }


#ifdef MAINDEBUG
    ibuf.clear();
    while (tcpClient.available()){
      ibuf.push(tcpClient.read());
    }
    if(ibuf.size()){
      Serial.println(F("Message received:"));
        ibuf.push(0);//end of string...needs to be added
        Serial.println(ibuf.begin()); //output the message
      }
#endif



    }
  //---------------------------------------------------------------------------

    return 0;
  }




