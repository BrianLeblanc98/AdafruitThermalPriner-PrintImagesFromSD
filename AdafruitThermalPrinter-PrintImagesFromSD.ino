#include <SD.h>
#include <SPI.h>
#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"

// Printer pins
#define RX_PIN 5
#define TX_PIN 6

// SD card pin
#define CS_PIN 10

// Printer baud rate
#define PRINTER_RATE 19200

// Serial monitor baud rate
#define MONITOR_RATE 9600

// Declare printer
SoftwareSerial mySerial(RX_PIN, TX_PIN);
Adafruit_Thermal printer(&mySerial);

void setup() {
  Serial.begin(MONITOR_RATE);
  initSD();
  initPrinter();

  File image = SD.open("test.bmp");
  if(image){
    printBitmapFromFile(image);
    image.close();
  }
}

void loop(){}

void printBitmapFromFile(File file){
  /*
  Information about BMPs from: https://en.wikipedia.org/wiki/BMP_file_format
  In the header of the BMP file, we get informarion about the file.
  At byte 10 we have 4 bytes that state the start position of the pixel array.
  At byte 18 we have 4 bytes that state the width of the image in pixels.
  At byte 22 we have 4 bytes that state the height of the image in pixels.

  The pixel array stores the image botom to top, so we must go up the rows from bottom to top.
  It's also important to note that the printer expects the bytes to be inverted
  */
  uint32_t startOffset;
  uint32_t width;
  uint32_t height;
  file.seek(10);
  file.read(&startOffset, sizeof(startOffset));
  file.seek(18);
  file.read(&width, sizeof(width));
  file.read(&height, sizeof(height));

  // We print each row of bytes separately, starting from the bottom of the pixel array
  int byteWidth = width/8;
  long pixelArrayEnd = height*byteWidth + startOffset;
  
  for(int y=0; y<height; y++){
    file.seek(pixelArrayEnd - y*48);
    uint8_t data[byteWidth];
    for(int x=0; x<byteWidth; x++){
      uint8_t b = file.read() ^ 0xff;
      data[x] = b;
    }
    printer.printBitmap(width, 1, data, false);
  }
  printer.feed(2);
}

void initSD(){
  Serial.println("Initializing SD card");
  pinMode(CS_PIN, OUTPUT);

  if (SD.begin()){
    Serial.println("SD card is ready to use.");
  } else {
    Serial.println("SD card initialization failed");
  }
}

void initPrinter(){
  mySerial.begin(PRINTER_RATE);
  printer.begin();
}
