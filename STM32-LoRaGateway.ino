/*
	A STM32F103C8T6 (Blue Pill) LoRa Gateway, but not a gateway like for the
	The Things Network.
	Please read the README.md for details

	24 apr 2020: Arjan Wooning
*/

#include <SPI.h>
#include <LoRa.h>
#include <EEPROM.h>

#define SERIAL_BAUD	 57600

// LoRa configuration
const long frequency = 866E6;	// LoRa Frequency

const int csPin = PA4;				 // LoRa radio chip select
const int resetPin = PC14;		 // LoRa radio reset
const int irqPin = PA1;				// change for your board; must be a hardware interrupt pin

// Gateway configuration
typedef struct {
	byte nodeId;						// Which Gateway NodeId 1...7
	byte hex_output	:2;	 // 0 = dec, 1 = hex, 2 = hex+ascii
	byte rssi_output	:1;	 // 0 = dec, 1 = hex, 2 = hex+ascii
	byte spare_flags	:5;
} GPIOconfig;

static GPIOconfig config;

#define MAX_PACKET_SIZE	10

static word value;
static byte stack[MAX_PACKET_SIZE + 4], top, sendLen, destNodeId;
static boolean sendMsg = false;

//Message max 31 bytes
typedef struct {
	byte nodeId;
	byte msg [MAX_PACKET_SIZE - 1];
} Payload;

void LoRa_rxMode(){
	LoRa.disableInvertIQ();							 // normal mode
	LoRa.receive();											 // set receive mode
}

void LoRa_txMode(){
	LoRa.idle();													// set standby mode
	LoRa.enableInvertIQ();								// active invert I and Q signals
}

void LoRa_sendMessage(Payload payload, byte payloadLen) {
	LoRa_txMode();												// set tx mode
	LoRa.beginPacket();									 // start packet
	LoRa.write((byte*) &payload, payloadLen); // add payload
	LoRa.endPacket(true);								 // finish packet and send it
}

void onTxDone() {
	// Serial.println("TxDone");
	LoRa_rxMode();
}

static void printOneChar (char c) {
	Serial.print(c);
}

static void showString (PGM_P s) {
	for (;;) {
		char c = pgm_read_byte(s++);
		if (c == 0)
				break;
		if (c == '\n')
				printOneChar('\r');
		printOneChar(c);
	}
}

static void saveConfig () {
	EEPROM.write(0, (int)config.nodeId);
}

static void showNibble (byte nibble) {
	char c = '0' + (nibble & 0x0F);
	if (c > '9')
		c += 7;
	Serial.print(c);
}

static void showByte (byte value) {
	if (config.hex_output) {
		showNibble(value >> 4);
		showNibble(value);
	} else
		Serial.print((word) value);
}

static void loadConfig () {
	config.nodeId = EEPROM.read(0);

	if ( config.nodeId > 7) {
		Serial.println("Init EEPROM for first time");
		config.nodeId = 1;
		saveConfig();
	}
}

const char helpText[] PROGMEM =
	"\n"
	"Available commands:\n"
	"  <nn>n     - set Gateway node ID (standard node ids are 1..7)\n"
	"  ...,<nn>s - send data packet to node <nn>\n"
	"  <n>x      - set reporting format (0: decimal, 1: hex, 2: hex+ascii)\n"
	"  <n>r      - set reporting of RSSI signal strength (0:off, 1:on)\n"
	"  v         - display board name and board id\n"
	"  h         - this help\n"
;

static void showHelp () {
	showString(helpText);
	Serial.println();
	Serial.print("nodeId=");
	Serial.println(config.nodeId, DEC);
}

//Cmnd example: 10,3a
static void handleSerialInput (char c) {
	byte stm32pinNr = 0;

	if ('0' <= c && c <= '9') {
		value = 10 * value + c - '0';
		return;
	}

	if (c == ',') {
		if (top < sizeof stack)
			stack[top++] = value; // truncated to 8 bits
		value = 0;
		return;
	}

	if ('a' <= c && c <= 'z') {
		Serial.print("> ");
		for (byte i = 0; i < top; ++i) {
			Serial.print((word) stack[i]);
			printOneChar(',');
		}
		Serial.print(value);
		Serial.println(c);
	}

	if (c > ' ') {
		switch (c) {

		case 's': // send packet to node ID N, no ack
			sendMsg = true; // Send the message
			sendLen = top;	// Payload length
			destNodeId = value;	 // Destination NodeId
			break;

		case 'n': // set node id
			config.nodeId = value & 0x3F;
			saveConfig();
			break;

		case 'x': // set reporting mode to decimal (0), hex (1), hex+ascii (2)
			config.hex_output = value;
			saveConfig();
			break;

		case 'r': // set reporting of RSSI signal strength
			config.rssi_output = value;
			saveConfig();
			break;

		case 'v': //display the interpreter version and configuration
			Serial.print("[LORA-GATEWAY]");
			break;

		case 'h':
			showHelp();
			break;

		// default:
		//		 Serial.print("Unknown input: ");
		//		 Serial.print(c);
		//		 Serial.print(" (0x");
		//		 Serial.print(c, HEX);
		//		 Serial.println(")");
				// showHelp();
		}
	}
	value = top = 0;
}

boolean runEvery(unsigned long interval)
{
	static unsigned long previousMillis = 0;
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= interval)
	{
		previousMillis = currentMillis;
		return true;
	}
	return false;
}


void setup() {
	Serial.begin(SERIAL_BAUD);									 // initialize serial

	while (!Serial);

	Serial.println();
	loadConfig();
	Serial.println("[LORA-GATEWAY]");

	LoRa.setPins(csPin, resetPin, irqPin);

	if (!LoRa.begin(frequency)) {
		Serial.println("LoRa init failed. Check your connections.");
		while (true);											 // if failed, do nothing
	}

	// Serial.println("LoRa init succeeded.");
	// Serial.println();
	// Serial.println("Only receive messages from nodes");
	// Serial.println("Tx: invertIQ enable");
	// Serial.println("Rx: invertIQ disable");
	// Serial.println();

	//LoRa.setTxPower(20);
	LoRa.enableCrc();
	LoRa.onReceive(onReceive);
	LoRa.onTxDone(onTxDone);
	LoRa_rxMode();
}

void onReceive(int packetSize) {
	if (packetSize > MAX_PACKET_SIZE)
		Serial.print("ERROR packetSize too long: "); // Start received message
	else
		Serial.print("OK"); // Start received message

	for (int i = 0; i < packetSize; i++) {
		if (!config.hex_output)
			printOneChar(' ');
		showByte((char)LoRa.read());
	}
	if (config.rssi_output) {
		Serial.print(" RSSI ");
		Serial.print(LoRa.packetRssi());
	}
	Serial.println();
	// String message = "";
	//
	// while (LoRa.available()) {
	//	 message += (char)LoRa.read();
	// }
	//
	// Serial.print("Gateway Receive: ");
	// Serial.println(message);
}

void loop() {
	if (Serial.available()) {
			handleSerialInput(Serial.read());
	} else {
		if (sendMsg) {
			sendMsg = false;

			// Check received package
			Payload txPayload;

			// Serial.print("Send to NodeID: ");
			// Serial.print(destNodeId, DEC);
			// Serial.print(" - msgLen:");
			// Serial.print(sendLen, DEC);
			//
			// Serial.print(" - data:");
			txPayload.nodeId = destNodeId;
			for (char i = 0; i < sendLen; i++) {
				// Serial.print(' ');
				// Serial.print(stack[i], DEC);
				txPayload.msg[i] = stack[i];
			}
			// Serial.println();
			LoRa_sendMessage(txPayload, sendLen + 1); // send message

			//Serial.println("Send Message!");
		}
		delay(10); //10ms
	}
}
