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
uint8_t respBuf[128]; // FIXME magic numbers!
uint8_t cmdBuf[128];  // FIXME Check buffer sizes and ideally don't use






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
const size_t image_size = 31616; // FIXME Set this to a useful value, and it only needs to be as big as the HEX file is (rounded up to next 128 bytes maybe?)
// FIXME Replace the hex below with an actual hex file (without the bootloader)!!!
uint8_t image_hexcode[image_size] = { R"(
:100000000C9435000C945D000C945D000C945D0024
:100010000C945D000C945D000C945D000C945D00EC
:100020000C945D000C945D000C945D000C945D00DC
:100030000C945D000C945D000C945D000C945D00CC
:100040000C94E9010C945D000C94B7010C9491019F
:100050000C945D000C945D000C945D000C945D00AC
:100060000C945D000C945D00EA0211241FBECFEFDA
:10007000D8E0DEBFCDBF11E0A0E0B1E0EEE3F6E0F6
:1000800002C005900D92AC31B107D9F721E0ACE187
:10009000B1E001C01D92A23CB207E1F710E0C5E358
:1000A000D0E004C02197FE010E941703C433D1079A
:1000B000C9F70E9433020C941D030C940000AF9208
:1000C000BF92CF92DF92EF92FF920F931F93CF9345
:1000D000DF936C017B018B01040F151FEB015E01A7
:1000E000AE18BF08C017D10759F06991D601ED913C
:1000F000FC910190F081E02DC6010995892B79F7DB
:10010000C501DF91CF911F910F91FF90EF90DF908C
:10011000CF90BF90AF900895FC01538D448D252F53
:1001200030E0842F90E0821B930B541710F0CF9691
:10013000089501970895FC01918D828D981761F0C3
:10014000A28DAE0FBF2FB11D5D968C91928D9F5FDA
:100150009F73928F90E008958FEF9FEF0895FC01B9
:10016000918D828D981731F0828DE80FF11D858D6C
:1001700090E008958FEF9FEF0895FC01918D228DFF
:10018000892F90E0805C9F4F821B91098F73992784
:10019000089585E291E00E94BD0021E0892B09F4D9
:1001A00020E0822F089580E090E0892B29F00E94C2
:1001B000C90081110C9400000895FC01A48DA80FC2
:1001C000B92FB11DA35ABF4F2C91848D90E0019699
:1001D0008F739927848FA689B7892C93A089B189B9
:1001E0008C91837080648C93938D848D981306C05A
:1001F0000288F389E02D80818F7D80830895EF92BE
:10020000FF920F931F93CF93DF93EC0181E0888FD0
:100210009B8D8C8D98131AC0E889F989808185FFA0
:1002200015C09FB7F894EE89FF896083E889F98942
:1002300080818370806480839FBF81E090E0DF9144
:10024000CF911F910F91FF90EF900895F62E0B8D97
:1002500010E00F5F1F4F0F731127E02E8C8D8E1152
:100260000CC00FB607FCFACFE889F989808185FFB9
:10027000F5CFCE010E94DD00F1CFEB8DEC0FFD2F0D
:10028000F11DE35AFF4FF0829FB7F8940B8FEA8974
:10029000FB8980818062CFCFCF93DF93EC01888D83
:1002A0008823B9F0AA89BB89E889F9898C9185FDF1
:1002B00003C0808186FD0DC00FB607FCF7CF8C917F
:1002C00085FFF2CF808185FFEDCFCE010E94DD005A
:1002D000E9CFDF91CF9108953FB7F8948091210144
:1002E00090912201A0912301B091240126B5A89BF1
:1002F00005C02F3F19F00196A11DB11D3FBFBA2FB8
:10030000A92F982F8827BC01CD01620F711D811D77
:10031000911D43E0660F771F881F991F4A95D1F7FB
:1003200008951F920F920FB60F9211242F933F93AF
:100330004F935F936F937F938F939F93AF93BF93ED
:10034000EF93FF9385E291E00E94DD00FF91EF9132
:10035000BF91AF919F918F917F916F915F914F91DD
:100360003F912F910F900FBE0F901F9018951F92E5
:100370000F920FB60F9211242F938F939F93EF93A9
:10038000FF93E0913501F09136018081E0913B01CE
:10039000F0913C0182FD1BC0908180913E018F5FF6
:1003A0008F7320913F01821741F0E0913E01F0E010
:1003B000EB5DFE4F958F80933E01FF91EF919F91F2
:1003C0008F912F910F900FBE0F901F9018958081E5
:1003D000F4CF1F920F920FB60F9211242F933F93D9
:1003E0008F939F93AF93BF9380911D0190911E01B6
:1003F000A0911F01B091200130911C0126E0230F34
:100400002D3758F50296A11DB11D20931C01809334
:100410001D0190931E01A0931F01B09320018091B4
:10042000210190912201A0912301B0912401019614
:10043000A11DB11D8093210190932201A09323015E
:10044000B0932401BF91AF919F918F913F912F91D4
:100450000F900FBE0F901F90189529E8230F039659
:10046000A11DB11DD2CF789484B5826084BD84B5BE
:10047000816084BD85B5826085BD85B5816085BD9F
:1004800080916E00816080936E0010928100809157
:1004900081008260809381008091810081608093DF
:1004A0008100809180008160809380008091B10004
:1004B00084608093B1008091B00081608093B0002F
:1004C00080917A00846080937A0080917A008260C3
:1004D00080937A0080917A008E7F80937A00809159
:1004E0007A00806880937A001092C100E091350113
:1004F000F091360182E08083E0913101F091320188
:100500001082E0913301F091340188E080831092F1
:100510003D01E0913901F0913A0186E08083E0915C
:100520003701F0913801808180618083E09137014B
:10053000F0913801808188608083E0913701F091EB
:100540003801808180688083E0913701F091380123
:1005500080818F7D8083C0E0D0E046E050E062E1A2
:1005600071E085E291E00E945F0042E050E069E1C5
:1005700071E085E291E00E945F000E946C014B01F6
:100580005C018AE0C82ED12CE12CF12C0E946C0178
:10059000681979098A099B09683E734081059105AC
:1005A000A8F321E0C21AD108E108F10888EE880E0C
:1005B00083E0981EA11CB11CC114D104E104F10414
:1005C00029F7209751F20E94C900882331F20E9436
:1005D0000000C3CFE5E2F1E01382128288EE93E0DF
:1005E000A0E0B0E084839583A683B78384E091E0A4
:1005F0009183808385EC90E09587848784EC90E0FC
:100600009787868780EC90E0918B808B81EC90E0DF
:10061000938B828B82EC90E0958B848B86EC90E0C0
:10062000978B868B118E128E138E148E0895EE0F7B
:0E063000FF1F0590F491E02D0994F894FFCF80
:10063E0000000000FF005F008C004C01BD009B001D
:0C064E00AF00746573742031000D0A00C9
:00000001FF
)" };

uint8_t pageBuffer[image_page_size];


void loop() {

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
      DEBUGLN("");
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
      DEBUGLN("Program mode entered and confirmed");
      // Now we can load the program!
      int currentAddress = 0x0000;
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
    DEBUGLN("Reset Programmer - End of Code - No looping today");
    while (digitalRead (startButton) == HIGH)
    {
  
      };



}


//End of Main loop.

// Functions below this point
bool getOKResponse(uint32_t timeout)
{
  DEBUGLN("We are in getOKResponse function");
  int ret = getResponse(respBuf, 2, responseTimeout);
  if (ret == 2)
  {
    DEBUGLN("getOKResponse function sees 2 bytes");
    if ((respBuf[0] == STK_INSYNC) && (respBuf[1] == STK_OK))
    {
      DEBUGLN("getOKResponse function retuns TRUE");
      return true;
    }
  }
  else {
    DEBUGLN("getOKResponse function retuns FALSE");
    return false;
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
  //DEBUGLN("End of checking in getResponse function");

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
  DEBUG(" bytes to target, plus CRC_EOP. Command was: ");
  Serial.write (*cmd);// Debug only takes 1 argument so Serial is used here
  DEBUGLN("");
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
