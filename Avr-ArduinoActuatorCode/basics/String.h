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

#ifndef BASICSSTRING_H_
#define BASICSSTRING_H_ 

//just a wrapper for chars... trying to keep the C++ style separate from C style
#include <stdlib.h> //for strlen
namespace basics{
	class String{
	public:
		String(const char* cs):charPtr_{cs}{};
		virtual ~String(){};
		size_t length() const{return strlen(charPtr_);}
		static const size_t CharSize=sizeof(char); //it is going to be 1...but still

		const char* data() const {return charPtr_;}

	private:
		const char* charPtr_;
		
	};


}
#endif