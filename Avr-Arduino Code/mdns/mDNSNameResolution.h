
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

#ifndef MDNSNAMERESOLUTION_H_
#define MDNSNAMERESOLUTION_H_ 
#include <stdint.h> //for uint8_t
#include <Arduino.h> //for debugging, using Serial
#include <basics/String.h>


namespace mdns {

	const uint8_t MaxNameSize=16; 
	uint32_t getIpFromHostName(basics::String const& hostName); //hostname needs to end with .local


	template<class ByteBuffer> 
	void 
	printHexBuffer(ByteBuffer const& buf){

		for(auto byte: buf){

			if(byte<= 0xF){
		    	Serial.print(F("0"));
			}

			Serial.print(byte, HEX);
		    Serial.print(' '); 
		}
	}


}
#endif