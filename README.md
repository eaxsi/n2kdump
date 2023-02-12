# n2kdump

Decode NMEA2000 communication

## Compile
> $ g++ -o n2kdump src/n2kdump.cpp

## Usage
Pipe candump to the n2kdump

> $ candump can0 | nk2dump


## Example output
```
ISO Request from: 0xe2 to: 0xfb
ISO Request from: 0xe2 to: 0xe2
ISO Address Claim from: 0xe2 to: 0xff
ISO Address Claim from: 0xfb to: 0xff
ISO Address Claim from: 0x54 to: 0xff
```
