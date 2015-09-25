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

/*
 * This code controls hardware that uses a a bipolar stepper motor using the Easydriver PCB:
 * http://schmalzhaus.com/EasyDriver/
 */

#include "Stepper.h"
#include <Arduino.h>

namespace stepperpolyhedron {
	Stepper::Stepper(const uint8_t dirPin,
	                 const uint8_t stepPin,
	                 const uint8_t sleepPin,
	                 const uint8_t switchPin,
	                 const uint32_t maxVal, 
	                 const uint32_t delayt):
	dirPin_{dirPin},
	stepPin_(stepPin),
	sleepPin_(sleepPin),
	switchPin_(switchPin),
	maxVal_(maxVal),
	delayt_(delayt){

		pinMode(dirPin_, OUTPUT);
		pinMode(stepPin_,  OUTPUT);
		pinMode(sleepPin_,  OUTPUT);
		pinMode(switchPin_,  INPUT_PULLUP);

		//start by calibrating it in down pos_... (otherwise it won't know where it is)
		//so it moves down and sets pos_ to 0)
		moveDown(); 
		
	}

	void Stepper::takeStep(){

		digitalWrite(stepPin_, LOW);
		delayMicroseconds(delayt_);
		digitalWrite(stepPin_, HIGH);
		delayMicroseconds(delayt_);
	}

	void Stepper::moveDown(){
	  //wake up stepper driver
	  digitalWrite(sleepPin_, HIGH); 
	  //set direction 
	  digitalWrite(dirPin_, Down);

	  while(digitalRead(switchPin_)==false){ //while the switch pin is not reached
	      takeStep();
	  }
#ifdef STEPPERDEBUG
	  Serial.println(F("Stop switch detected."));
#endif 
	  pos_=0; //it is 0... (calibrate every time) necseary for moving up
	 
	  //put stepper driver to sleep
	  digitalWrite(sleepPin_, LOW);
	  //set state
	  state_=Down;
	}

	void Stepper::moveUp(){

		//we need to check if it is already up...
		if(state_==Up){
			return;
		}

	  //wake up stepper driver
	  digitalWrite(sleepPin_, HIGH); 
	  //set direction 
	  digitalWrite(dirPin_, Up);

	  //if going up
	  while(pos_<maxVal_){
	    if(digitalRead(switchPin_)==true){
	      pos_=0; //if it has overshot, reset the position, so it is 0 when the sensor looses contact
	    }else{
	       pos_++;
	    }
	    takeStep();
	  }

	  //top reached

#ifdef STEPPERDEBUG
	  Serial.print(F("Top reached at pos: "));
	  Serial.println(pos_);
#endif 
	  //put stepper driver to sleep
	  digitalWrite(sleepPin_, LOW);

	  state_=Up;

	}

}