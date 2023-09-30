#include <OSCLite.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>

// For WiFi and socket/data
WiFiMulti wfMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

// Define OSC data types
enum OSCType {
    OSC_TYPE_INT32 = 'i',
    OSC_TYPE_FLOAT32 = 'f',
    OSC_TYPE_STRING = 's',
    OSC_TYPE_BLOB = 'b' // Adding support for OSC Blob type
};

// Define an OSC argument structure
struct OSCArgument {
    enum OSCType type;
    void* data;
    size_t size;
};

// Define an OSC message structure
struct OSCMessage {
    // Member variables
    char address[256]; // Assuming a maximum address length of 255 characters
    char typeTag[256]; // Assuming a maximum number of arguments
    struct OSCArgument* arguments;
    size_t numArguments;
};

// Constructor-like function for OSCMessage
struct OSCMessage* createOSCMessage(const char* address, const char* typeTag) {
    struct OSCMessage* msg = (struct OSCMessage*)malloc(sizeof(struct OSCMessage));
    if (msg == NULL) {
        return NULL; // Memory allocation failed
    }

    strncpy(msg->address, address, sizeof(msg->address));
    msg->address[sizeof(msg->address) - 1] = '\0'; // Ensure null termination

    strncpy(msg->typeTag, typeTag, sizeof(msg->typeTag));
    msg->typeTag[sizeof(msg->typeTag) - 1] = '\0'; // Ensure null termination

    msg->arguments = NULL;
    msg->numArguments = 0;

    return msg;
}

// Setter function to add an argument to the OSCMessage
void addOSCArgument(struct OSCMessage* msg, enum OSCType type, void* data, size_t size) {
    // Allocate memory for a new argument
    struct OSCArgument newArgument;
    newArgument.type = type;
    newArgument.data = malloc(size);
    if (newArgument.data == NULL) {
        return; // Memory allocation failed
    }
    memcpy(newArgument.data, data, size);
    newArgument.size = size;

    // Resize the arguments array and add the new argument
    msg->numArguments++;
    msg->arguments = (struct OSCArgument*)realloc(msg->arguments, sizeof(struct OSCArgument) * msg->numArguments);
    if (msg->arguments == NULL) {
        return; // Memory allocation failed
    }
    msg->arguments[msg->numArguments - 1] = newArgument;
}

// Destructor-like function for OSCMessage
void destroyOSCMessage(struct OSCMessage* msg) {
    if (msg != NULL) {
        // Free argument data
        for (size_t i = 0; i < msg->numArguments; i++) {
            free(msg->arguments[i].data);
        }
        free(msg->arguments);

        // Free the OSCMessage itself
        free(msg);
    }
}

// Function to encode an OSC message into a byte array
uint8_t* encodeOSCMessage(const struct OSCMessage *message, size_t *encodedLength) {
    size_t addressLength = strlen(message->address);
    size_t paddedAddressLength = (addressLength + 4) & ~3; // Ensure 4-byte alignment and null termination
    size_t typeTagLength = strlen(message->typeTag);
    size_t paddedtypeTagLength = (typeTagLength + 4) & ~3; // Ensure 4-byte alignment and null termination

    // Calculate the total size needed for encoding the message
    size_t totalSize = paddedAddressLength + paddedtypeTagLength;
    for (size_t i = 0; i < message->numArguments; i++) {
        totalSize += message->arguments[i].size;
    }
    
    *encodedLength = totalSize;

    uint8_t *encodedData = (uint8_t*)malloc(*encodedLength);
    if (!encodedData) {
        return NULL; // Memory allocation failed
    }
    
    size_t currentPosition = 0;
    
    // Write the OSC address and type tag
    // memcpy(encodedData + currentPosition, message->address, paddedAddressLength);
    memcpy(encodedData + currentPosition, message->address, addressLength);
    currentPosition += addressLength;
    int diff;
    diff = paddedAddressLength-addressLength;
    for (int i=0; i<diff; i++) {
          encodedData[currentPosition++]=0;
    }
    // currentPosition += paddedAddressLength;
    memcpy(encodedData + currentPosition, message->typeTag, typeTagLength);
    currentPosition += typeTagLength;
    diff = paddedtypeTagLength-typeTagLength;
    for (int i=0; i<diff; i++) {
          encodedData[currentPosition++]=0;
    }

    // Encode OSC arguments
    for (size_t i = 0; i < message->numArguments; i++) {
        if (message->arguments[i].type == OSC_TYPE_STRING) {
          Serial.println("String value: ");
          Serial.println((char *)message->arguments[i].data);
          Serial.println(message->arguments[i].size);

          memcpy(encodedData + currentPosition, message->arguments[i].data, message->arguments[i].size);
          currentPosition += message->arguments[i].size;
        }
        if (message->arguments[i].type == OSC_TYPE_INT32 || message->arguments[i].type == OSC_TYPE_FLOAT32) {
          // Perform endianness conversion in a few lines (only works for 32bit objects)
          uint32_t bigEndianValue;
          *((uint8_t*)&bigEndianValue + 0) = *((uint8_t*)message->arguments[i].data + 3);
          *((uint8_t*)&bigEndianValue + 1) = *((uint8_t*)message->arguments[i].data + 2);
          *((uint8_t*)&bigEndianValue + 2) = *((uint8_t*)message->arguments[i].data + 1);
          *((uint8_t*)&bigEndianValue + 3) = *((uint8_t*)message->arguments[i].data + 0);
          memcpy(encodedData + currentPosition,  &bigEndianValue, message->arguments[i].size);
          currentPosition += message->arguments[i].size; 
        }
    }
    
    return encodedData;
}

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
      struct OSCMessage* oscMsg = createOSCMessage("/max/led", ",ifs");

      // Add arguments to the OSCMessage using the setter function
      int32_t intValue = 1;
      float floatValue = 9.109383701;
      const char* stringValue = "Hello, MAX!";

      addOSCArgument(oscMsg, OSC_TYPE_INT32, &intValue, sizeof(int32_t));
      addOSCArgument(oscMsg, OSC_TYPE_FLOAT32, &floatValue, sizeof(float));
      addOSCArgument(oscMsg, OSC_TYPE_STRING, (void*)stringValue, strlen(stringValue) + 1); // +1 for null terminator

      size_t encodedLength;

      // Encode the OSC message
      uint8_t *encodedMessage = encodeOSCMessage(oscMsg, &encodedLength);
      Serial.println("\t Encoded Message");
      hexdump(encodedMessage, encodedLength);
      Serial.println(encodedLength);
      Serial.println("\t End Encoded Message");
			webSocket.sendBIN(encodedMessage, encodedLength);

      // Cleanup
      destroyOSCMessage(oscMsg);
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

	wfMulti.addAP("VectorVictor", "whatsyour");

	//WiFi.disconnect();
	while(wfMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
	webSocket.begin("192.168.1.19", 12345, "/");

	// // event handler
	webSocket.onEvent(webSocketEvent);

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(1000);
}

void loop() {
	webSocket.loop();
}