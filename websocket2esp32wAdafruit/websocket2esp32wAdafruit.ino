#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <OSCMessage.h>
#include <Adafruit_NeoPixel.h>

// For WiFi and socket/data
WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
OSCMessage msg;
OSCMessage m2e;
OSCErrorCode error;

// For NeoPixel
const int numPixels = 8;
const int pixelPin = 9;
Adafruit_NeoPixel ring = Adafruit_NeoPixel(numPixels, pixelPin);

#define USE_SERIAL Serial
#define PAULSSUGGESTION False

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		USE_SERIAL.printf("%02X ", *src);
		src++;
	}
	USE_SERIAL.printf("\n");
}

void handleNeoPixel(OSCMessage &msg){
  int led, r, g, b;

  /** Grab the ints in the OSC packet for 
  led address, red, green and blue channel settings
  **/
  led = msg.getInt(0);
  r = msg.getInt(1);
  g = msg.getInt(2);
  b = msg.getInt(3);
  ring.setPixelColor(led,r,g,b);

  ring.show();

  USE_SERIAL.printf("/led: n->%d, r->%d, g->%d, b->%d \n", led, r, g, b);
  // USE_SERIAL.println(led);
  msg.empty();

}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
	switch(type) {
    	case WStype_CONNECTED:
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
			break;
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[WSc] Disconnected!\n");
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);
			break;
		case WStype_BIN:
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			// hexdump(payload, length);
      int size;
      size = length;
      // Need to fill OSC data structure with packet
    if (size > 0) {
      while (size--) {
        msg.fill(payload[length-size-1]);
      }
    if (!msg.hasError()) {
      msg.dispatch("/max/led", handleNeoPixel); // Sends data to function
    } 
    else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
      }
    }

			// const char * message = msg.send();     
      // webSocket.sendBIN(msg.bytes(), length);
			// Print p;
			// m2e.send(p);
			// Serial.println("Two packets->Begin");
			// Serial.println(msg.bytes());
			// hexdump(payload, length);
			// uint8_t* encodedMessage;
			// encodedMessage = encodeOSCMessage(&msg, &length);
			// hexdump(encodedMessage, length);
			// hexdump(payload, length);
			// Serial.println("Two packets->End");
			// webSocket.sendBIN(encodedMessage, length);

			webSocket.sendBIN(payload, length);

			break;
		  case WStype_ERROR:			
		  case WStype_FRAGMENT_TEXT_START:
		  case WStype_FRAGMENT_BIN_START:
		  case WStype_FRAGMENT:
		  case WStype_FRAGMENT_FIN:
			break;
	}

}

void setup() {

	// USE_SERIAL.begin(921600);
	USE_SERIAL.begin(115200);
	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for(uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	WiFiMulti.addAP("VectorVictor", "whatsyour");

	//WiFi.disconnect();
	while(WiFiMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
	webSocket.begin("192.168.1.19", 12345, "/");

	// // event handler
	webSocket.onEvent(webSocketEvent);

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);
}

void loop() {
	webSocket.loop();
}

// void send(OSCMessage * msg, uint8_t * p, int len){
//     uint8_t nullChar = '\0';
//     //send the address
//     int addrLen = strlen(msg->getAddress()) + 1;
//     //padding amount
//     int addrPad = padSize(addrLen);
// 		hexdump(msg->getAddress(), addrLen);	

// }

// Function to encode an OSC message into a byte array
// uint8_t* encodeOSCMessage(OSCMessage *message, size_t *encodedLength) {
//     size_t addressLength = strlen(message->getAddress());
//     size_t paddedAddressLength = (addressLength + 4) & ~3; // Ensure 4-byte alignment
    
//     *encodedLength = paddedAddressLength + message->bytes();
//     uint8_t *encodedData = (uint8_t*)malloc(*encodedLength);
//     if (!encodedData) {
//         return NULL; // Memory allocation failed
//     }
    
//     // Copy the OSC address and pad with null bytes
// 		const char* addr = message->getAddress();
//     // strcpy((char*)encodedData, addr);
//     // memset(encodedData + addressLength + 1, 0, paddedAddressLength - addressLength);
    
//     // Copy the OSC message data
//     // memcpy(encodedData + paddedAddressLength, message->data, message->dataCount);
    
//     return encodedData;
// }

// Function to encode an OSC message into a byte array
// uint8_t* encodeOSCMessage(const struct OSCMessage *message, size_t *encodedLength) {
//     size_t addressLength = strlen(message->getAddress());
//     size_t typeTagLength = strlen(message->typeTag);
    
//     // Calculate the total size needed for encoding the message
//     size_t totalSize = 1 + addressLength + 1 + typeTagLength;
//     for (size_t i = 0; i < message->numArguments; i++) {
//         totalSize += message->arguments[i].size;
//     }
    
//     *encodedLength = totalSize;
//     uint8_t *encodedData = (uint8_t*)malloc(*encodedLength);
//     if (!encodedData) {
//         return NULL; // Memory allocation failed
//     }
    
//     size_t currentPosition = 0;
    
//     // Write the OSC address and type tag
//     encodedData[currentPosition++] = '/';
//     memcpy(encodedData + currentPosition, message->address, addressLength);
//     currentPosition += addressLength;
    
//     encodedData[currentPosition++] = ',';
//     memcpy(encodedData + currentPosition, message->typeTag, typeTagLength);
//     currentPosition += typeTagLength;
    
//     // Encode OSC arguments
//     for (size_t i = 0; i < message->numArguments; i++) {
//         memcpy(encodedData + currentPosition, message->arguments[i].data, message->arguments[i].size);
//         currentPosition += message->arguments[i].size;
//     }
    
//     return encodedData;
// }