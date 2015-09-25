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
#include "stepperpolyhedron/Stepper.h"



#include <Arduino.h>
#include <Adafruit_CC3000.h>
#include <Adafruit_CC3000_Server.h>

#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

#include "mDNSNameResolution.h"
#include <basics/Buffer.h>

#define MAINDEBUG



Adafruit_CC3000_Client connectToServer(Adafruit_CC3000& cc3000, uint32_t ip, uint16_t port){

  Adafruit_CC3000_Client tcpClient = cc3000.connectTCP(ip, port);
  basics::String messageType{"motorcr"};
  tcpClient.write(messageType.data(),messageType.length());
  delay(1000); //wait for response

  basics::Buffer<char,20> ibuf; //USED AS THE MAIN IO BUFFER

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

uint32_t getIp(){ //here we can set either static or dynamic forms of getting ips...

  uint32_t ip=0;
  uint32_t t = millis();
  do{
    //ip=mdns::getIpFromHostName("my-macbook");
    ip=mdns::getIpFromHostName("rpi-name"); //Raspberry Py with 0 conf, with for example Avahi. 
  }while(ip==0 && (millis()-t)<60000);

  //Hard coded IP (Used with Open Stack Server, for example)
  //uint32_t  ip=cc3000.IP2U32(225, 155, 193, 208);

  return ip;
}


int main()
{
  init();

  //Serial Init
  //---------------------------------------------------------------------------
#ifdef MAINDEBUG
  Serial.begin(57600);
#endif
  //---------------------------------------------------------------------------


  //Stepper Init. This will also set the stepper down
  //---------------------------------------------------------------------------
  stepperpolyhedron::Stepper  stepper{7,8,9,6,35115}; //pins and max val. Speed interval could also be specified (default 100)
  //---------------------------------------------------------------------------


 //Wireless init
  //---------------------------------------------------------------------------
  //IRQ 3, VBAT 5, CS  10 IRQ needs to be an interrupt pin. VBAT and IRQ can be any pins

#ifdef MAINDEBUG
  Serial.println(F("Initialising the CC3000."));
#endif

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
  cc3000.scanSSIDs(10000);// a bit longer, safer, it seems...
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
  uint32_t ip=getIp();

  uint16_t port = 8080;
  Adafruit_CC3000_Server server(port); //for listening to connections
  Adafruit_CC3000_Client tcpClient = connectToServer(cc3000,ip,port);

  //---------------------------------------------------------------------------




  //Now wait and process command
  //---------------------------------------------------------------------------
  bool mainValue=false;
  bool moving=false;

  basics::Buffer<char,20> ibuf; 

  const uint32_t reconnectInterval=15L*60L*1000L; //every 15 minutes
  //const uint32_t reconnectInterval=20L*1000L; //for testing every 20 secs
  uint32_t t = millis();
  //t = millis();

#ifdef MAINDEBUG
  uint32_t intervalCt=0;
#endif


  for(;;){

    //MANAGE MESSAGES
    //---------------------------------------------------------------------------

    if(tcpClient.available()){
      ibuf.push(tcpClient.read());
    }
    else if(ibuf.size()){ //if tcpClient not available (message finished) but data in buffer, process it
      ibuf.push(0);//end of string...needs to be added
      // Serial.println(ibuf.begin()); //output the message
     //process the message
      if(ibuf.compare("up", 2)==2){

#ifdef MAINDEBUG
        Serial.println(F("Setting up."));
#endif
        stepper.moveUp();
      }

      else if(ibuf.compare("down", 4)==4){

#ifdef MAINDEBUG
        Serial.println(F("Setting down."));
#endif
        stepper.moveDown(); 
      }

      ibuf.clear(); //reset the message buffer

    } 
    //---------------------------------------------------------------------------


    //CHECK CONNECTION AT TIME INTERVALS
    //---------------------------------------------------------------------------

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

        //digitalWrite(4, 1); //set the led on...
        connetWireless(cc3000);
      }
      //there seems no other way of testing from here if the socket is still alive...
      if(tcpClient.write("test",4)!=4){ //-1 error, otherwise it should return the number of bytes transmitted
      //reconnect:
#ifdef MAINDEBUG
        Serial.println(F("Error transmitting. Resetting the tcpclient."));
#endif
        //Serial.println(F("Resetting the tcpclient."));
        ip=getIp();
        tcpClient.close();
        tcpClient = connectToServer(cc3000,ip,port);
      }


      t=millis();
    } 
    //end of interval connection check
    //---------------------------------------------------------------------------
    

  } //end of forever loop
  

  return 0;
}




