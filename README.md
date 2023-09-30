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
Here's a simple example:


