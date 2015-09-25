
#CMAKE File based on the code from Tomasz Bogdal (QueezyTheGreat), https://github.com/queezythegreat/arduino-cmake
cmake_minimum_required(VERSION 2.8.5)
include(CMakeParseArguments)

#paths need to be correctly configured, obviously. System paths
#or for example in Sublime in the current Build System...
#set(CMAKE_CROSSCOMPILING 1) 
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER   avr-gcc)
set(CMAKE_CXX_COMPILER avr-g++)

#needed so subdirectories can find include <> paths relative to this dir
set(CMAKE_INCLUDE_CURRENT_DIR ON) 

Project(WirelessServo)


set(CSTANDARD "-std=gnu99")
set(C++ "-std=c++11") #default in avr-g++ 4.8.1, but it removes warnings
#set(C++ "") #for avr-g++ 4.3.2
set(IGNORE "")
set(CDEBUG "-gstabs")
set(CWARN "-Wall")
set(CTUNING "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")
set(COPT "-Os -g")
# -ffunction-sections is specially important to reduce the Program and Data size,
# As it discards all unused code...
set(MINSIZEREL "-mcall-prologues -ffunction-sections -fdata-sections -fno-exceptions") 

#set(CINCS "-I${ArduinoCode_SOURCE_DIR}/libarduinocore")
set(CMCU "atmega328p")
set(CDEFS "-DF_CPU=16000000")
set(ARDUINOVERSION "-DARDUINO=105") 


set(CFLAGS "-mmcu=${CMCU} ${CDEBUG} ${CDEFS} ${COPT} ${CWARN} -Wstrict-prototypes ${CSTANDARD} ${CTUNING} ${CEXTRA} ${ARDUINOVERSION} ${MINSIZEREL} ")
set(CXXFLAGS "-mmcu=${CMCU} ${CDEBUG} ${CDEFS} ${CINCS} ${COPT} ${CTUNING} ${ARDUINOVERSION}  ${CWARN} ${C++} ${MINSIZEREL} ")


set(CMAKE_C_FLAGS ${CFLAGS})
set(CMAKE_CXX_FLAGS ${CXXFLAGS})

#for the project
#set(PROGRAM_NAME WirelesWebSocket)
set(PROGRAM_NAME ${PROJECT_NAME})
#set(PORT /dev/tty.usbmodemfa131)
set(PORT /dev/tty.usbserial-14P05910) #this is the Arduino Nano port
set(BAUD 57600)

include_directories (/Users/pablo/Code/arduino/libsCMake/variants/standard)

#not customizable

#Core
add_subdirectory(/Users/pablo/Code/arduino/libsCMake/libarduinocore "${CMAKE_CURRENT_BINARY_DIR}/libarduinocore") 
include_directories(/Users/pablo/Code/arduino/libsCMake/libarduinocore)
link_libraries(arduinocore "-lc -lm -Wl,--gc-sections")

#For CC3000 Wireless
add_subdirectory(/Users/pablo/Code/arduino/libsCMake/libraries/SPI "${CMAKE_CURRENT_BINARY_DIR}/SPI") 
include_directories(/Users/pablo/Code/arduino/libsCMake/libraries/SPI)
link_libraries(SPI "-lc -lm -Wl,--gc-sections")

add_subdirectory(/Users/pablo/Code/arduino/Arduino/libraries/Adafruit_CC3000 "${CMAKE_CURRENT_BINARY_DIR}/ACC3000")
include_directories(/Users/pablo/Code/arduino/Arduino/libraries/Adafruit_CC3000)
link_libraries(AdaFruit_CC3000 "-lc -lm -Wl,--gc-sections")

#For Servo
# add_subdirectory(/Users/pablo/Code/arduino/libsCMake/libraries/Servo "${CMAKE_CURRENT_BINARY_DIR}/Servo") 
# include_directories(/Users/pablo/Code/arduino/libsCMake/libraries/Servo)
# link_libraries(Servo "-lc -lm -Wl,--gc-sections")

#Using Softwre servo library instead
# add_subdirectory(/Users/pablo/Code/arduino/Arduino/libraries/SoftwareServo "${CMAKE_CURRENT_BINARY_DIR}/SoftwareServo")
# include_directories(/Users/pablo/Code/arduino/Arduino/libraries/SoftwareServo)
# link_libraries(SoftwareServo "-lc -lm -Wl,--gc-sections")


#Additional Arduino Libraries
# add_subdirectory(/Users/pablo/Code/arduino/Arduino/libraries/Adafruit_LEDBackpack "${CMAKE_CURRENT_BINARY_DIR}/AdafruitLEDBackpack")
# include_directories(/Users/pablo/Code/arduino/Arduino/libraries/Adafruit_LEDBackpack)
# link_libraries(Adafruit_LEDBackpack "-lc -lm -Wl,--gc-sections")



#local libs
add_subdirectory(mdns) 
include_directories(mdns)
link_libraries(mdns "-lc -lm -Wl,--gc-sections")

add_subdirectory(stepperpolyhedron) 
include_directories(stepperpolyhedron)
link_libraries(stepperpolyhedron "-lc -lm -Wl,--gc-sections")


add_executable(${PROGRAM_NAME} main.cpp)
#add_executable(${PROGRAM_NAME} main.cpp)


set_target_properties(${PROGRAM_NAME} PROPERTIES SUFFIX ".elf")


#Here we set the custom target for uploading:

#Programmers list for Arduino can be found in:
#/Applications/Arduino.app/Contents/Resources/Java/hardware/arduino/programmers.txt 


set(EEP_FLAGS -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0)
set(HEX_FLAGS -O ihex -R .eeprom)
set(SIZE_FLAGS -C --mcu=${CMCU})

add_custom_command(TARGET ${PROGRAM_NAME} POST_BUILD
					COMMAND avr-objcopy
					ARGS     ${EEP_FLAGS}
					         ${PROGRAM_NAME}.elf
					         ${PROGRAM_NAME}.eep
					COMMENT "Generating EEP image"
					VERBATIM)

add_custom_command(TARGET ${PROGRAM_NAME} POST_BUILD
					COMMAND avr-objcopy
					ARGS    ${HEX_FLAGS}
					        ${PROGRAM_NAME}.elf
					        ${PROGRAM_NAME}.hex
					COMMENT "Generating HEX image"
					VERBATIM)

 add_custom_command(TARGET ${PROGRAM_NAME} POST_BUILD
 					COMMAND avr-size
 					ARGS 	${SIZE_FLAGS}
 							${PROGRAM_NAME}.elf
 							${PROGRAM_NAME}.eep
 					COMMENT "Calculating size: \n"
					VERBATIM)


 add_custom_target(upload
                  COMMAND avrdude
                  -p ${CMCU} -c arduino -b ${BAUD} -P ${PORT} -D -V 
                  -U flash:w:${PROGRAM_NAME}.hex
                  -Ueeprom:w:${PROGRAM_NAME}.eep:i)



#this creates a custom make target called "serial". It can be called from terminal
#(from the build directory) through "make serial", and it will open the screen terminal application
#as a serial port listener... (check reference for screen: 
#http://www.cyberciti.biz/faq/unix-linux-apple-osx-bsd-screen-set-baud-rate/)
#and:
#http://ss64.com/osx/screen.html

#add_custom_target(serial COMMAND screen /dev/tty.usbmodemfa131 57600 cs8)
add_custom_target(serial COMMAND screen ${PORT} ${BAUD} cs8)
#we copy the script necessary...
file(COPY "/Users/pablo/Code/Arduino/cmake/callMakeSerial" DESTINATION ${PROJECT_BINARY_DIR})

