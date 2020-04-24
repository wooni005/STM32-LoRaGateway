# STM32-LoRaGateway
A STM32F103C8T6 (Blue Pill) LoRa Gateway. Not a LoRaWAN gateway for the The Things Network!

This gateway is receiving and sending LoRa messages with the HopeRF RFM95W chip.
The serial channel on the uUSB connector is used to	send and receive messages. 
Used protocol on the serial channel is like the RF12_Demo of JeeLabs.
This LoRa gateway is using the Arduino LoRa lib for interfacing the RFM95W.
InvertIQ is used to send messages to the nodes, the nodes do need to receive the messages with InvertIQ enabled.
The nodes are sending the messages with InvertIQ disabled and the gateway is also receiving by the InvertIQ disabled.
By using the technique a Gateway never receive messages from another Gateway
and a Node never receive message from another Node.
Only Gateway to Node and vice versa.

24 apr 2020: Arjan Wooning
