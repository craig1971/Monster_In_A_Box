
/*
Wave Shield Pins in Use: 2, 3, 4, 5, 10, 11, 12 & 13
Pins 13, 12, 11 are always used by the SD card (they are the only pins that have a high speed SPI interface). 
Then there are 5 other pins used to talk to the DAC and SD card, but they can be set to connect to any arduino pin. 
However, by default, the library is configured to use pins 10 (for SD card) and pins 2, 3, 4 and 5 for the DAC. 
To chanage these pins requires modifying the library - the pins are referenced by their 'hardware' pin names (ie PORTD, etc) 
not by arduino pins. 

That means pins 6, 7, 8, 9 and the 6 analog in pins (also known as digital i/o pins 14-20) are available.
https://learn.adafruit.com/adafruit-wave-shield-audio-shield-for-arduino/faq
*/
#include "MonsterSounds.h"

#define RESERVED_00   0   // Reserved for Serial RX
#define RESERVED_01   1   // Reserved for Serial TX
#define RESERVED_02   2   // Reserved for Wave Shield
#define RESERVED_03   3   // Reserved for Wave Shield
#define RESERVED_04   4   // Reserved for Wave Shield
#define RESERVED_05   5   // Reserved for Wave Shield

#define FOG_MACHINE   6   // Connect Digital Pin on Arduino to Relay Module
#define RED_LEDS      7   // Connect Digital Pin on Arduino to Relay Module
#define LID_BOUNCER   8   // Connect Digital Pin on Arduino to Relay Module
#define RESERVED_09   9   // Connect Digital Pin on Arduino to Relay Module

#define RESERVED_10  10   // Reserved for Wave Shield
#define RESERVED_11  11   // Reserved for Wave Shield
#define RESERVED_12  12   // Reserved for Wave Shield
#define RESERVED_13  13   // Reserved for Wave Shield

#define PIR_SENSOR      A0 // PIR Input
#define MOTION_LED      A1 // LED: lights when motion is detected (regardless of pause/sleep/wake state)
#define PAUSED_LED      A2 // LED: lights when system is paused
#define READY_LED       A3 // LED: lights when monster is in READY_TO_WAKE state
#define PAUSE_BUTTON    A4 // Pause Switch Input
#define DEBUG_BUTTON    A5 // Debug Switch Input

// Effects Timer Settings - in Seconds (EDIT THESE)
#define WAKE_DELAY           30 // Minimum amount of time between 'awake' occurencs in Seconds
#define WAKE_DELAY_DEBUG     10 // WAKE_DELAY override when DEGUB switch is engaged
#define SLEEP_SOUND_DELAY     1 // Number of seconds to wait between attempting to fire the next 'sleep' sound
#define WAKE_MIN              3 // Minimum amount of 'awake' time in Seconds
#define WAKE_MAX              5 // Maximum amount of 'awake' time in Seconds
#define RED_LIGHT_EXTRA_TIME  1 // Allows the red lights to run a bit longer than the lid bouncer, if desired
#define SMOKE_EXTRA_TIME      2 // Allows the smoke to run a bit longer than the lid bouncer, if desired 

// Effects Timers Settings - in Milliseconds (DO NOT EDIT THESE)
#define WAKE_DELAY_MILLIS             WAKE_DELAY * 1000
#define WAKE_DELAY_DEBUG_MILLIS       WAKE_DELAY_DEBUG * 1000
#define SLEEP_SOUND_DELAY_MILLIS      SLEEP_SOUND_DELAY * 1000 
#define WAKE_MIN_MILLIS               WAKE_MIN * 1000
#define WAKE_MAX_MILLIS               WAKE_MAX * 1000
#define RED_LIGHT_EXTRA_TIME_MILLIS   RED_LIGHT_EXTRA_TIME * 1000
#define SMOKE_EXTRA_TIME_MILLIS       SMOKE_EXTRA_TIME * 1000

MonsterSounds sounds;

static unsigned long timeSinceLastSnore= 0;
static unsigned long wakeAllowedTimer = 0;

static unsigned long lidBounceTimer = 0;
static unsigned long lidBounceDuration = 0;

static unsigned long smokeTimer = 0;
static unsigned long smokeDuration = 0;

static unsigned long redLightTimer = 0;
static unsigned long redLightDuration = 0;

enum States {
   STATE_INITIALIZE,     // Only while running setup() and first time into loop()
   STATE_PAUSED,         // Turn off all sounds and effects
   STATE_SLEEPING,       // No effects, sleeping sounds, does not allow awake to be triggered
   STATE_READY_TO_WAKE,  // No effects, sleeping sounds, allows awake to be triggered
   STATE_AWAKE};         // Fires effects and monster awake sounds

States state = STATE_INITIALIZE;

void setup() 
{  
  // initialize serial communication:
  Serial.begin(9600);
  
  // Setup all the relay Pins
  pinMode(LID_BOUNCER, OUTPUT);
  pinMode(RED_LEDS,    OUTPUT);
  pinMode(FOG_MACHINE, OUTPUT);
  
  pinMode(PAUSED_LED,  OUTPUT);
  pinMode(MOTION_LED,  OUTPUT);
  pinMode(READY_LED,   OUTPUT);  

  // Force all Effects to OFF
  stopAllEffects();

  pinMode(PIR_SENSOR,      INPUT);
  pinMode(PAUSE_BUTTON,    INPUT_PULLUP);
  pinMode(DEBUG_BUTTON,    INPUT_PULLUP);

  sounds.initialize(); // Monster Sounds
  
  sounds.playSystemReady(); delay(1000);
  
  Serial.println();
  Serial.print("*** System Ready ***");
  Serial.println();
  
}

/*
 * NOTE: All buttons are using pullups, thus LOW means the button is PRESSED
 *       Keep in mind the pull-up means that swiitch logic is inverted. 
 *       It goes HIGH when it's open, and LOW when it's pressed. 
 *       
 *       The PIR motion sensor does NOT behave this way.
*/

/*
 * Main processing loop
 *     - Manages the Monster's State Machine
 */
void loop() {    

  boolean pauseSwitchClosed = digitalRead(PAUSE_BUTTON) == LOW;
  boolean motionDetected    = digitalRead(PIR_SENSOR) == HIGH;
  
  digitalWrite(MOTION_LED, digitalRead(PIR_SENSOR));
  
  switch (state) {
    
    case STATE_INITIALIZE:
      
      if (pauseSwitchClosed) {  goToPause(); } 
      else { goToSleep();  }
      break;
      
    case STATE_PAUSED:
    
      if (!pauseSwitchClosed) { goToSleep(); digitalWrite(PAUSED_LED, LOW); }
      else { digitalWrite(PAUSED_LED, HIGH); }
      break;
      
    case STATE_SLEEPING: 
    
      if (pauseSwitchClosed) { goToPause(); } 
      else if ( isAllowedToWake() ) { goToReadyToWake(); } 
      else { processSleeping(); }
      break;
      
    case STATE_READY_TO_WAKE:  
    
      if (pauseSwitchClosed)   { goToPause(); digitalWrite(READY_LED, LOW); }   
      else if (motionDetected) { goToAwake(); digitalWrite(READY_LED, LOW); }
      else { processSleeping(); }          
      break;
      
    case STATE_AWAKE:  
    
      if (pauseSwitchClosed){ goToPause(); }      
      else if ( processAwakeAnimation() ) { goToSleep(); } // processAwakeAnimation() returns true when all animations are complete
      break;
      
    default: Serial.println("UNKNOWN STATE"); break;  // We should never get here      
  }  
}

/*
 * Transition to the Pause State
 */
inline void goToPause() {
  Serial.println("PAUSED");
  state = STATE_PAUSED;
  stopAllEffects();
  sounds.stopAll();
}

/*
 * Transition to the Sleep State
 */
inline void goToSleep() {
  Serial.println("GOING TO SLEEP");
  state = STATE_SLEEPING;
  wakeAllowedTimer = millis();
}

/*
 * Transition to the Ready To Awake State
 *     This is a special case of the sleeping state
 */
inline void goToReadyToWake() {
  Serial.println("READY TO WAKE");
  state = STATE_READY_TO_WAKE;  
}

/*
 * Transition to the Awake State
 *     - Wake the monster and process the effects & sounds
 */
inline void goToAwake() {
  Serial.println("AWAKE");
  state = STATE_AWAKE;
  wakeMonster();
}

/*
 * process a cycle of the SLEEP activity
 *    - Run Sleep Sounds
 *    - Update sleep timer
 */
inline void processSleeping() {
  if ((millis() - timeSinceLastSnore) > SLEEP_SOUND_DELAY_MILLIS) {   
      sounds.playSnore();
      timeSinceLastSnore = millis();        
  }
}


/*
 * Determines if monster is allowed to wake up at this time.
 *      - Check debug switch, if its closed we use a shorter Wake Allowed Timer
 *      - Monster must sleep for a predefined minimum amout of time before it may be awoken
 *      - Illuminate LED when it is ready to be awoken
 */
inline boolean isAllowedToWake() {
  boolean isDebug = digitalRead(DEBUG_BUTTON) == LOW;
  unsigned long requiredDelay = WAKE_DELAY_MILLIS;
  if ( isDebug ) { requiredDelay = WAKE_DELAY_DEBUG_MILLIS; }
  
  boolean isAllowed = (millis() - wakeAllowedTimer ) > requiredDelay;

  if (isAllowed ) { digitalWrite(READY_LED, HIGH); } 
  return isAllowed;
}

/*
 * Wake Monster
 *   Starts the Awake Animations
 *   Plays the Awake Sounds
 *   
 *   Call this ONCE to start the AWAKE state.
 */
void wakeMonster() 
{    
  int activityDuration = random(WAKE_MIN_MILLIS, WAKE_MAX_MILLIS); // this is how long the monster will be active

  Serial.print("   wake duration: ");
  Serial.print(activityDuration);
  Serial.println(" ms ");
  
  bounceLid(activityDuration);
  flashRedLight(activityDuration + RED_LIGHT_EXTRA_TIME_MILLIS);
  activateSmoke(activityDuration + SMOKE_EXTRA_TIME_MILLIS);    

  sounds.playRoar();
}
 
/*
 * Manages the progress of the AWAKE animations
 *    Call this EVERY CYCLE during the AWAKE state.
 *    Returns TRUE when all animations are complete
 */
boolean processAwakeAnimation()
{ 
  printTimersToLog();
  
  boolean done1 = false;
  boolean done2 = false;
  boolean done3 = false;
  
  if (millis() - lidBounceTimer > lidBounceDuration) 
  {
    bounceLid(0);
    done1 = true;
  }
  
  if (millis() - redLightTimer > redLightDuration ) 
  {
    flashRedLight(0);
    done2 = true;
  }
  
  if (millis() - smokeTimer > smokeDuration )
  {
    activateSmoke(0);
    done3 = true;
  } 
  
  return done1 && done2 && done3;
}

/*
 * Manage Effect: Bounce the box lid
 *     - diration is the number of milliseconds that the effect should run
 *     - duration of 0 means the effect should be stopped
 */
inline void bounceLid(unsigned long duration)
{
  if (duration <= 0) 
  {
    closeRelay(LID_BOUNCER);
    lidBounceDuration = 0;
    
  } else {
    // start the lid bouncing
    openRelay(LID_BOUNCER);
    lidBounceTimer = millis();
    lidBounceDuration = duration;
  }
}

/*
 * Manage Effect: Flash Red Lights
 *     - diration is the number of milliseconds that the effect should run
 *     - duration of 0 means the effect should be stopped
 */
inline void flashRedLight(unsigned long duration)
{
  if (duration <= 0) 
  {
    closeRelay(RED_LEDS);
    redLightDuration = 0;
    
  } else {
    // start the light flashing
    openRelay(RED_LEDS);
    redLightTimer = millis();
    redLightDuration = duration;
  }
  
}

/*
 * Start/Stop Effect: Activate Smoke
 *     - diration is the number of milliseconds that the effect should run
 *     - duration of 0 means the effect should be stopped
 */
 inline void activateSmoke(unsigned long duration)
 {
  // 'press' the smoke button
  // duration should be a fixed amount of time needed for the machine to respond to the action
  // set a timeout to stop after duration
  
  if (duration <= 0) 
  {
    closeRelay(FOG_MACHINE);
    smokeDuration = 0;
    
  } else {
    // start the light flashing
    openRelay(FOG_MACHINE);
    smokeTimer = millis();
    smokeDuration = duration;
  }
 }


/*
 * Stop all of the Effects 
 *         - This effectively turns the monster off
 */
inline void stopAllEffects()
{
  bounceLid(0);
  flashRedLight(0);
  activateSmoke(0);
}

/*
 * Prints the Awake Animation timers to the log once per second
 */
inline void printTimersToLog() {
  static unsigned long timeofLastTimerLog = 0;
  if (millis() - timeofLastTimerLog >= 1000)  {
    
    Serial.print("   lid: ");
    Serial.print( (millis()-lidBounceTimer) > lidBounceDuration ? 0 : (lidBounceDuration -(millis()-lidBounceTimer) ) );
    Serial.print("  lights: ");
    Serial.print( (millis()-redLightTimer) > redLightDuration ? 0 : (redLightDuration -(millis()-redLightTimer) ) );
    Serial.print("  smoke: ");
    Serial.println( (millis()-smokeTimer) > smokeDuration ? 0 : (smokeDuration -(millis()-smokeTimer) ) );
    
    timeofLastTimerLog = millis();
  }
}

/* 
 *  Sets the Normally Open (NO) terminal to OPEN
 *  Normally Closed will become Closed
 */
 inline void openRelay(int channel)
 {
  digitalWrite(channel, HIGH);
 }

/* 
 *  Sets the Normally Open (NO) terminal to Closed.
 *  Normally Closed will become OPEN
 */
 inline void closeRelay(int channel)
 {
  digitalWrite(channel, LOW);
 }
