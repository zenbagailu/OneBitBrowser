
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

#ifndef STEPPERPOLYHEDRONSTEPPER_H_
#define STEPPERPOLYHEDRONSTEPPER_H_ 

#include <stdint.h>

#define STEPPERDEBUG

namespace stepperpolyhedron {
	class Stepper final
	{

	public:
		enum Direction: bool {Up=false,Down=true};

		Stepper(const uint8_t dirPin,const uint8_t stepPin,const uint8_t sleepPin,const uint8_t switchPin, 
		        const uint32_t maxVal, const uint32_t delayt=100); //100 quite nice. 40 is about as fast as it goes. 
		~Stepper();
		Stepper( Stepper const&)=delete;
		Stepper& operator = (Stepper const&)=delete;

		
		void moveUp();
		void moveDown();

	private:
		void takeStep();
	
	private:

	Direction state_;

	const uint8_t dirPin_ = 7;
	const uint8_t stepPin_ = 8;
	const uint8_t sleepPin_ = 9;
	const uint8_t switchPin_ = 6;

	const uint32_t maxVal_=32725; //up position of the stepper. Needs to be calibrated
	//constexpr uint32_t delayt=300;
	const uint32_t delayt_=100; 
	uint32_t pos_;
	};
}
#endif