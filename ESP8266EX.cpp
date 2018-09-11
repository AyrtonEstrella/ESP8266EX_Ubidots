/*------------------------------------------------------------------------
  An Arduino library for the ESP8266 WiFi-serial bridge

  https://www.adafruit.com/product/2282

  The ESP8266 is a 3.3V device.  Safe operation with 5V devices (most
  Arduino boards) requires a logic-level shifter for TX and RX signals.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried and Phil Burgess for Adafruit Industries.
  MIT license, all text above must be included in any redistribution.

  Modified by Ayrton Estrella from IDW.
  This library was tested with this firmware version:
    AT version:0.25.0.0(Jun  5 2015 16:27:16)

  ------------------------------------------------------------------------*/

#include "ESP8266EX.h"

// Constructor
ESP8266EX::ESP8266EX(Stream *streamDevice, Stream *streamDebug, int8_t r) :
  _resetPin(r), _host(NULL), _writing(false) {

  deviceStream = streamDevice;
  debugStream = streamDebug;
  clearArray(replybuffer);
};

/*------------------------- LIBRARY INTERNAL METHODS -------------------------*/
// Override boot marker string, or pass NULL to restore default.
void ESP8266EX::setBootMarker(char *newMarker) {
  strcpy(_bootMarker, newMarker);
}

// Anything printed to the EPS8266 object will be split to both the WiFi
// and debugStream streams.  Saves having to print everything twice in debugStream code.
size_t ESP8266EX::write(uint8_t c) {
  if(debugStream) {
    if(!_writing) {
      debugStream->print(F(requestHeader));
      _writing = true;
    }
    debugStream->write(c);
  }
  return deviceStream->write(c);
}

// Remove spaces from a given string
void ESP8266EX::remove_spaces (char* str_trimmed, char* str_untrimmed) {
  while (*str_untrimmed != '\0') {
    if(!isspace(*str_untrimmed)) {
      *str_trimmed = *str_untrimmed;
      str_trimmed++;
    }
    str_untrimmed++;
  }
  *str_trimmed = '\0';
}

/*
  Locates a character within a char array. By default, searches from the
  beginning of the array, but can also start from a given index.
*/
int ESP8266EX::indexof(char *string, char toSearch, int offset) {
  int _length = strlen(string);
  for(int index = offset; index < _length; index ++)
    if(string[index] == toSearch)  return index;

  return -1;
}

// Returns the index of a specified char in a string from the end
int ESP8266EX::indexOfBackwards(char *string, char toSearch) {
  int _length = strlen(string);
  for(int index = _length - 1; index >= 0; index --)
    if(string[index] == toSearch)  return index;

  return -1;
}

// Return true if the strings are equal
boolean ESP8266EX::stringCompare(char *string1, char *string2) {
  return strcmp(string1, string2) == 0;
}

// Find a specified string inside a given string and returns true if found
boolean ESP8266EX::stringFind(char *toSearch, char *toFind) {
  return strstr(toSearch, toFind) != NULL;
}

// Empty the serial receive buffer
void ESP8266EX::flushInput() {
  while (deviceStream->available())
    deviceStream->read();
}

// Read a line from the serial buffer and stores it in [replybuffer]
boolean ESP8266EX::readline(int timeout, boolean multiline) {
  clearArray(replybuffer);

  int replyIndex = 0;
  while (timeout --) {
    if (replyIndex >= 254) break;

    while (deviceStream->available()) {
      char inByte = deviceStream->read();       // Lee el dato recibido

      if (inByte == '\r') continue;
      else if (inByte == '\n') {
        if (replyIndex == 0)              // Ignore first newline
          continue;
        else if (multiline == false) {
          timeout = 0;
          break;
        }
      }

      replybuffer[replyIndex] = inByte;   // Store the byte
      replyIndex++;
    }

    if (timeout == 0)
      return false;

    delay(2);
  }

  return true;
}


// Send a AT command and stores the response on [replybuffer]
boolean ESP8266EX::getReply(char *send, int timeout) {
  flushInput();

  debugStream->print(F(requestHeader));
  debugStream->println(send);

  deviceStream->println(send);

  boolean result = readline(timeout);
  debugStream->print(F(responseHeader));
  debugStream->println(replybuffer);

  return result;
}

//Send a command and verifies the reply
boolean ESP8266EX::sendCheckReply(char *send, char *reply, int timeout) {
  getReply(send, timeout);            // Envia el comando y guarda la respuesta en replybuffer

  return (stringCompare(replybuffer, reply));
}

// Looks if there is a specific reply on the [replybuffer]
boolean ESP8266EX::expectReply(char *reply, int timeout) {
  readline(timeout);
  debugStream->print(F(responseHeader));
  debugStream->println(replybuffer);

  return (stringCompare(replybuffer, reply));
}

/**
  Similar to expectReply but searches multiple lines until finding the
  specified reply
*/
boolean ESP8266EX::expectReplyMulti(char *reply, int timeout) {
  for (int i = 0; i < timeout; i++) {
    if (stringCompare(replybuffer, reply))
      return true;
    else if (strstr(replybuffer, reply) != NULL)
      return true;

    expectReply(NULL);
  }
  return false;
}

// Searches for the reply and evaluate up to 10 possible errors
int ESP8266EX::findReply(char *reply, char *error1 = NULL, char *error2 = NULL,
              char *error3 = NULL, char *error4 = NULL, char *error5 = NULL,
              char *error6 = NULL, char *error7 = NULL, char *error8 = NULL,
              char *error9 = NULL, char *error0 = NULL) {

    boolean result = false;
    for (int i = 0; i < _defaultTries*2; i++) {
      expectReply(NULL);

      if (stringFind(replybuffer, reply))
        return true;
      else if (stringFind(replybuffer, error1))
        return false;
      else if (stringFind(replybuffer, error2))
        return false;
      else if (stringFind(replybuffer, error3))
        return false;
      else if (stringFind(replybuffer, error4))
        return false;
      else if (stringFind(replybuffer, error5))
        return false;
      else if (stringFind(replybuffer, error6))
        return false;
      else if (stringFind(replybuffer, error7))
        return false;
      else if (stringFind(replybuffer, error8))
        return false;
      else if (stringFind(replybuffer, error9))
        return false;
      else if (stringFind(replybuffer, error0))
        return false;
    }
}

/*------------------------- CUSTOM METHODS ----------------------------*/

boolean ESP8266EX::setModuleEcho(boolean enable) {
  debugStream->println(F("\nSet module echo"));
  if (enable)
    return sendCheckReply("ATE1", "OK");
  else
    return sendCheckReply("ATE0", "OK");
}

// ESP8266 is reset by momentarily connecting RST to GND
boolean ESP8266EX::hardReset(void) {
  if(_resetPin < 0)
    return true;
  digitalWrite(_resetPin, LOW);
  pinMode(_resetPin, OUTPUT); // Open drain; reset -> GND
  delay(10);                  // Hold a moment
  pinMode(_resetPin, INPUT);  // Back to high-impedance pin state

  return expectReplyMulti(_bootMarker);    // Purge boot message from stream
}

// Soft reset.  Returns true if expected boot message received, else false.
boolean ESP8266EX::softReset(void) {
  debugStream->println(F("\nSoft reset..."));
  getReply("AT+RST");                     // Issue soft-reset command
  boolean response = expectReplyMulti(_bootMarker);    // Wait for boot message

  if (response == false)
    debugStream->println(F("Boot marker not found"));

  return response;
}

// Get the current firmware version
boolean ESP8266EX::getVersion() {
  debugStream->println(F("\nChecking firmware version..."));
  getReply("AT+GMR");                     // Issue soft-reset command
  boolean response = expectReplyMulti("OK");    // Wait for boot message

  if (response == false)
    debugStream->println(F("Error getting version"));

  return response;
}

// Set the wifi station mode
boolean ESP8266EX::setWifiMode(char mode) {
  debugStream->println(F("\nSet WiFi mode"));

  boolean success = false;
  if (mode == STATION_MODE)
    success = sendCheckReply("AT+CWMODE_DEF=1", "OK");

  if (success == false)
    debugStream->println(F("\nNo response"));

  return success;
}

// Set the wifi connection mode
boolean ESP8266EX::setConnectionMode(char mode) {
  debugStream->println(F("\nSet connection mode"));

  if (mode == SINGLE_CONNECTION)
    return sendCheckReply("AT+CIPMUX=0", "OK");

  return false;
}

// Connect to WiFi access point.
boolean ESP8266EX::connectToAP(char *ssid, char *pass) {
  debugStream->println(F("\nConnecting to WiFi AP"));

  print("AT+CWJAP_DEF=\"");
  print(ssid);
  print("\",\"");
  print(pass);
  println("\"");

  if (!expectReplyMulti("WIFI CONNECTED", 10)) {
    debugStream->println(F("Wifi connect error"));
    return false;
  }

  return waitForIP();
}

// Wait for IP message from module
boolean ESP8266EX::waitForIP() {
  if (expectReplyMulti("WIFI GOT IP")) {
    expectReplyMulti("OK");
    getIP();
    return true;
  }
  else {
    debugStream->println(F("Wifi connect error"));
    return false;
  }
}

// Gets the local IP
void ESP8266EX::getIP() {
  getReply("AT+CIFSR");
  expectReplyMulti("OK");
}

// Disconnect from WiFi AP
void ESP8266EX::closeAP(void) {
  debugStream->println(F("\nDisconnect from AP"));
  getReply("AT+CWQAP");
  expectReplyMulti("OK");
}

boolean ESP8266EX::connectTCP(char *hostname, char *port) {
  if (tcpConnected())
    return true;

  debugStream->println(F("\nStarting TCP connection"));

  print(F("AT+CIPSTART=\"TCP\",\""));
  print(hostname);
  print(F("\","));
  print(port);
  print(F("\r\n"));

  if (expectReplyMulti("CONNECT") == false)
    return false;

  _host = hostname;
  return true;
}


// Close the current TCP connection
boolean ESP8266EX::closeTCP() {
  debugStream->println(F("\nStarting TCP connection"));

  if (sendCheckReply("AT+CIPCLOSE", "CLOSE") == false)
    return false;
  if (findReply("OK", "ERROR") == false)
    return false;

  return true;
}

// Returns true if TCP connection is up
boolean ESP8266EX::tcpConnected() {
  debugStream->println(F("\nChecking TCP status..."));
  if (sendCheckReply("AT+CIPSTATUS", "STATUS:3")) {
    debugStream->println(F("Connected"));
    return true;
  }
  else {
    debugStream->println(F("Not connected"));
    return false;
  }
}

// Send the AT+CIPSEND command with the request length
boolean ESP8266EX::tcpSendRequest(int bytesToSend) {
  char sendCmd[15];
  sprintf(sendCmd, "AT+CIPSEND=%u", bytesToSend);
  getReply(sendCmd);

  if (findReply("> ") == false) {
    debugStream->println(F("ERROR! CIPSEND failed"));
    return false;
  }

  return true;
}

// Requests page from currently-open TCP connection
boolean ESP8266EX::requestURL(char *url) {
  char tcpRequest[150];
  char sendCmd[15];

  sprintf(tcpRequest, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", url, _host);
  sprintf(sendCmd, "AT+CIPSEND=%u", strlen(tcpRequest));

  getReply(sendCmd);
  if (findReply("> ") == false) {
    debugStream->println(F("ERROR! TCP send failed"));
    return false;
  }

  getReply(tcpRequest);
  boolean success;
  success = expectReplyMulti("SEND OK");
  success = findReply("+IPD");
  success = findReply("+IPD");

  if (success == false) {
    debugStream->println(F("ERROR! TCP send failed"));
    return false;
  }

  return true;
}

/*------------------------- UBIDOTS METHODS ----------------------------*/
void ESP8266EX::setUbidotsToken(char *token) {
  _ubidotsToken = token;
}

void ESP8266EX::setUbidotsURL(char *url) {
  _ubidotsUrl = url;
}

void ESP8266EX::setDeviceName(char *device) {
  _ubidotsDevice = device;
}

void ESP8266EX::setVariablesNames(char *var1, char *var2, char *var3, char *var4) {
  _ubidotsVar1 = var1;
  _ubidotsVar2 = var2;
  _ubidotsVar3 = var3;
  _ubidotsVar4 = var4;
}

// Send a GET request to Ubidots server
boolean ESP8266EX::ubidotsGetRequest(char *url, char *auth) {
  char tcpRequest[150];
  char sendCmd[15];

  sprintf(tcpRequest, "GET %s%s HTTP/1.1\r\nHost: %s\r\n\r\n", url, auth, _host);
  sprintf(sendCmd, "AT+CIPSEND=%u", strlen(tcpRequest));

  getReply(sendCmd);
  if (findReply("> ") == false) {
    debugStream->println(F("ERROR! TCP send failed"));
    return false;
  }

  getReply(tcpRequest);
  boolean success;
  success = expectReplyMulti("SEND OK");
  success = findReply("+IPD");
  success = findReply("+IPD");

  if (success == false) {
    debugStream->println(F("ERROR! TCP send failed"));
    return false;
  }

  return true;
}

// Send a POST request to Ubidots server
boolean ESP8266EX::ubidotsPostRequest(char *value1, char *value2, char *value3, char *value4) {
  char variableString[60];
  sprintf(variableString, "{\"%s\": %s, \"%s\": %s, \"%s\": %s, \"%s\": %s}",
                          _ubidotsVar1, value1, _ubidotsVar2, value2,
                          _ubidotsVar3, value3, _ubidotsVar4, value4);

  int requestLength = 86 + strlen(_ubidotsUrl) + strlen(_ubidotsDevice) +
                          strlen(_ubidotsToken) + strlen(_host) +
                          strlen(variableString);

  if (tcpSendRequest(requestLength) == false)
    return false;

  print(F("POST "));
  print(_ubidotsUrl);
  print(_ubidotsDevice);
  print(F("/?token="));
  print(_ubidotsToken);
  print(F(" HTTP/1.1\r\nHost: "));
  println(_host);
  print(F("Content-Type: application/json\r\nContent-Length: "));
  println(strlen(variableString));
  println(F(""));
  print(variableString);
  debugStream->println(F(""));

  return ubidotsPostResponse();
}

// Get the post request status response
boolean ESP8266EX::ubidotsPostResponse() {
  boolean success;
  success = expectReplyMulti("SEND OK");
  success = expectReplyMulti("status_code", 25);

  if (success == false) {
    debugStream->println(F("ERROR! TCP send failed"));
    return false;
  }
}
