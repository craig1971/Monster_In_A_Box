

#define LID_BOUNCER 2   // Connect Digital Pin on Arduino to Relay Module
#define RED_LEDS    3   // Connect Digital Pin on Arduino to Relay Module
#define FOG_MACHINE 4   // Connect Digital Pin on Arduino to Relay Module
#define UNUSED      5   // Connect Digital Pin on Arduino to Relay Module

#define ACTION_BUTTON 6
 
#define FRAMERATE          20
#define FRAMEDELAY         1000/FRAMERATE

#define WAKE_SECONDS_MIN    3 // Seconds
#define WAKE_SECONDS_MAX    5 // Seconds
#define WAKE_DELAY         10 // Seconds

#define WAKE_FRAMES_MIN     WAKE_SECONDS_MIN  * FRAMERATE
#define WAKE_FRAMES_MAX     WAKE_SECONDS_MAX * FRAMERATE
#define WAKE_DELAY_FRAMES   WAKE_DELAY  * FRAMERATE

void setup() 
{  
  // initialize serial communication:
  Serial.begin(9600);
  
  // Setup all the relay Pins
  pinMode(LID_BOUNCER, OUTPUT);
  pinMode(RED_LEDS,    OUTPUT);
  pinMode(FOG_MACHINE, OUTPUT);
  pinMode(UNUSED,      OUTPUT);

  openRelay(LID_BOUNCER);
  openRelay(RED_LEDS);
  openRelay(FOG_MACHINE);
  openRelay(UNUSED);

  pinMode(ACTION_BUTTON, INPUT_PULLUP);
  
}

boolean monsterIsAwake = false;

void loop() 
{
  if (digitalRead(ACTION_BUTTON) == LOW)   // using a pullup, thus LOW means the button is pressed
  {
    wakeMonster();
  }

  if (monsterIsAwake) 
  {
    updateDevices();
  }
  
  delay(FRAMEDELAY);
}


/*
 * Wake Monster
 */
void wakeMonster() 
{
  if (!monsterIsAwake)
  {
    int activityDuration = random(WAKE_FRAMES_MIN, WAKE_FRAMES_MAX); // this is how long the monster will be active

  Serial.print("WAKE UP : ");
  Serial.print(WAKE_FRAMES_MIN);
  Serial.print(" : ");
  Serial.print(WAKE_FRAMES_MAX);
  Serial.print(" : ");
  Serial.println(activityDuration);
    
    int wakeDuration = activityDuration + WAKE_DELAY_FRAMES;    // force a delay between wake cycles
    monsterIsAwake = true;

    bounceLid(activityDuration);
    flashRedLight(activityDuration);
    activateSmoke(FRAMERATE);    // Hold the smoke button for 1 second 
  }
}

int lidBounceCountdown     = 0;
int flashRedLightCountdown = 0;
int smokeCountdown         = 0;
 
/*
 * 
 */
void updateDevices()
{ 
  Serial.print(lidBounceCountdown);
  Serial.print("  -  ");
  Serial.print(flashRedLightCountdown);
  Serial.print("  -  ");
  Serial.print(smokeCountdown);
  Serial.println(" ");
  
  monsterIsAwake = lidBounceCountdown > 0 || flashRedLightCountdown > 0 || smokeCountdown > 0;
  
  if (lidBounceCountdown <= 0) 
  {
    bounceLid(0);
  } else {
    lidBounceCountdown--;
  }
  
  if (flashRedLightCountdown <= 0) 
  {
    flashRedLight(0);
  } else {
    flashRedLightCountdown--;
  }
  
  if (smokeCountdown <= 0) 
  {
    activateSmoke(0);
  } else {
    smokeCountdown--;
  }
  
  
}


/*
 * Bounce the box lid
 *     duration of 0 means STOP
 */
void bounceLid(int duration)
{
  if (duration <= 0) 
  {
    openRelay(LID_BOUNCER);    
    
  } else {
    // start the lid bouncing
    closeRelay(LID_BOUNCER);
    
    // set a timeout to stop after duration
    lidBounceCountdown = duration;
  }
}


/*
 * Flash Red Lights
 *     duration of 0 means STOP
 */
void flashRedLight(int duration)
{
  if (duration <= 0) 
  {
    openRelay(RED_LEDS);
    
  } else {
    // start the light flashing
    closeRelay(RED_LEDS);
    
    // set a timeout to stop after duration
    flashRedLightCountdown = duration;
  }
  
}

/*
 * Activate Smoke
 */
 void activateSmoke(int duration)
 {
  // 'press' the smoke button
  // duration should be a fixed amount of time needed for the machine to respond to the action
  // set a timeout to stop after duration
  
  if (duration <= 0) 
  {
    openRelay(FOG_MACHINE);
    
  } else {
    // start the light flashing
    closeRelay(FOG_MACHINE);
    
    // set a timeout to stop after duration
    smokeCountdown = duration;
  }
 }

/* 
 *  Sets the Normally Open (NO) terminal to OPEN
 *  Normally Closed will become Closed
 */
 void openRelay(int channel) {
  digitalWrite(channel, HIGH);
 }

/* 
 *  Sets the Normally Open (NO) terminal to Closed.
 *  Normally Closed will become OPEN
 */
 void closeRelay(int channel) {
  digitalWrite(channel, LOW);
 }
