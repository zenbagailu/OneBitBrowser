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

#include "mDNSNameResolution.h"
#include "UdpSocket.h"
#include <utility/socket.h>
#include "basics/Buffer.h"


//#define NAMERESDEBUG 


namespace mdns {

	template<class BufferR, class BufferA> 
	uint32_t 
	processResponse(BufferR const& response, BufferA const& request){

		//For a reference of processing response: https://en.wikipedia.org/wiki/MDNS

		if(response[2]!=0x84){ //check response code
#ifdef NAMERESDEBUG 
			Serial.println(F("no response."));
#endif 
			return 0;
		}

		if(response[12]!=request[12]){
#ifdef NAMERESDEBUG 
			Serial.println(F("different size."));
#endif 
			return 0;
		}

		uint8_t pos=12+response[12]+8;  //name + 0x05 +"local" + 0x00 end byte
		for(uint8_t i=12;i<pos;++i){ 
			if(tolower(response[i])!=tolower(request[i])){ //to lower because the cases may not necessarily match
#ifdef NAMERESDEBUG 
				Serial.println(F("request and respond don't match."));
#endif 
				return 0;
			}
		}

		//IPv4 code and IPv4 class code
		if(response[pos++]!=0x00 || //IPv4 code
		   response[pos++]!=0x01 ||
		   response[pos++]!=0x80 || //IPv4 class code
		   response[pos++]!=0x01){
#ifdef NAMERESDEBUG 
		   Serial.println(F("no IPv4 code or IPv4 class code."));
#endif
		   	return 0;
		}

		pos+=4;

		//the IPv4 length (hex 00 04),
		if(response[pos++]!=0x00 || 
		   response[pos++]!=0x04 ){
#ifdef NAMERESDEBUG 
		   	Serial.println(F("no   IPv4 length."));
#endif
		   	return 0;
		}

		uint32_t ip = response[pos++];
		ip <<= 8;
		ip |= response[pos++];
		ip <<= 8;
		ip |= response[pos++];
		ip <<= 8;
		ip |= response[pos++];

		return ip;
	}



	// mDNS port - 5353    mDNS multicast address - 224.0.0.251 
	//it cannot be PROGMEM in pronciple (only supported types in avr/pgmspace.h)
	const sockaddr address ={AF_INET,0x14,0xe9,0xe0,0x00,0x00,0xfb}; 
	//static const sockaddr address ={AF_INET,0x14,0xe9,0xe0,0x00,0x00,0xfb}; 

	uint32_t getIpFromHostName(basics::String const& hostName){ //should not have .local!!!


		if(hostName.length()>MaxNameSize){

#ifdef NAMERESDEBUG 
			Serial.println(F("Hostname to get is too long."));
#endif 
			return 0; //they should return some error message (an enum type, for exapmple)
		}


		UdpSocket socket(address);

		if(!socket.isValid()){

#ifdef NAMERESDEBUG 
			Serial.println(F("socket is invalid."));
#endif
			return 0; //0 is in this case an invalid address
		}

		//send request packet, from https://en.wikipedia.org/wiki/Multicast_DNS
		const uint8_t requestHeader[]={	
			0x00, 0x00, //ID (00 00)
			0x00, 0x00, //FLAGS (00 00 for a request)
			0x00, 0x01, //QDCOUNT, Number of question items
			0x00, 0x00, //ANCOUNT
			0x00, 0x00, //NSCOUNT
			0x00, 0x00  //ARCOUNT
		}; 

		const uint8_t requestEnd[]={
			0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c, //0x05 (five characters) l,o,c,a,l
			0x00, 		//End of the string
			0x00, 0x01, //host address QTYPE flag
			0x00, 0x01  //internet QCLASS flag
			//0x80, 0x01  //internet QCLASS flag (Also works)
		}; 


		static const uint8_t hs=sizeof(requestHeader);
		static const uint8_t es=sizeof(requestEnd);

		//const uint8_t hNSize=static_cast<const uint8_t>(hostName.length());
		const size_t hNSize=hostName.length();

		basics::Buffer<uint8_t,hs+es+MaxNameSize+1> request;
	

		request.push(requestHeader, hs);
		request.push(hNSize);
		//type punning... this should be solved more elegantly
		request.push(reinterpret_cast<const uint8_t*>(hostName.data()), hostName.length()); 
		request.push(requestEnd, es);


		socket.sendData(request);

#ifdef NAMERESDEBUG 
		Serial.println(F("Sent packet: "));
		printHexBuffer(request);
		Serial.println();
#endif 

#if CC3000_RX_BUFFER_SIZE <171
#warning CC3000_RX_BUFFER_SIZE may be too small. Change it in in cc30000_common.
#endif 

		uint32_t ip=0;
		//For this size to work, CC3000_RX_BUFFER_SIZE needs to be modified (in cc30000_common.h)
		//171 seems to work...
		basics::Buffer<uint8_t,128> response; 
		uint32_t t = millis();
		uint32_t timeout=10000; //if it has not got an answer in 10 seconds, return
		do{
			response.clear();
			if(socket.receiveData(response)){

#ifdef NAMERESDEBUG 
				Serial.println(F("Response packet: "));
				printHexBuffer(response);
				Serial.println();
#endif 
			}
			
			ip=processResponse(response,request);

		//process response, do until it gets the right one...(Should have a timeout...)
		}while(ip==0 && (millis()-t)<timeout); 
		
		return ip;

	}

}