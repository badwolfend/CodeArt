// --------------------------------------------------------------------------
// OSC via WebSocket
// by Contra
// --------------------------------------------------------------------------
"use strict";

import { Server } from "ws";

import { WebSocketPort, readPacket } from "osc";
import { outlet, removeHandlers, addHandler, MESSAGE_TYPES } from "max-api";

const wss = new Server({ port: 12345 });

let webSocketPort;

// let tcpServer = new osc.TCPSocketPort({});
// tcpServer.open('127.0.0.1', 53000);
// tcpServer.send({ address: "/version", args: 1, }, '127.0.0.1', 53000);
// tcpServer.on('data', function(data) { console.log('DATA: ' + data); });

// let server = new osc.TCPSocketPort({});

// server.open('127.0.0.1', 57121); // change to remote host/port
// server.on('ready', () => {
//   console.log('ready');
// });

wss.on("connection", function connection(ws) {
  console.log("connection");

  let isConnected = true;

  webSocketPort = new WebSocketPort({
    // webSocketPort = new osc.TCPSocketPort({
    socket: ws,
  });

  ws.on("message", function incoming(message) {
    console.log("received : ", message);
    console.log(ws._socket.remoteAddress);
    //message data type is ArrayBuffer
    const msgParsed = readPacket(message, { metadata: true });
    console.log("received parsed : ", msgParsed);

    outlet("message", msgParsed);
  });

  ws.on("error", (err) => {
    console.log("error:", err);
  });

  ws.on("close", function stop() {
    removeHandlers("send");
    console.log("Connection closed");

    ws.terminate();

    isConnected = false;
  });

  // Handle the Max interactions here...
  addHandler("send1", (...args) => {
    //console.log("send args: " + args);
    if (webSocketPort && isConnected) {
      webSocketPort.send({
        address: "/max/led",
        args: [
          {
            type: "i",
            value: args[0],
          },
          {
            type: "i",
            value: args[1],
          },
          {
            type: "i",
            value: args[2],
          },
          {
            type: "i",
            value: args[3],
          },
        ],
      });
    }
  });
});

addHandler(MESSAGE_TYPES.ALL, (handled, ...args) => {
  if (!handled) {
    // Max.post('No client connected.')
    // just consume the message
  }
});
