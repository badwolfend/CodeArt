#include <OSCMLite.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include "secrets.h"
// For WiFi and socket/data
WiFiMulti wfMulti;
WebSocketsClient webSocket;
OSCMLite oscm;

#define USE_SERIAL Serial

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
			hexdump(payload, length);

      // Create an OSCMessage object using the constructor-like function
       OSCMLite* oscMsg = oscm.createOSCMessage("/max/led", ",ifs");

      // Add arguments to the OSCMessage using the setter function
      int32_t intValue = 1;
      float floatValue = 9.109383701;
      const char* stringValue = "Hello, MAX!";
      
      oscm.addOSCArgument(oscMsg, oscm.OSC_TYPE_INT32, &intValue, sizeof(int32_t));
      oscm.addOSCArgument(oscMsg, oscm.OSC_TYPE_FLOAT32, &floatValue, sizeof(float));
      oscm.addOSCArgument(oscMsg, oscm.OSC_TYPE_STRING, (void*)stringValue, strlen(stringValue) + 1); // +1 for null terminator

      size_t encodedLength;

      // Encode the OSC message
      uint8_t *encodedMessage = oscm.encodeOSCMessage(oscMsg, &encodedLength);
      Serial.println("\t Encoded Message");
      hexdump(encodedMessage, encodedLength);
      Serial.println(encodedLength);
      Serial.println("\t End Encoded Message");
			webSocket.sendBIN(encodedMessage, encodedLength);

      // Cleanup
      oscm.destroyOSCMessage(oscMsg);
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
	wfMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

	//WiFi.disconnect();
	while(wfMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
	webSocket.begin(IP_ADDR, PORT, "/");

	// // event handler
	webSocket.onEvent(webSocketEvent);

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(1000);
}

void loop() {
	webSocket.loop();
}