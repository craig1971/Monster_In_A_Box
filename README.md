## Ver 4.1 Changes

Modified relays to use Normally Open (NO) positions

Updated sound effects
   - Roar sounds now have rattling chain sounds added
   - Start up sound is now a monster sounds (no longer uses AOL "welcome")
   - Old sounds are in subfolders in the zip


## Ver 4.0 Changes

Total rewrite of main sketch code
 
Main loop is now implemented as a state machine

All timing now based on event time vs. current time evaluations 
   - timing is no longer dependent on frames
   - allowed removal of delay() in main loop
   - eliminates the 'port busy' issue when connecting to Arduino IDE
    
Relays now using Normally Open rather than Normally Closed connections

Added Motion Detected LED

Added Ready to Wake LED

Added Paused LED

Removed Action Trigger Switch

Added abillity to add extra time to Red Lights & Smoke
   - allows these effects to run a bit longer than the lid bouncer
Added lots of comments



# Monster In A Box
Arduino based controller for a Monster In A Box Halloween prop.

The idea is pretty simple: create the illusion that some type of container is holding a dangerous monster that is on the verge of escaping. The illusion can be created with any combination of motion, sound, light, smoke and, of course, surprise.

Full Build Instructions are Here:
https://www.hackster.io/craig-jameson/monster-in-a-box-41cc38


<img src="./images/image_0.jpg" height=250><img src="./images/image_1.jpg" height=250><br>

Checkout a video of the box on Halloween night 
https://www.youtube.com/watch?v=Fk9QA6AY-tw

### Wiring Diagram

<img src="./Monster%20in%20a%20Box%20Diagram.png" width=700><br>



### This project depends on the use of an Adafruit Wave Shield
https://learn.adafruit.com/adafruit-wave-shield-audio-shield-for-arduino/overview

<img src="./images/waveshield.jpg" width=300><br>
