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

#ifndef BASICSBUFFER_H_
#define BASICSBUFFER_H_ 

#include <stdlib.h>
//this is more of a test of some c++ (c++11 included) features in avr gcc)
//it should really be improved... it should give the basic functionality of 
//the stl array (C++11), and include algorithms and functions for integration
//with traditional C style arrays

namespace basics {

	//std not ported yet, so we do a simple fixed size array. It will truncate if the size is
	//not correct...
	template<class T, size_t MaxSize> class Buffer{

	public:

		typedef T const*				const_iterator;
		typedef T *						iterator;
		typedef size_t 					size_type;
		typedef T  						type;
		typedef T 		    			consec_iterator[];
		typedef consec_iterator const 	cost_consec_iterator;

		Buffer( Buffer const&)=delete; //we prevent copying by default
		Buffer& operator = (Buffer const&)=delete;

		Buffer():numElements_{0}{}
		static size_type ElemSize(){return sizeof(type);} 
		size_type size() const{ return numElements_;}
		void setSize(size_type size) {numElements_=size;} //this is a hack...
		size_type capacity()const{ return MaxSize;}
		const_iterator begin() const{return &iarr_[0];}
        const_iterator end() const{return &iarr_[numElements_];}
        iterator begin(){return &iarr_[0];}
        iterator end(){return &iarr_[numElements_];}
        T& operator [](int indx){return iarr_[indx];}
        T const& operator [](int indx)const {return iarr_[indx];}
        void clear(){numElements_ = 0;}

		bool push(T const& element);
		bool push(const_iterator it, size_type size);

		template<class Iterator>
		size_t compare(Iterator otherIt, size_t num);
	
    private:  	
		T iarr_[MaxSize];
		size_type numElements_;

	};

	template<class T, size_t MaxSize>
	bool 
	Buffer<T,MaxSize>::push(T const& element){  
		if(numElements_ < MaxSize){
			iarr_[numElements_++]=element;
			return true; //no exceptions in AVR, this makes it manageable
		}else{
			return false;
		}
	}

	//The data used here needs to be consecutive in memory... it cannot be pointer to 
	//a T object (the Buffer::iterator type) in a container that does not store its data
	//consequtively in memory...

	template<class T, size_t MaxSize>
	bool 
	Buffer<T,MaxSize>::push(cost_consec_iterator it, size_type size){  //it would be great to have std::copy implemented instead.
		if(numElements_+size <= MaxSize){
			memcpy(&iarr_[numElements_],it, size*sizeof(T));
			numElements_+=size;
			return true;
		}else{
			memcpy(&iarr_[numElements_],it, (MaxSize-numElements_)*sizeof(T)); //truncate
			numElements_=MaxSize;
			return false;
		}
	}

	template<class T, size_t MaxSize>
	template<class Iterator>
	size_t
	Buffer<T,MaxSize>::compare(Iterator otherIt, size_t num){

		num= num<=numElements_ ? num : numElements_;

		auto it=begin();

		for(size_t i=0; i<num; ++i){
			
			if(*otherIt++ != *it++){
				return i;
			}
		}
		return num;
	}

}


#endif