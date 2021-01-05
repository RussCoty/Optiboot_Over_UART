/* This sketch will allow programming of an Atmega 328 or similar that has optiboot installed, over serial UART
    Currently set up for Teensy 3.6 to proigram a 328AU over Serial prot 2 (Teensy 3.6 pins 9 and 10) connected
    to the Serial UART on the 328 directly.
*/

#define syncTimeout       1000
#define responseTimeout   1000

/* STK500 constants list, from AVRDUDE */
#define STK_OK              0x10
#define STK_FAILED          0x11  // Not used
#define STK_UNKNOWN         0x12  // Not used
#define STK_NODEVICE        0x13  // Not used
#define STK_INSYNC          0x14  // ' '
#define STK_NOSYNC          0x15  // Not used
#define ADC_CHANNEL_ERROR   0x16  // Not used
#define ADC_MEASURE_OK      0x17  // Not used
#define PWM_CHANNEL_ERROR   0x18  // Not used
#define PWM_ADJUST_OK       0x19  // Not used
#define CRC_EOP             0x20  // 'SPACE'
#define STK_GET_SYNC        0x30  // '0'
#define STK_GET_SIGN_ON     0x31  // '1'
#define STK_SET_PARAMETER   0x40  // '@'
#define STK_GET_PARAMETER   0x41  // 'A'
#define STK_SET_DEVICE      0x42  // 'B'
#define STK_SET_DEVICE_EXT  0x45  // 'E'
#define STK_ENTER_PROGMODE  0x50  // 'P'
#define STK_LEAVE_PROGMODE  0x51  // 'Q'
#define STK_CHIP_ERASE      0x52  // 'R'
#define STK_CHECK_AUTOINC   0x53  // 'S'
#define STK_LOAD_ADDRESS    0x55  // 'U'
#define STK_UNIVERSAL       0x56  // 'V'
#define STK_PROG_FLASH      0x60  // '`'
#define STK_PROG_DATA       0x61  // 'a'
#define STK_PROG_FUSE       0x62  // 'b'
#define STK_PROG_LOCK       0x63  // 'c'
#define STK_PROG_PAGE       0x64  // 'd'
#define STK_PROG_FUSE_EXT   0x65  // 'e'
#define STK_READ_FLASH      0x70  // 'p'
#define STK_READ_DATA       0x71  // 'q'
#define STK_READ_FUSE       0x72  // 'r'
#define STK_READ_LOCK       0x73  // 's'
#define STK_READ_PAGE       0x74  // 't'
#define STK_READ_SIGN       0x75  // 'u'
#define STK_READ_OSCCAL     0x76  // 'v'
#define STK_READ_FUSE_EXT   0x77  // 'w'
#define STK_READ_OSCCAL_EXT 0x78  // 'x'

#define DEBUG(x)        Serial.print(x)
#define DEBUGLN(x)      Serial.println(x)

#define rstPin 32 //this pin is connected to the 328 just to make sure we are going into programming mode in optiboot correctly. 

const int startButton = 28; //This Button can be used to start the process
uint8_t respBuf[200]; // FIXME magic numbers!
uint8_t cmdBuf[200];  // FIXME Check buffer sizes and ideally don't use






void setup() {

  Serial.begin (115200);
  Serial2.begin(115200);
  pinMode (startButton, INPUT);
  delay(500);
  DEBUGLN ("SETUP RUN");
  // Using the Reset pin
  pinMode (rstPin, INPUT);
  digitalWrite (rstPin, HIGH);

}

// FIXME This will make more sense to be in a different file
// The actual image to flash. This can be copy-pasted as-is from a
// .hex file. If you do, replace all lines below starting with a
// colon, but make sure to keep the start and end markers {R"( and
// )"} in place.
const size_t chipsize = 32767 - 1024; // Total flash AVAILABLE in the 328, so not inc. bootloader - FIXME Set this to the right value too!!!
const size_t image_page_size = 128;
const size_t image_size = 19000; // FIXME Set this to a useful value, and it only needs to be as big as the HEX file is (rounded up to next 128 bytes maybe?)
// FIXME Replace the hex below with an actual hex file (without the bootloader)!!!
uint8_t image_hexcode[image_size] = { R"(
:107E0000112494B714BE892F8D7011F0892FDED004
:107E100085E08093810082E08093C00088E18093B8
:107E2000C10086E08093C20080E18093C4008EE0B0
:0A7FD000E7DFEE27FF270994020601
:00000001FF
    )"
                                    };

uint8_t pageBuffer[image_page_size];


void loop() {


  //  // Wait for start button to be pressed
  //  DEBUGLN("PRESS SHIFT");
  //  while (digitalRead (startButton) == HIGH) {
  //    // DEBUG(".");
  //    delay(100);
  //  }

  // RESET THE TARGET WITH A wire connecteed to rstPin (currently 32) on the programmer.
  // Just to test things
  digitalWrite (rstPin, LOW);
  delay (2);
  digitalWrite (rstPin, HIGH);
  DEBUGLN ("Target Reset"); 

  // Get in sync first
  bool inSync = false;
  uint32_t syncstart = millis();

  while (!inSync && (millis() - syncstart < syncTimeout))
  {
    DEBUGLN("Trying to get in sync...");
    // We should flush the serial buffer to clear out any garbage
    // FIXME Maybe Serial.clear() on the Teensy would be better than this
    // FIXME while loop?
    DEBUGLN ("Next we will purge the Programmer's UART buffer");
    while (Serial2.available())
    {
      //Serial.println(Serial2.read());   // read it and send it out Serial (USB)

      (void)Serial2.read(); // Read in any data and throw it away
    }

    cmdBuf[0] = STK_GET_SYNC;
    sendCommand(cmdBuf, 1);

    if (getOKResponse(responseTimeout)) {
      DEBUGLN("We're in sync!!!");
      inSync = true;
    }
  }

  if (inSync)
  {
    // Now we can reprogram the 328

    // Enter program mode
    cmdBuf[0] = STK_ENTER_PROGMODE;
    sendCommand(cmdBuf, 1);

    if (getOKResponse(responseTimeout))
    {
      // Now we can load the program!
      uint8_t currentAddress = 0x0000;
      uint8_t* hextext = image_hexcode;
      while (currentAddress < chipsize && hextext) {
        DEBUG("Writing address $"); DEBUGLN(currentAddress);
        //byte *hextextpos = readImagePage (hextext, currentAddress, pagesize, pageBuffer);
        byte *hextextpos = readImagePage (hextext, currentAddress, image_page_size, pageBuffer);


        boolean blankpage = true;
        for (uint8_t i = 0; i < image_page_size; i++) {
          if (pageBuffer[i] != 0xFF) blankpage = false;
        }
        if (! blankpage) {
          if (! sendPage(pageBuffer, currentAddress, image_page_size))
            DEBUG("Flash programming failed");
        }
        hextext = hextextpos;
        currentAddress += image_page_size;
      }
      //*
      // We should now verify that it's been written
      // Start again at the beginning of the hex
      currentAddress = 0;// This was 0x00000
      hextext = image_hexcode;
      while (currentAddress < chipsize && hextext) {
        DEBUG("Verifying address $"); DEBUGLN(currentAddress);
        byte *hextextpos = readImagePage (hextext, currentAddress, image_page_size, pageBuffer);

        boolean blankpage = true;
        for (uint8_t i = 0; i < image_page_size; i++) {
          if (pageBuffer[i] != 0xFF) blankpage = false;
        }
        if (! blankpage) {
          // Read the flash back from the 328...
          uint8_t verifyBuffer[image_page_size];
          if (! readPage(verifyBuffer, currentAddress, image_page_size))
          {
            if (memcmp(verifyBuffer, pageBuffer, image_page_size) != 0)
            {
              DEBUG("Flash verification failed at ");
              DEBUGLN(currentAddress);
            }
          }
        }
        hextext = hextextpos;
        currentAddress += image_page_size;
      }

      // We've flashed and verified, so now leave program mode
      cmdBuf[0] = STK_LEAVE_PROGMODE;
      sendCommand(cmdBuf, 1);

      if (getOKResponse(responseTimeout))
      {
        DEBUGLN("Woohoo!  All is good.");
      }
      else
      {
        DEBUGLN("Dunno why we couldn't leave program mode :shrug:");
      }
      // Now we're done, we need to reboot the 328
      // Looking at Optiboot, if we get a command we can't verify, it'll
      // reboot with the watchdog... so sending two OKs will mean it
      // fails the VerifySpace check and reboots...
      cmdBuf[0] = STK_OK;
      cmdBuf[1] = STK_OK;
      sendCommand(cmdBuf, 2);
    }
    else
    {
      DEBUGLN("Failed to enter program mode");
    }
  }
  else
  {
    DEBUGLN("FIXME Tell the user it's timed out, did they hit the reset in time?");
    DEBUGLN("");
    DEBUGLN("");
    DEBUGLN("");
    DEBUGLN("");    
  }

  // FIXME This is the end of the program...
  //  DEBUGLN("Reset Programmer - End of Code - No looping today");
  //  while (1)
  //  {
  //
  //    };



}


//End of Main loop.

// Functions below this point
bool getOKResponse(uint32_t timeout)
{
  DEBUGLN("We are in getOKResponse function");
  int ret = getResponse(respBuf, 2, responseTimeout);
  if (ret == 2)
  {
    if ((respBuf[0] == STK_INSYNC) && (respBuf[1] == STK_OK))
    {
      return true;
      DEBUGLN("getOKResponse function retuns TRUE");
    }
  }
  else {
    return false;
    DEBUGLN("getOKResponse function retuns FALSE");
  }
}

int getResponse(uint8_t* buffer, int bufferLen, uint32_t timeout)
{
  DEBUGLN("We are in getResponse function");
  uint32_t reponsestart = millis();
  while ((millis() - reponsestart < timeout)  && (Serial2.available() < bufferLen))
  {
    delay(1);

  }
  DEBUGLN("End of checking in getResponse function");

  if ((millis() - reponsestart) < timeout) //testing this funciton ...debug not showing which is strnage
  {
    // We've got some data

    DEBUGLN("getResponse function retuned a value");
    return Serial2.readBytes(buffer, bufferLen);
  }
  else
  {
   
    DEBUGLN("getResponse function retuned a -1"); 
    return -1;
  }
}

// Sends a short command and tacks on the CRC_EOP
void sendCommand(uint8_t* cmd, size_t cmdLen)
{

  DEBUG("Sending ");
  Serial.print(cmdLen);// Debug only takes 1 argument so Serial is used here
  DEBUGLN(" bytes to target, plus CRC_EOP");
  Serial2.write(cmd, cmdLen);
  Serial2.write(CRC_EOP);
}

// Sends a command with data and tacks on the CRC_EOP
void sendCommandPlusData(uint8_t* cmd, size_t cmdLen, uint8_t* data, size_t dataLen)
{
  DEBUGLN("Sending some data to target, plus CRC_EOP");
  Serial2.write(cmd, cmdLen);
  Serial2.write(data, dataLen);
  Serial2.write(CRC_EOP);
}

// Basically, write the pagebuff (with image_page_size bytes in it) into page $pageaddr
boolean sendPage (byte * pagebuff, uint16_t pageaddr, uint8_t pagesize) {
  // Tell the 328 where this page goes
  cmdBuf[0] = STK_LOAD_ADDRESS;
  cmdBuf[1] = pageaddr & 0xff;
  cmdBuf[2] = (pageaddr & 0xff00) >> 8;
  sendCommand(cmdBuf, 3);

  if (getOKResponse(responseTimeout))
  {
    // Now we can send the page to write
    cmdBuf[0] = STK_PROG_PAGE;
    // There are 3 bytes of data expected by Optiboot before we get
    // to the actual page data.  It /looks/ like there's a byte that
    // gets ignored and then two bytes of pagesize, but given that
    // pagesize is 128, the second of those (the high byte) also gets
    // ignored.  FIXME We could check the stkv500 protocol to be sure...
    cmdBuf[1] = STK_UNKNOWN;
    cmdBuf[2] = pagesize;
    cmdBuf[3] = STK_UNKNOWN;
    sendCommandPlusData(cmdBuf, 4, pagebuff, pagesize);
    if (getOKResponse(responseTimeout))
    {
      DEBUGLN("Page written okay");
      return true;
    }
  }
  else
  {
    DEBUGLN("Something humorous.  Failed to load address");
  }
  return false;
}

// Read the page at pageaddr from the 328 and store into pagebuff
// Return true if successful, else false
boolean readPage (byte * pagebuff, uint16_t pageaddr, uint8_t pagesize) {
  // Tell the 328 where this page goes
  cmdBuf[0] = STK_LOAD_ADDRESS;
  cmdBuf[1] = pageaddr & 0xff;
  cmdBuf[2] = (pageaddr & 0xff00) >> 8;
  sendCommand(cmdBuf, 3);

  if (getOKResponse(responseTimeout))
  {
    // Now we can read the page from the 328
    cmdBuf[0] = STK_READ_PAGE;
    // There are 3 bytes of data expected by Optiboot before we get
    // to the actual page data.  It /looks/ like there's a byte that
    // gets ignored and then two bytes of pagesize, but given that
    // pagesize is 128, the second of those (the high byte) also gets
    // ignored.  FIXME We could check the stkv500 protocol to be sure...
    cmdBuf[1] = STK_UNKNOWN;
    cmdBuf[2] = pagesize;
    cmdBuf[3] = STK_UNKNOWN;
    sendCommandPlusData(cmdBuf, 4, pagebuff, pagesize);
    // The response will be INSYNC<pagesize bytes of data>STK_OK
    if (getResponse(pagebuff, 1, responseTimeout) == 1)
    {
      // Check that it's the INSYNC byte
      if (pagebuff[0] == STK_INSYNC)
      {
        // Now we can read in the page data
        if (getResponse(pagebuff, pagesize, responseTimeout) == pagesize)
        {
          // Check we've got the OK at the end
          uint8_t ok[1];
          if (getResponse(ok, 1, responseTimeout) == 1)
          {
            if (ok[0] == STK_OK)
            {
              // It's all ok!  Finally.
              return true;
            }
          }
        }
        DEBUGLN("Something else humorous.  Didn't get pagesize bytes read");
      }
      DEBUGLN("Another humorous thing.  We're not in sync!");
    }
  }
  else
  {
    DEBUGLN("Something humorous.  Failed to load address");
  }
  return false;
}

/*
   readImagePage

   Read a page of intel hex image from a string in pgm memory.
*/

// Returns number of bytes decoded
byte *readImagePage (byte *readhextext, uint16_t readpageaddr, uint8_t readpagesize, byte *readpage)
{

  uint16_t len;
  uint8_t page_idx = 0;
  byte *beginning = readhextext;

  byte b, cksum = 0;

  //Serial.print("page size = "); Serial.println(readpagesize, DEC);

  // 'empty' the page by filling it with 0xFF's
  for (uint8_t i = 0; i < readpagesize; i++)
    readpage[i] = 0xFF;

  while (1) {
    uint16_t lineaddr;

    // Strip leading whitespace
    byte c;
    do {
      c = pgm_read_byte(readhextext++);
    } while (c == ' ' || c == '\n' || c == '\t');

    // read one line!
    if (c != ':') {
      DEBUGLN("No colon?");
      break;
    }
    // Read the byte count into 'len'
    len = hexton(pgm_read_byte(readhextext++));
    len = (len << 4) + hexton(pgm_read_byte(readhextext++));
    cksum = len;

    // read high address byte
    b = hexton(pgm_read_byte(readhextext++));
    b = (b << 4) + hexton(pgm_read_byte(readhextext++));
    cksum += b;
    lineaddr = b;

    // read low address byte
    b = hexton(pgm_read_byte(readhextext++));
    b = (b << 4) + hexton(pgm_read_byte(readhextext++));
    cksum += b;
    lineaddr = (lineaddr << 8) + b;

    if (lineaddr >= (readpageaddr + readpagesize)) {
      return beginning;
    }

    b = hexton(pgm_read_byte(readhextext++)); // record type
    b = (b << 4) + hexton(pgm_read_byte(readhextext++));
    cksum += b;
    //Serial.print("Record type "); Serial.println(b, HEX);
    if (b == 0x1) {
      // end record, return nullptr to indicate we're done
      readhextext = nullptr;
      break;
    }
#if VERBOSE
    Serial.print("\nLine address =  0x"); Serial.println(lineaddr, HEX);
    Serial.print("Page address =  0x"); Serial.println(readpageaddr, HEX);
#endif
    for (byte i = 0; i < len; i++) {
      // read 'n' bytes
      b = hexton(pgm_read_byte(readhextext++));
      b = (b << 4) + hexton(pgm_read_byte(readhextext++));

      cksum += b;
#if VERBOSE
      Serial.print(b, HEX);
      Serial.write(' ');
#endif

      readpage[page_idx] = b;
      page_idx++;

      if (page_idx > readpagesize) {
        DEBUGLN("Too much code");
        break;
      }
    }
    b = hexton(pgm_read_byte(readhextext++));  // chxsum
    b = (b << 4) + hexton(pgm_read_byte(readhextext++));
    cksum += b;
    if (cksum != 0) {
      DEBUGLN("Bad checksum: ");
      Serial.print(cksum, HEX);
    }
    if (pgm_read_byte(readhextext++) != '\n') {
      DEBUGLN("No end of line");
      break;
    }
#if VERBOSE
    Serial.println();
    Serial.println(page_idx, DEC);
#endif
    if (page_idx == readpagesize)
      break;
  }
#if VERBOSE
  Serial.print("\n  Total bytes read: ");
  Serial.println(page_idx, DEC);
#endif
  return readhextext;
}

/*
   hexton
   Turn a Hex digit (0..9, A..F) into the equivalent binary value (0-16)
*/
byte hexton (byte h)
{
  if (h >= '0' && h <= '9')
    return (h - '0');
  if (h >= 'A' && h <= 'F')
    return ((h - 'A') + 10);
  DEBUGLN("Bad hex digit!");
  return (h);
}
