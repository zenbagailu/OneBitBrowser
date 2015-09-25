
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

//This is a wrapper for a socket for the wireless cc3000 library
//the idea is that mDNS name resolution can be used by re-implmenting this
//class for any other type of socket. It uses C++11 conventions, so it needs 
//a recent avr-gcc compiler.

#ifndef MDNSSOCKET_H_
#define MDNSSOCKET_H_ 

#include  <stdint.h> //for uint definitions
#include <utility/socket.h>
#include <utility/cc3000_common.h>


namespace mdns {

	class UdpSocket
	{
	public:
		UdpSocket(sockaddr const& address);
		virtual ~UdpSocket();
		UdpSocket( UdpSocket const&)=delete;
		UdpSocket& operator = (UdpSocket const&)=delete;

		uint32_t isValid();

		template<typename DataContainer>  
		bool sendData(DataContainer const& data);

		template<typename DataContainer>
		bool 
		receiveData(DataContainer& data);

	
	private:
		int32_t	socketHandle_;
		sockaddr const  address_;

	};

	template<typename DataContainer>
	bool UdpSocket::sendData(DataContainer const& data){

		//sendto is needed (rather than send) because the destination port
		//(given in the address) needs also to be set to the same as the 
		//from port (set in the address given to the socket)
		//This is also the recommendation for UDP Packages from TI:
		//http://processors.wiki.ti.com/index.php/CC3000_Host_Programming_Guide#CC3000_Socket_API
		size_t len=data.size()*DataContainer::ElemSize();
		return len == sendto(socketHandle_, data.begin(), len, 0, &address_, sizeof(address_));
	}

	template<typename DataContainer>
	bool 
	UdpSocket::receiveData(DataContainer& data){

		size_t len=0;

		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000; //100 millisec

		fd_set readSet;
		FD_ZERO( &readSet );        
		FD_SET( socketHandle_,  &readSet); 

		//socklen_t sockLen = sizeof(address_);

		if(0<select(socketHandle_+1, &readSet, NULL, NULL, &timeout )){
		
			socklen_t sockLen = sizeof(address_);
			len=recvfrom(socketHandle_, data.begin(), data.capacity(), 0, const_cast<sockaddr*>(&address_), &sockLen);
			
		} else{
			return false;
		}

		data.setSize(len); //this is quite clumsy... 
		return len>0;
	}


}
#endif