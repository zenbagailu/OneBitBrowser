
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

#include "UdpSocket.h"
#include <Adafruit_CC3000.h>
//#include <utility/socket.h>
//#include <string.h> //for memset

namespace mdns {
	UdpSocket::UdpSocket(sockaddr const& address):socketHandle_{-1},address_(address){
		// Create the UDP socket
		socketHandle_= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if(socketHandle_<0){
			return;
		}

		if (bind(socketHandle_, &address_, sizeof(address_)) < 0) {
	    	closesocket(socketHandle_);
	    	socketHandle_=-1; //invalidate
	    	return;
	    } 
	}

	UdpSocket::~UdpSocket(){
		if(isValid()){ 
			closesocket(socketHandle_);	//it actually returns 0 if success, -1 if it fails
		}
	}

	uint32_t UdpSocket::isValid(){
		return socketHandle_>=0;
	}

}