#include <OSCMLite.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include "secrets.h"

#include <PulseSensorPlayground.h>

// For WiFi and socket/data
WiFiMulti wfMulti;
WebSocketsClient webSocket;
OSCMLite oscm;

#define USE_SERIAL Serial


// Initial Setting sfor the Pulse Sensor
const int PULSE_INPUT = A8;
const int PULSE_BLINK = LED_BUILTIN;
const int PULSE_FADE = 5;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle
int Signal;                // holds the incoming raw data. Signal value can range from 0-1024
/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;


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
			// USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			// hexdump(payload, length);

      // Create an OSCMessage object using the constructor-like function
       OSCMLite* oscMsg = oscm.createOSCMessage("/max/led", ",f");

      // Add arguments to the OSCMessage using the setter function
      float floatValue = Signal;
      oscm.addOSCArgument(oscMsg, oscm.OSC_TYPE_FLOAT32, &floatValue, sizeof(float));

      size_t encodedLength;

      // Encode the OSC message
      uint8_t *encodedMessage = oscm.encodeOSCMessage(oscMsg, &encodedLength);
      // hexdump(encodedMessage, encodedLength);
			webSocket.sendBIN(encodedMessage, encodedLength);

      // Cleanup
      oscm.destroyOSCMessage(oscMsg);
			break;
	}

}

void setup() {
  pinMode(PULSE_BLINK,OUTPUT);         // pin that will blink to your heartbeat!

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
    /*
     Wait a bit.
     We don't output every sample, because our baud rate
     won't support that much I/O.
  */
  delay(10);
  Signal = analogRead(PULSE_INPUT);  // Read the PulseSensor's value.
                                              // Assign this value to the "Signal" variable.

  //  Serial.println(Signal);                    // Send the Signal value to Serial Plotter.


  //  if(Signal > THRESHOLD){                          // If the signal is above "550", then "turn-on" Arduino's on-Board LED.
  //    digitalWrite(PULSE_BLINK,HIGH);
  //  } else {
  //    digitalWrite(PULSE_BLINK,LOW);                //  Else, the sigal must be below "550", so "turn-off" this LED.
  //  }

	webSocket.loop();
}