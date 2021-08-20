#include <SoftwareSerial.h>
#include <Keypad.h>

// Length of password + 1 for null character
#define Password_Length 7
// Character to hold password input
char Data[Password_Length];
// Password
char Master[Password_Length] = "967830";

// Making while lock
bool entering = false;

// Pin connected to lock relay input
int lockOutput = 12;

// Counter for character entries
byte data_count = 0;

// Character to hold key input
char customKey;

// Constants for row and column sizes
const byte ROWS = 4;
const byte COLS = 3;


// array to represent keys on keypad
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

// connections to arduino
byte rowPins[ROWS] = {10, 9, 8, 7};
byte colPins[COLS] = {6, 5, 4};

// create keypad object
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// choose two pins for software serial
SoftwareSerial rSerial(2, 3); // RZ, TX

// For SparkFun's tags, we will receive 16 bytes on every
// tag read, but throw four away. The 13th space will always
// be 0, since proper strings in Arduino end with 0

// These constants hold the total tag length (tagLen) and
// the length of the part we want to keep (idLen),
// plus the total number of tags we want to check against (kTags)
const int tagLen = 16;
const int idLen = 13;
const int kTags = 4;

char knownTags[kTags][idLen] = {
             "AAAAAAAAAAAA", // 67009AFF1A18 is card id
             "444444444444",
             "555555555555",
             "7A005B0FF8D6"
};

char newTag[idLen];

void setup() {
  // Starts the hardware and software serial ports
  Serial.begin(9600);
  rSerial.begin(9600);

  // Set lockOutput as an OUTPUT pin
  pinMode(lockOutput, OUTPUT);
}

void loop() {
  // Counter for the newTag array
  int i = 0;
  // Variable to hold each byte read from the serial buffer
  int readByte;
  // Flag so we know when a tag is over
  boolean tag = false;

  // This makes sure the whole tag is in the serial buffer before
  // reading, the Arduino can read faster than the ID module can deliver
  if (rSerial.available() == tagLen) {
    tag = true;
  }

  if (tag == true) {
    while (rSerial.available()) {
      //Take each byte out of the serial buffer, one at a time      
      readByte = rSerial.read();

    /* THis will skip the first byte (2, STX, start of text) and the last three,
     *  ASCII  13, CR/carriage return, ASCII 10, LF/linefeed, and ASCII 3, ETX/end of
     *  text, leaving only the unique part of the tag string, It puts the byte into
     *  the first space in the array, then steps ahead one spot */
     if (readByte != 2 && readByte!= 13 && readByte != 10 && readByte != 3) {
      newTag[i] = readByte;
      i++;
     }

     // If we see ASCII 3, ETX, the tag is over
     if (readByte == 3) {
      tag = false;
     }
     
    }
  }


  // don't dp anything if the newTag array is full of zeros
  if (strlen(newTag)==0) {
    return;
  }

  else {
    int total = 0;

    for (int ct=0; ct < kTags; ct++){
      total += checkTag(newTag, knownTags[ct]);
    }

    // If newTag matched any of the tags
    // we checked against, total will be 1
    if (total > 0) {
      entering = true;

      Serial.println("Enter Password");

      while (entering == true) {

        //get key value if pressed
        char customKey = customKeypad.getKey();
  
        if (customKey) {
          // Enter keypress into array and increment counter
          Data[data_count] = customKey;
          data_count++;
        }
  
        // See if we have reached the password length
        if (data_count == Password_Length - 1) {
  
          if (!strcmp(Data, Master)) {
            // Password is correct
            Serial.println("Correct");
            // Turn on relay for 5 seconds
            digitalWrite(lockOutput, HIGH);
            delay(5000);
            digitalWrite(lockOutput, LOW);
            
          }
          else {
            // Password is incorrect
            Serial.println("Incorrect");
            delay(1000);
            
          }
  
          // Clear data
          Serial.println("DONE DONE DONE");
          clearData();
          entering = false;
        }
        
        }
    }

    else {
      // This prints out unknown cards so you can add them to your knownTags as needed
      Serial.print("Unknown tag: ");
      Serial.print(newTag);
      Serial.println();
    }
  }

  // Once newTag has been checked, fill it with zeroes
  // to get ready for the next tag read
  for (int c=0; c < idLen; c++) {
    newTag[c] = 0;
  }
}

// This function steps through both newTag and one of the known
//tags. If there is a mismatch anywhere in the tag, it will return 0,
// but if every character in the tag is the same, it returns 1
int checkTag(char nTag[], char oTag[]) {
  for (int i = 0; i < idLen; i++) {
    if (nTag[i] != oTag[i]) {
      return 0;
    }
  }
 return 1;
}

void clearData() {
  // Go through array and clear data
  while (data_count != 0) {
    Data[data_count--] = 0;
  }
  return;
}
