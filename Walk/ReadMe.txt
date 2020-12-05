Walk.ino is a highly modified version of the original Nybble.ino code.

The goal was to reduce the size of the program, increase the speed of walking,
and, in general experiment with Nybble's walking.

Changes, in order of implementation:

1.  As suggested by Rongzhong Li, I edited the IRremote library to enable only the 
    device which is being used (in my case, NEC) by editing IRremote.h.  This saves 4K.  

2.  Converted the GetCmd() function to return an index (a number) instead of a string
    to identify the button which was pushed on the remote. This saved space, but also
    allows using tables instead of code to choose commands.

3.  There is now a list of #defines for each of the the instincts in EEPROM.  These
    are the addresses.  So, if the order changes, this table will need to be changed.
    ***this is not as flexible as the original code.

4.  the commands index into a table, cmdInfo[].  The commands can be SIMPLE (e.g. 
    sayMeow, or Faster), an INTRINSIC_SKILL, or a NEWBILITY_SKILL, or a SEQUENCE of
    commands.  This table saves programming, and space.

5.  Instead of updating one leg position per loop, all leg angles are updated at once, 
    the frequency controlled by tMotorWait (typically set to 20ms).  stepsPerDuty 
    tells the code how many interpolation steps are to be taken between each line in
    the skill.  E.G. if stepsPerDuty is 10, it will take 10*20ms to travel to the
    next line in the skill, but the motion will be much smoother.  

6.  stepsPerDuty is increased by the lower left remote button and decreased by the
    lower right button.  This controls the speed.

7.  The Gyro library was expensive, so I replaced it with code written by Dejan for 
    howtomechatronics.com.  It uses much less CPU time and space, but it probably is
    not as accurate, since it doesn't use interrupts. E.G: there is a remote button
    to reset the Gyro when it gets confused.  There is SOME auto recovery, but not
    as sophisticated as in the original code. 

8.  There are a number of NewBilities.  The primary one being myWalk.  It is shorter
    than the original Walk, and was created with a helper program written in C.  The
    helper program takes a list of foot positions and creates the angles needed for
    the Newbility.  The Walk is designed so that three feet are always on the ground,
    and each foot is moving backwards before it reaches the ground, and continues
    to move backwards after it leaves.  While on the ground, all feet move exactly the
    same distance at each step.