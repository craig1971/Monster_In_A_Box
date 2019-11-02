
/* NOTE ABOUT ARDUINO PINS
 * Pins 13, 12, 11 are always used by the SD card (they are the only pins that have a high speed SPI interface). 
 * Then there are 5 other pins used to talk to the DAC and SD card, but they can be set to connect to any arduino pin. 
 * However, by default, the library is configured to use pins 10 (for SD card) and pins 2, 3, 4 and 5 for the DAC. 
 * To chanage these pins requires modifying the library - the pins are referenced by their 'hardware' pin names (ie PORTD, etc) not by arduino pins. 
 * That means pins 6, 7, 8, 9 and the 6 analog in pins (also known as digital i/o pins 14-20) are available.
 * 
 */
 
void MonsterSounds::initialize()
{
  Serial.println("Initializing Sounds...");
  if(!card.init())        Serial.println(F("Card init. failed!"));
  if(!vol.init(card))     Serial.println(F("No partition!"));
  if(!root.openRoot(vol)) Serial.println(F("Couldn't open dir"));
  Serial.println(F("Files found:"));
  root.ls();

  randomSeed(analogRead(0));
}


void  MonsterSounds::playSystemReady()
{
  this->playfile("WELCOME0.WAV");
}

void  MonsterSounds::playRoar()
{
  int index = random(3);  // 0, 1, 2
  while (index == previousRoarSound)
  {
    index = random(3);
  }
  previousRoarSound = index;
  
  this->playfile(roarSounds[index]);
}

void  MonsterSounds::playSnore()
{
  if (!wave.isplaying) // Do not interupt an exising sound with snoring
  {
    int index = random(3); // 0, 1, 2
    this->playfile(sleepSounds[index]);
  }
}

void MonsterSounds::stopAll()
{
  wave.stop(); // Stop any currently-playing WAV
}

// -------------------------------------------------------------------
// playfile()    
// Open and start playing a WAV file
// -------------------------------------------------------------------
void MonsterSounds::playfile(char *name) 
{    
  PgmPrint("Playing sound: ");
  Serial.println(name); 
  
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }

  if (!file.open(root, name)) {
    PgmPrintln("File not found ");
    return;
  }

  if (!wave.create(file)) {
    PgmPrintln("Not a valid WAV");
    return;
  }
  
  // ok time to play!
  wave.play();
}

