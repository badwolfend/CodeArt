# OSCMLite

OSCMLite is a lightweight C library for encoding and decoding OSC (Open Sound Control) messages.

## Table of Contents
- [Introduction](#introduction)
- [Usage](#usage)
- [Example](#example)

## Introduction
OSCMLite is designed to simplify the process of working with OSC messages in C so they can easily be packaged in a packet to send off device. It provides an interface for creating, encoding, and decoding OSC messages.
Specifically, the main point of this is to allow conversion of OSC object to an encoded byte object.  This is universal to OSC and receivers should be able to read the byte stream and understand the data (assuming they are running at least OSC 1.1). 

## Usage
To use OSCMLite in your project, you'll need to include the library and follow these basic steps:

1. Create an OSC message using `createOSCMessage`.
2. Add arguments to the message using `addOSCArgument`.
3. Encode the message into a byte array using `encodeOSCMessage`.
4. Send the encoded message or decode received OSC messages as needed.

## Example
Here's a simple example that is contained in the TESTOSCLibrary project folder:
```c
// Create an OSCMessage object using the constructor-like function
OSCMLite* oscMsg = oscm.createOSCMessage("/max/led", ",ifs");
```
Here we create the oscMsg with address "/max/led" and with 3 types of data (arguments) int32 (i), float32 (f) and string (s). 


Next, add values to the OSCMessage.
```c
// Add arguments to the OSCMessage using the setter function
int32_t intValue = 1;
float floatValue = 9.109383701;
const char* stringValue = "Hello, MAX!";
      
oscm.addOSCArgument(oscMsg, oscm.OSC_TYPE_INT32, &intValue, sizeof(int32_t));
oscm.addOSCArgument(oscMsg, oscm.OSC_TYPE_FLOAT32, &floatValue, sizeof(float));
oscm.addOSCArgument(oscMsg, oscm.OSC_TYPE_STRING, (void*)stringValue, strlen(stringValue) + 1); // +1 for null terminator
```
Here we have used ``` addOSCArgument() ``` to add the three values to our OSC object.  The function requires us to give the OSCMLite object address, the data type, the value to add, and the total size of the data you will be adding.  

Next, we will encode this object in an integer array of bytes.  This would be the data you would package in a UDP/TCP packet.  This encoding is the primary reason for this library.  Since we get the raw data in the packet, we can put it in any transmission protocol we want.  

```c
size_t encodedLength;

// Encode the OSC message
uint8_t *encodedMessage = oscm.encodeOSCMessage(oscMsg, &encodedLength);
Serial.println("\t Encoded Message");
hexdump(encodedMessage, encodedLength);
```
Here we have simply used the ``` encodeOSCMessage() ``` to take our previously defined OSCLite structure and encode it in a byte array ``` uint8_t encodedMessage ```, which can then be put inside a packet.  

And finally, we just print out the HEX to serial, send the packet through a websocket and then cleanup and free the memory of the object.

```c
Serial.println(encodedLength);
Serial.println("\t End Encoded Message");
webSocket.sendBIN(encodedMessage, encodedLength);

// Cleanup
oscm.destroyOSCMessage(oscMsg);
```

