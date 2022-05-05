/*
    arduino-I2C-debug-tool v1.0
    Copyright (C) 2022 S.Pauthner
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
/* Usage:
 * # mastermode:
 *    [r/w] [address] [data0/sizeOfRequests] [data1] [data2] [data3] ... ([s])
 *    Seperate the blocks with spaces or commas; the s at the end should not be needed
 *    e.g. write: w 42 155 20 12 30 (s)    or    w42,115 20,12 30
 *         read : r 32 4 (s)               or    r0x20,4
 */
#include <Arduino.h>
#include <Wire.h>

// Configuration
#define baud 9600             // Baud rate for serial communication
// #define slaveAddress 0x20  // Comment to use master mode !Slave mode not supported yet!
#define monitor               // Option to print the actual input; usefull on monitors that don't show the entered characters

#define version     1.0
#define invalid     -1
#define inputlength 100
#define datalength  100

// global Variables
char input[inputlength];
uint8_t actualInputIndex = 0;
uint8_t data[datalength];
uint8_t actualDataIndex = 0;
uint8_t address = 0;
bool read = false;

// Programm
void clearInput() {
  for (uint8_t i = 0; i < sizeof(input); i++) {
    input[i] = invalid;
  }
  actualInputIndex = 0;
  for (uint8_t i = 0; i < sizeof(data); i++) {
    data[i] = 0;
  }
  actualDataIndex = 0;
  address = 0;
  read = false;
}

void printInput(String title) {
  Serial.print(title);
  for (uint8_t i = 0; i < actualInputIndex; i++) {
    Serial.print(input[i]);
  }
  Serial.println();
}

void printData(String title) {
  Serial.print(title);
  for (size_t i = 1; i <= actualDataIndex; i++) {
    Serial.print(data[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void makeTidy() {         // make eventual bad input to a tidy array (replace , with spaces)(remove multiple spaces)
  char array[actualInputIndex];
  uint8_t count = 0;
  uint8_t newMaxIndex = actualInputIndex;

  for (uint8_t i = 0; i < actualInputIndex; i++) { // replace , with spaces
    if (input[i] == ',') {
      input[i] = ' ';
    }
  }

  for (uint8_t i = 0; i < actualInputIndex; i++) { // remove double spaces
    if (input[i] == ' ' && array[count-1] == ' ') {
      newMaxIndex--;
    }
    else{
      array[count] = input[i];
      count++;
    }
  }

  for (uint8_t i = newMaxIndex; i > 0; i--) {     // remove spaces at the end
    if (array[i] == ' ') {
      newMaxIndex--;
    }
    else{
      break;
    }
  }
  if (array[newMaxIndex-1] == 13) {               // remove \r at the end
    newMaxIndex--;
  }
  if (array[newMaxIndex-1] == 10) {               // remove eventual \n at the end
    newMaxIndex--;
  }

  actualInputIndex = newMaxIndex;                 // override input and actualInputIndex
  for (size_t i = 0; i < actualInputIndex; i++) {
    input[i] = array[i];
  }

}

void generateData() {
  actualDataIndex = 0;
  for (uint8_t i = 0; i < actualInputIndex; i++) {
    if ((input[i] == 'r' || input[i] == 'w') && input[i+1] == ' ') {
      i++;
    }
    else if ((input[i] == 'r' || input[i] == 'w') && input[i+1] != ' ') {

    }
    else if (input[i] == ' ') {  // count count up by space
      actualDataIndex++;
    }
    else {
      data[actualDataIndex] = data[actualDataIndex]*10 + (input[i]-48);  // move old number left and add new number right
    }
  }
}

void generateAddress() {
  address = 0;
  uint8_t hex = 0;        // 0 = no hex format; else x position
  for (size_t i = 0; i < actualInputIndex; i++) {
    if (input[i] == 'x') {
      hex = i;
      break;
    }
  }
  if (hex !=0) {          // if I2C address in hex
    address += (input[hex+1]-48)*16;
    address += (input[hex+2]-48);

  }
  else {
    address = data[0];    // if I2C address in dec
  }
}

void generateRW() {
  read = false;
  for (size_t i = 0; i < actualInputIndex; i++) {
    if (input[i] != ' ') {
      if (input[i] == 'r' || input[i] == 'R') {
        read = true;        // Set operation to read if first letter r or R; else operation is write
      }
      break;
    }
  }
}

void readI2C() {
  Wire.requestFrom(address, data[1]);
  Serial.print("Requesting ");
  Serial.print(data[1]);
  Serial.print(" byte(s) from slave at ");
  Serial.println(address);
  if (Wire.available()) {
    while (Wire.available()) {
      Serial.print(Wire.read());
      Serial.print(" ");
    }
  }
  else {
    Serial.print("No data received!");
  }
  Serial.println();

}

void writeI2C() {
  Wire.beginTransmission(address);
  for (uint8_t i = 1; i <= actualDataIndex; i++) {
    Wire.write(data[i]);
  }
  Wire.endTransmission();

  Serial.print("Wrote to slave at ");
  Serial.print(address);
  printData(" data: ");

}

void doI2Cstuff() {
  makeTidy();
  //printInput("After makeTidy: ");
  generateData();
  //printData("generated Data array: ");
  generateAddress();
  //Serial.print("Target address: ");
  //Serial.println(address);
  generateRW();
  //Serial.print("Read? ");
  //Serial.println(read);

  if (read) {
    readI2C();
  }
  else {
    writeI2C();
  }

  clearInput();
}

void serialEvent() {
  while (Serial.available()) {
    input[actualInputIndex] = Serial.read();
    if (input[actualInputIndex] == 's' || input[actualInputIndex] == 10 ){      // action if 's' or '\n'
      doI2Cstuff();
    }
    else if (input[actualInputIndex] == 8) {                                    // action by backspace
      if (actualInputIndex > 0) {
        actualInputIndex--;
      }
    }
    else{
      actualInputIndex++;
    }
  }
  #ifdef monitor
  printInput("Actual input: ");
  #endif
}

void setup() {
  clearInput();

  Serial.begin(baud);

  #ifndef slaveAddress
  Wire.begin();
  #else
  Wire.begin(slaveAddress);
  #endif

  Serial.print("Welcome to I2C debug tool v");
  Serial.println(version);
  Serial.println("Usage (master mode): [r/w] [address] [quantityOfBytesToRead/data0] [data1] [data2] ...");
  Serial.println("Seperate the blocks with spaces or commas");


}

void loop() {
  // put your main code here, to run repeatedly:
}
