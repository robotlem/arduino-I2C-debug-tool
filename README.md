# arduino-I2C-debug-tool
An easy program to debug and test I2C systems

# Usage
- Mastermode:
  - Type into the serial console:
  - [r/w] [address] [quantityToRead/byte0] [byte1] [byte2] [byte3] ...
  - Seperate the blocks with spaces or commas
  - Example (write to 0x30): w 0x30 120 30 115  or  w48,120,30,115
  - Example (read from 0x20): r 0x20 4  or  r32 4
- Slavemode:
  - coming soon...
   
