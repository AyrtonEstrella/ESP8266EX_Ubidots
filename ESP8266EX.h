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
  ------------------------------------------------------------------------*/

#ifndef _ESP8266EX_H_
#define _ESP8266EX_H_

#include <Arduino.h>

#define ESP_RECEIVE_TIMEOUT   1000L
#define ESP_RESET_TIMEOUT     5000L
#define ESP_CONNECT_TIMEOUT  15000L
#define ESP_IPD_TIMEOUT     120000L

#define clearArray(__array) (memset(__array, NULL, sizeof(__array)))
const int _bufferSize = 512;
const int _defaultTimeout = 1000;
const int _defaultTries = 5;

typedef const __FlashStringHelper Fstr; // PROGMEM/flash-resident string
typedef const PROGMEM char        Pchr; // Ditto, kindasorta

#define defaultBootMarker "ready\r\n"
#define requestHeader    "> ARDUINO:\t"
#define responseHeader   "< DEVICE:\t"

// Subclassing Print makes debugging easier -- output en route to
// WiFi module can be duplicated on a second stream (e.g. Serial).
class ESP8266EX : public Print {
  public:
    const char STATION_MODE = 1;
    const char SINGLE_CONNECTION = 0;
    char replybuffer[_bufferSize];

    ESP8266EX(Stream *streamDevice = &Serial, Stream *streamDebug = NULL, int8_t r = -1);

    // SERIAL METHODS
    void flushInput();
    int indexof(char *string, char toSearch, int offset);
    int indexOfBackwards(char *string, char toSearch);
    boolean stringCompare(char *string1, char *string2);
    boolean stringFind(char *toSearch, char *toFind);
    void remove_spaces (char* str_trimmed, char* str_untrimmed);
    boolean readline(int timeout = _defaultTimeout, boolean multiline = false);
    boolean getReply(char *send, int timeout = _defaultTimeout);
    boolean sendCheckReply(char *send, char *reply, int timeout = _defaultTimeout);
    boolean expectReply(char *reply, int timeout = _defaultTimeout);
    boolean expectReplyMulti(char *reply, int timeout = _defaultTries);
    int findReply(char *reply, char *error1 = NULL, char *error2 = NULL,
                  char *error3 = NULL, char *error4 = NULL, char *error5 = NULL,
                  char *error6 = NULL, char *error7 = NULL, char *error8 = NULL,
                  char *error9 = NULL, char *error0 = NULL);

    // ESP8266 METHODS
    void    setBootMarker(char *newMarker = NULL);
    boolean hardReset(void);
    boolean softReset(void);
    boolean setModuleEcho(boolean enable);
    boolean getVersion();
    boolean setWifiMode(char mode);
    boolean setConnectionMode(char mode);
    boolean connectToAP(char *ssid, char *pass);
    boolean waitForIP();
    void    getIP();
    boolean connectTCP(char *hostname, char *port);
    boolean tcpConnected();
    boolean tcpSendRequest(int bytesToSend);
    boolean requestURL(char *url);
    void    closeAP(void);
    boolean closeTCP();

    // UBIDOTS METHODS
    void setUbidotsToken(char *token);
    void setUbidotsURL(char *url);
    void setDeviceName(char *device);
    void setVariablesNames(char *var1, char *var2, char *var3, char *var4);
    boolean ubidotsGetRequest(char *url, char *auth);
    boolean ubidotsPostRequest(char *value1, char *value2, char *value3, char *value4);
    boolean ubidotsPostResponse();

  private:
    Stream   *deviceStream,     // -> ESP8266, e.g. SoftwareSerial or Serial1
             *debugStream;      // -> _host, e.g. Serial
    int8_t   _resetPin;  // -1 if RST not connected
    boolean   _writing;
    virtual size_t write(uint8_t); // Because Print subclass

    char *_host;       // Non-NULL when TCP connection
    char _bootMarker[50]; // String indicating successful boot
    char *_ubidotsToken;
    char *_ubidotsUrl;
    char *_ubidotsDevice;
    char *_ubidotsVar1, *_ubidotsVar2, *_ubidotsVar3, *_ubidotsVar4;
};

#endif // _ESP8266EX_H_
