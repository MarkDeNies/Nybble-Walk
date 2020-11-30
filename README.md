# Nybble-Walk
Nybble Arduino Code and support programs

Walk:  Arduino code for the Nybble robot cat.  Heavily modified version of the OpenCat arduino code.  Major changes: replaced IMU library with much simplified direct calls
       to the device.  Saves about 5K, and avoids interrupts, at the expense of requiring reasonably frequent calls by loop().  Only works with the remote (also modified
       to used constants instead of strings).  All commands are table driven:  if a command is received, it is looked up in the table.  The table tells whether it is a 
       simple command (e.g. Meow()), an intrinsic command, such as balance, rest, etc., a Newbility (a skill in memory), or a complex command, built of a sequence of skills,
       and simple commands.  There is a basic time for skills:  typically 20 or 40 ms. There is also a "stepsPerDuty" variable (initially = 18).  Each line in a skill will 
       be divided into "StepsPerDuty" sub-intervals. A simple command can increase or decrease stepsPerDuty by 50%.  In this manner, a walk (0r other skill) can be speeded up
       or slowed down.  With a StepsPerDuty of 1 and a basic time of 20ms, Nybble will walk much faster than the original (5x?).  Or it can be slowed 50x.
       
LegAngles: an MFC program which can be used to create skills.  E.g. MyWalk, in Walk.ico is a 20 line skill to walk, created by LegAngles from a text file of positions 
       for each foot at each line of the skill.  The text file of positions contains 4 pairs of locations, one for the left front, right front, right rear, and left rear
       leg.  each pair is the vertical and horizontal position of that foot.  0,0 is directly below the hip, with the leg fully extended. Vertical goes from 0 to H, where H
       is the sum of the lengths of the upper and lower leg.  These are constants in the program, and should be changed to reflect your kitty's measurements.  In my version,
       the upper leg length is 1, and the lower leg is 1.36 (my kitty has custom booties).  There are commands to calculate individual positions for front and rear feet,
       given the angles.  
