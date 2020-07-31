# STM32-LoRaGateway
A STM32F103C8T6 (Blue Pill) LoRa Gateway. A STM32F103C8T6 (Blue Pill) LoRa Gateway. A gateway from LoRa to USB serial interface. Not a LoRaWAN gateway for the The Things Network!

This gateway is receiving and sending LoRa messages by using the HopeRF RFM95W chip.

## Serial Channel
The serial channel on the uUSB connector is used to	send and receive messages.
Used protocol on the serial channel is like the RF12_Demo of JeeLabs.

## LoRa communication
This LoRa gateway is using the Arduino LoRa lib for interfacing the RFM95W.
InvertIQ is used to send messages to the nodes, the nodes do need to receive the messages with InvertIQ enabled.
The nodes are sending the messages with InvertIQ disabled and the gateway is also receiving by the InvertIQ disabled.
By using the technique a Gateway never receive messages from another Gateway
and a Node never receive message from another Nodes, only Gateway to Node and vice versa.

## How to connect the RFM95W to the STM32F103

| RFM95W | STM32F103 |
| ------ | --------- |
| VCC    | 3V3       |
| GND    | GND       |
| MISO   | PA6       |
| MOSI   | PA7       |
| SCK    | PA5       |
| NSS    | PA4       |
| RESET  | PC14      |
| DIO0   | PA1       |

## Programming the board

I'm using the Arduino IDE with [STM32duino](https://github.com/stm32duino/Arduino_Core_STM32) in the board manager. No need to flash the bootloader, flashing the board with STLink V2, with the STLink V2 there is no need to use the switch the boot jumpers.
You can find here more information about the setup here:
https://alselectro.wordpress.com/2018/11/18/stm32f103-bluepill-getting-started-with-arduino-core/

## Usage

When the board is programmed, connect the micro-USB cable to the machine and startup a serial terminal program.
Press 'h' to see the available commands:

```
Available commands:
<nn>n     - set Gateway node ID (standard node ids are 1..7)
...,<nn>s - send data packet to node <nn>
<n>x      - set reporting format (0: decimal, 1: hex, 2: hex+ascii)
<n>r      - set reporting of RSSI signal strength (0:off, 1:on)
v         - display board name and board id
h         - this help

```

### Set NodeId

Default the Gateway Id is set to '1', but that can be changed.

Change the Gateway nodeId into '2':

```
2n
```

### Receiving a LoRa message

CRC is by default enabled, the first byte is the nodeId.

```
OK <nodeId> <dataByte0> .. <dataByteX>
```

For example from nodeId 3, with 2 data bytes:

```
OK 3 1 2
```

### Sending a LoRa message

```
<dataByte0>,<dataByteX>,<nodeId>s
```

For example to send 2 data bytes to nodeId 3:

```
1,2,3s
```

### Get the RSSI signal strength of the received message

Switch on the RSSI reporting mode:

```
1r
```
That will get for example a receive message response like this:

```
OK 3 1 2 RSSI -62
```

Switch off the RSSI reporting mode:

```
0r
```

### Message Ack

The Arduino LoRa library doesn't have any low-level message acknowledge implemented. I'm verifying a message on application level.