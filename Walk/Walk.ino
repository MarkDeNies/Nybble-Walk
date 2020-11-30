/* Main Arduino sketch for walk test.  This will check to see if there is any problem with sending all 
 *  8 legs the next position to go to in the walk in each loop --Done.  9/2020
 *  
 *  If that works, create a GetCmd function to get the next command from IR.  --Done 9/2020
 *  
 *  Replace the text return from getCmd with byte constants.  Use the byte constants to index into a
 *  table to determine what to do (intrinsic skill, simple command like meow, or a sequence of commands.
 *  
 *  Then add the gyro code.  Hopefully, if the gyro code is only called when needed, it'll not interfere with
 *  the IR?  Ended up replacing it with Dejan's MPU6050 code 11/3/2020.    
 *  
 *  11/2020: turns out many of the problems (e.g. IR vs MPU interference) were due to not properly configuring 
 *  Arduino for the board.  Instructions 4.2.3.  Now, I can set tMotorWait to 10ms.  Everything goes faster.
*/
#define MAIN_SKETCH
#include <IRremote.h>
#include "OpenCat.h"

//****************This section could be combined into a class (data containter) for control.  Or include in motion class?
bool bRunning;                    // true if currently running a gait command
bool bNeedToResetGyro = false;    // Gyros need to be reset when motion settles down
unsigned long tStart,tPrev;
unsigned long loopcnt, prevcnt;
unsigned long tPrevMotorCall = 0;     // Time of last motor update
byte iStep=1;                         // current step in the current dutyAngle
int stepsPerDuty = 18;                // initial number of motor calls to divide each dutyAngle into (effectively slows and smooths the motion)
unsigned int tMotorWait = 10;         // ms delay between motor updates: 
unsigned long tStopTime = 0xffffffff; // Time at which to stop running the current command (which must be a gait).
unsigned long tPrevSeq = 0xffffffff;  // used to prevent sequence steps from running faster than ever 40ms
bool bInSequence;                     // true if currently running a sequence of commands
byte seqNo;             // Current Sequence being run
byte seqStep;           // Current step in the current sequence
byte nSequence;         // numbewr of setps in current sequence
bool bIntrinsicLoaded;  // true when the intrinsic skill has been loaded by the sequence
float mostRecentAngle[8]; // remembers the current position of all leg motors
float previousAngle[8];         //remembers the most recent Duty angle (i.e. intrinsic entry) for all leg motors

byte iDuty=0;             // index into the currently active skill
byte firstMotionJoint;  // index of first entry in motion.dutyAngles[] which is a leg joint
  //***************end of This Section
  
  //****EEPROM addresses of intrinsic skills
#define ADDRESS_BD 0
#define ADDRESS_BK ADDRESS_BD + sizeof(bd)
#define ADDRESS_BKL ADDRESS_BK + sizeof(bk)
#define ADDRESS_BKR ADDRESS_BKL + sizeof(bkL)
#define ADDRESS_CR ADDRESS_BKR + sizeof(bkR)
#define ADDRESS_CRL ADDRESS_CR + sizeof(cr)
#define ADDRESS_CRR ADDRESS_CRL + sizeof(crL)
#define ADDRESS_LY ADDRESS_CRR + sizeof(crR)
#define ADDRESS_TR ADDRESS_LY + sizeof(ly)
#define ADDRESS_TRL ADDRESS_TR + sizeof(tr)
#define ADDRESS_TRR ADDRESS_TRL + sizeof(trL)
#define ADDRESS_VT ADDRESS_TRR + sizeof(trR)
#define ADDRESS_WK ADDRESS_VT + sizeof(vt)
#define ADDRESS_WKL ADDRESS_WK + sizeof(wk)
#define ADDRESS_WKR ADDRESS_WKL+ sizeof(wkL)
#define ADDRESS_BALANCE ADDRESS_WKR + sizeof(wkR)
#define ADDRESS_BUTTUP ADDRESS_BALANCE + sizeof(balance)
#define ADDRESS_CALIB ADDRESS_BUTTUP + sizeof(buttUp)
#define ADDRESS_CD1 ADDRESS_CALIB + sizeof(calib)
#define ADDRESS_CD2 ADDRESS_CD1 + sizeof(cd1)
#define ADDRESS_DROPPED ADDRESS_CD2 + sizeof(cd2)
#define ADDRESS_HI ADDRESS_DROPPED + sizeof(dropped)
#define ADDRESS_HI2 ADDRESS_HI + sizeof(hi)
#define ADDRESS_LIFTED ADDRESS_HI2 + sizeof(hi2)
#define ADDRESS_PEE ADDRESS_LIFTED + sizeof(lifted)
#define ADDRESS_PEE1 ADDRESS_PEE + sizeof(pee)
#define ADDRESS_PU1 ADDRESS_PEE1 + sizeof(pee1)
#define ADDRESS_PU2 ADDRESS_PU1 + sizeof(pu1)
#define ADDRESS_RC1 ADDRESS_PU2 + sizeof(pu2)
#define ADDRESS_RC10 ADDRESS_RC1 + sizeof(rc1)
#define ADDRESS_RC2 ADDRESS_RC10 + sizeof(rc10)
#define ADDRESS_RC3 ADDRESS_RC2 + sizeof(rc2)
#define ADDRESS_RC4 ADDRESS_RC3 + sizeof(rc3)
#define ADDRESS_RC5 ADDRESS_RC4 + sizeof(rc4)
#define ADDRESS_RC6 ADDRESS_RC5 + sizeof(rc5)
#define ADDRESS_RC7 ADDRESS_RC6 + sizeof(rc6)
#define ADDRESS_RC8 ADDRESS_RC7 + sizeof(rc7)
#define ADDRESS_RC9 ADDRESS_RC8 + sizeof(rc8)
#define ADDRESS_REST ADDRESS_RC9 + sizeof(rc9)
#define ADDRESS_SIT ADDRESS_REST + sizeof(rest)
#define ADDRESS_SLEEP ADDRESS_SIT + sizeof(sit)
#define ADDRESS_STR ADDRESS_SLEEP + sizeof(sleep)
#define ADDRESS_ZERO ADDRESS_STR + sizeof(str)
  //******end of EEPROM addresses

// NewBilities:  Note, If only one line, there must be 16 entries.
//                     If >1 line, each line has 8 entries.

//const char earScratch[] PROGMEM = { 
//3, 0, 0,
//  50, 50,-50,-60, -10, -10, 10, -82,
// -60, 50,-50,-60, -40, -10, 10, -82,
// -60, 50,-50,-60, -62, -10, 10, -82,
//};

const char turnAround[] PROGMEM = { 
3, 0, 0,
  70, 30,-70,-30, -1,  5, 1, -5,  // original position
  75, 30,-80,-30, 10, -5, 1,  0,  // turn:  swap left and right side positions
  80, 50,-80,-50, -1, -5, 20,  0,  // turn:  swap left and right side positions
};
const char myWalkOld[] PROGMEM = { 
42, 0, 0,
 12, 58,-55,-50, 21, 27, -2,-12,   //0
 13, 59,-63,-48, 20, 29, -8,-11,
 17, 59,-67,-46, 18, 32,-20,-11,
 21, 59,-66,-44, 16, 34,-33,-10,
 24, 59,-64,-41, 16, 37,-37,-10,
 27, 58,-62,-38, 15, 41,-41,-11,
 30, 58,-60,-35, 13, 44,-45,-12,
 32, 58,-57,-33, 13, 47,-48,-13,
 35, 59,-57,-30, 12, 47,-47,-14,
 38, 61,-58,-27, 12, 43,-42,-15,
 40, 64,-59,-24, 12, 38,-37,-16,  //10
 43, 65,-59,-21, 11, 34,-33,-17,
 45, 67,-59,-18, 11, 23,-30,-19,
 47, 64,-59,-15, 11, 10,-26,-21,
 49, 56,-58,-12, 12,  3,-23,-23,
 51, 45,-57,-12, 13,  1,-21,-23,
 53, 31,-56,-12, 15,  5,-18,-20,
 54, 18,-55,-14, 17, 13,-15,-17,
 55, 16,-54,-16, 20, 15,-13,-15,
 56, 15,-52,-18, 22, 18,-13,-13,
 57, 13,-50,-33, 25, 20,-12, -4,
 58, 12,-48,-47, 27, 21,-12,  0,  //21
 59, 13,-46,-55, 29, 20,-11, -2,
 59, 17,-44,-63, 32, 18,-10, -8,
 59, 21,-41,-67, 34, 16,-10,-20,
 59, 24,-38,-66, 37, 16,-11,-33,
 58, 27,-35,-64, 41, 15,-12,-37,
 58, 30,-33,-62, 44, 13,-13,-41,
 58, 32,-30,-60, 47, 13,-14,-45,
 59, 35,-27,-57, 47, 12,-15,-48,
 61, 38,-24,-57, 43, 12,-16,-47,
 64, 40,-21,-58, 38, 12,-17,-42,
 65, 43,-18,-59, 34, 11,-19,-37, //32
 67, 45,-15,-59, 23, 11,-21,-33,
 64, 47,-12,-59, 10, 11,-23,-30,
 56, 49,-12,-59,  3, 12,-23,-26,
 45, 51,-12,-58,  1, 13,-20,-23,
 31, 53,-14,-57,  5, 15,-17,-21,
 18, 54,-16,-56, 13, 17,-15,-18,
 16, 55,-18,-55, 15, 20,-13,-15,
 15, 56,-33,-54, 18, 22, -4,-13,
 13, 57,-47,-52, 20, 25,  0,-13,
};
const char myWalk[] PROGMEM = { 
20, 0, 0,
52,48,-50,-48,56,18,-20,-66,
63,50,-48,-52,27,20,-18,-56,
61,53,-46,-55,17,20,-17,-47,
56,54,-43,-56,11,23,-17,-41,
48,55,-40,-56,9,26,-17,-37,
33,56,-37,-56,19,29,-17,-33,
37,56,-51,-56,17,33,-9,-29,
40,56,-58,-55,17,37,-13,-26,
43,56,-62,-54,17,41,-20,-23,
46,55,-63,-53,17,47,-32,-20,
48,52,-48,-50,18,56,-66,-20,
50,63,-52,-48,20,27,-56,-18,
53,61,-55,-46,20,17,-47,-17,
54,56,-56,-43,23,11,-41,-17,
55,48,-56,-40,26,9,-37,-17,
56,33,-56,-37,29,19,-33,-17,
56,37,-56,-51,33,17,-29,-9,
56,40,-55,-58,37,17,-26,-13,
56,43,-54,-62,41,17,-23,-20,
55,46,-53,-63,47,17,-20,-32,
};
typedef enum {
  CMD_WALK,
  CMD_WALK_LEFT,
  CMD_WALK_RIGHT,
  CMD_BACK,
  CMD_BACK_LEFT,
  CMD_BACK_RIGHT,
  CMD_FASTER,
  CMD_SLOWER,
  CMD_BALANCE,
  CMD_REST,
  CMD_SIT,
  CMD_HI,
  CMD_TROT,
  CMD_TURNAROUND,
  CMD_MEOW,
  CMD_DEBUG,    // used to turn on/off the cmdDebug variable
  CMD_SIT_MEOW,
  CMD_WALK_TURNAROUND_SAY_HI,
  CMD_RESET_GYRO,
  CMD_MY_WALK,
  CMD_ZERO,
  NCOMMAND,   //Always last
} CommandIndex;
#define CMD_INVALID  NCOMMAND

typedef enum {
  CMD_SIMPLE,            // command is to do a simple task (e.g. increase walking speed, meow, etc
  CMD_INTRINSIC_SKILL,  // run one of Ronzong Li's skills.  (Thes are in EEPROM -- see above)
  CMD_NEWBILITY_SKILL,  // run a custom skill (these are in the program)
  CMD_SEQUENCE,
};

// Info about the current command
typedef struct {
  byte type;
  void (*subr)(int);
  unsigned int   address;
} CMDINFO;

//****************************************************************************************************************
// Increase steps/Duty (i.e. slow down Nybble) if factor > 0,  Decrease steps/Duty if factor < 0;
void changeSpeed(int factor)
{
  int howMuch = stepsPerDuty / factor;
  if (howMuch == 0) howMuch = factor > 0? 1 : -1;  // change howMuch to +1 or -1

    stepsPerDuty += howMuch;

  if (stepsPerDuty < 1) stepsPerDuty = 1;
  if (stepsPerDuty > 50) stepsPerDuty = 50;
  PT("Steps per DutyCyle="); //****debug
  PTL(stepsPerDuty);         //****debug
}

//****************************************************************************************************************
void sayMeow(int howMuch)
{
  meow();
}

//******************start of MPU replacement code:  TODO:  encapsulate variables and functions into a class
const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY; // , GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY; //, gyroAngleZ;  // I don't need the Z angle

float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY; //, GyroErrorZ;
float elapsedTime, currentTime, previousTime;

//****************************************************************************************************************
// code from:  Arduino and MPU6050 Accelerometer and Gyroscope Sensor Tutorial
// by Dejan, https://howtomechatronics.com
void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gyro data error. 
  // From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values 200 times

  for (int c=0; c< 200; c++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // Sum all readings  
    AccErrorX = AccErrorX + (atan(AccY / sqrt(AccX*AccX + AccZ*AccZ)) * DEG_PER_RAD);
    AccErrorY = AccErrorY + (atan(-1 * AccX / sqrt(AccY*AccY + AccZ*AccZ)) * DEG_PER_RAD);
  }
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / 200;
  AccErrorY = AccErrorY / 200;

  // Read gyro values 200 times
  for (int c=0; c< 200; c++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 4, true);  // used to be 6
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
//    GyroZ = Wire.read() << 8 | Wire.read();  //GyroZ not currently used
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
 //   GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);  //GyroZ not currently used
  }
  //Divide the sum by 200 to get the error value
  GyroErrorX = GyroErrorX / 200;
  GyroErrorY = GyroErrorY / 200;
//  GyroErrorZ = GyroErrorZ / 200;
}

//****************************************************************************************************************
// code from:  Arduino and MPU6050 Accelerometer and Gyroscope Sensor Tutorial
// by Dejan, https://howtomechatronics.com
void mpuSetup() {
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
  /*
  // Configure Accelerometer Sensitivity - Full Scale Range (default +/- 2g)
  Wire.beginTransmission(MPU);
  Wire.write(0x1C);                  //Talk to the ACCEL_CONFIG register (1C hex)
  Wire.write(0x10);                  //Set the register bits as 00010000 (+/- 8g full scale range)
  Wire.endTransmission(true);
  // Configure Gyro Sensitivity - Full Scale Range (default +/- 250deg/s)
  Wire.beginTransmission(MPU);
  Wire.write(0x1B);                   // Talk to the GYRO_CONFIG register (1B hex)
  Wire.write(0x10);                   // Set the register bits as 00010000 (1000deg/s full scale)
  Wire.endTransmission(true);
  delay(20);
  */
  // Call this function if you need to get the IMU error values for your module
  calculate_IMU_error();
  delay(20); 
  currentTime = millis();            // set Current time.  Used for integration of gyro angles
}

//****************************************************************************************************************
// If IMU gets bad pitch or roll, this resets them to zero.  
void resetGyro(int howMuch)
{
  PTL("reset gyros to zero");  //iDuty
 gyroAngleX =  gyroAngleY = 0; //, gyroAngleZ;  // I don't need the Z angle

//float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY; //, GyroErrorZ;
elapsedTime = currentTime = previousTime = millis();
}

//****************************************************************************************************************
bool bCmdDebug = false;     // used to let user turn on/off debug print 
void cmdDebug(int howMuch)  // toggle bCmdDebug
{
  bCmdDebug = bCmdDebug ? false : true;
}

// table of Commands, indexed by commandIndex, which are returned by getCmd()
PROGMEM const CMDINFO cmdInfo[NCOMMAND] ={  // This data is read-only, so it is in Program memory
  {CMD_INTRINSIC_SKILL,0, ADDRESS_WK},
  {CMD_INTRINSIC_SKILL,0, ADDRESS_WKL},
  {CMD_INTRINSIC_SKILL,0, ADDRESS_WKR},
  {CMD_INTRINSIC_SKILL,0, ADDRESS_BK},
  {CMD_INTRINSIC_SKILL,0, ADDRESS_BKL},
  {CMD_INTRINSIC_SKILL,0, ADDRESS_BKR},
  {CMD_SIMPLE, &changeSpeed, -3},  // 1/3 slower
  {CMD_SIMPLE, &changeSpeed, +2},  // 50% faster
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_BALANCE},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_REST},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_SIT},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_HI},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_TR},
  {CMD_NEWBILITY_SKILL, 0, turnAround},
  {CMD_SIMPLE, &sayMeow, 0},
  {CMD_SIMPLE, &cmdDebug, 0},
  {CMD_SEQUENCE, 0, 0},   // CMD_SIT_MEOW
  {CMD_SEQUENCE, 0, 1},  // CMD_WALK_TURNAROUND_SAY_HI,
  {CMD_SIMPLE,&resetGyro,0},  //****debug: reset gyros
  {CMD_NEWBILITY_SKILL,0,myWalk},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_ZERO},
};

//*******************************************************************************
// defined sequences of commands.  These are constant, and stored in program memory (FLASH)

 typedef struct {
    byte type;
    void (*subr)(int);
    unsigned int   address; 
    int wait; 
 } SEQUENCE_ENTRY;

 typedef struct {
  byte nSequence;         // number of entries in the sequence 
  SEQUENCE_ENTRY *entry;  // pointer to the sequences
 } SEQUENCE;

// sequences of commands.  Can contain simple commands, Intrinsic skills or newbility skills
PROGMEM const SEQUENCE_ENTRY sequenceMeowSit[] {
  {CMD_SIMPLE, &sayMeow, 0, 1000},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_SIT, 2000},
  {CMD_SIMPLE, &sayMeow, 0, 2000},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_REST, 1400}, // then lie down
};

PROGMEM const SEQUENCE_ENTRY sequenceWalkTurnAroundSayHi[] {
  {CMD_INTRINSIC_SKILL,0, ADDRESS_BALANCE, 400},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_WK, 3000}, // walk faster every 3000ms
  {CMD_SIMPLE, &changeSpeed, -2, 3000}, // walk faster every 3000ms
  {CMD_SIMPLE, &changeSpeed, -2, 3000},
  {CMD_INTRINSIC_SKILL,0, ADDRESS_BALANCE, 400},  // then stop
  {CMD_SIMPLE, &changeSpeed, +1, 450},
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_BKL, 12000},  // walk backwards and left
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_WKR, 9000},   // walf front and right
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_SIT, 400}, // then sit down
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_HI, 1000}, // then raise left front paw
  {CMD_INTRINSIC_SKILL, 0, ADDRESS_HI2, 1000}, // 
};

PROGMEM const SEQUENCE sequence[] {
  {4,sequenceMeowSit},
  {11, sequenceWalkTurnAroundSayHi},
};

IRrecv irrecv(IR_RECIEVER);     // create instance of 'irrecv'

//****************************************************************************************************************
// This should be part of an mpu class?
void checkBodyMotion() {
    // === Read acceleromter data === 
   // code from:  Arduino and MPU6050 Accelerometer and Gyroscope Sensor Tutorial
   // by Dejan, https://howtomechatronics.com
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
  // Calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(AccY / sqrt(AccX*AccX + AccZ*AccZ)) * DEG_PER_RAD) - AccErrorX; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * AccX / sqrt(AccY*AccY + AccZ*AccZ)) * DEG_PER_RAD) - AccErrorY; // AccErrorY ~(-1.58)
  
  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 4, true); // Read 4 registers total (6 if GyroZ needed), each axis value is stored in 2 registers
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
//  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;  // GyroZ not currently used
  
  // Correct the outputs with the calculated error values.  Values are then in 
  // degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  // Note: since we don't use yaw, those lines are commented out
  gyroAngleX = gyroAngleX + (GyroX-GyroErrorX) * elapsedTime; // deg/s * s = deg
  gyroAngleY = gyroAngleY + (GyroY-GyroErrorY) * elapsedTime;
 // float yaw =  yaw + GyroZ * elapsedTime;
  // Complementary filter - combine acceleromter and gyro angle values
  float roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  float pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  if (fabs(pitch) > LARGE_PITCH) { //****debug:  this is pretty unsophisticated
    PT("pitch is: ");
    PTL(pitch);
    bNeedToResetGyro = true;   // Gyros need to be reset when motion settles down
  }
  if (fabs(roll) > LARGE_PITCH) {  //****debug:  this is pretty unsophisticated
    PT("roll is: ");
    PTL(roll);
    bNeedToResetGyro = true;   // Gyros need to be reset when motion settles down
  }
  if (bCmdDebug) {  //****debug
    PT("roll, pitch=");
    PT(roll);
    PT(" ");
    PTL(pitch);
  }

  pitch = pitch * RAD_PER_DEG;
  roll = -roll * RAD_PER_DEG;   // roll value is backwards on my Nybble
 
  RollPitchDeviation[0] = roll - motion.expectedRollPitch[0];
  RollPitchDeviation[0] = sign(roll) * max(fabs(RollPitchDeviation[0]) - levelTolerance[0], 0);//filter out small angles
  
  RollPitchDeviation[1] = pitch - motion.expectedRollPitch[1];
  RollPitchDeviation[1] = sign(pitch) * max(fabs(RollPitchDeviation[1]) - levelTolerance[1], 0);//filter out small angles
}

//******************************************************************************
// when resetting motors, Load the rest intrinsic skill, and clear the Gyro angle
void resetMotorsToRest()
{
  loadIntrinsic(ADDRESS_REST);
  for (int i=0; i<8; i++) {
     float targetAngle = motion.dutyAngles[i+DOF-WALKING_DOF];
     mostRecentAngle[i] = targetAngle;
     calibratedPWM(DOF-WALKING_DOF+i, targetAngle);  // start out in rest position
  }
  delay(200);
  resetGyro(0);
  bNeedToResetGyro = false;

  bRunning = false;
  bInSequence = false;
  tStart = millis();
  tPrev  = tStart;
}

//***********************************************************************************************
void setup() {

  pinMode(BUZZER, OUTPUT);
  loopcnt = 0;             //***debug: for use in printing info periodically (once/second)

  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  //Wire.setClock(400000);
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif

  Serial.begin(500000);
  Serial.setTimeout(10);
  while (!Serial);
  // wait for ready
  while (Serial.available() && Serial.read()); // empty buffer
  delay(100);
  PTLF("\n*Start*");
  assignSkillAddressToOnboardEeprom();
  PTL();

  // servo initialization
  pwm.begin();

    pwm.setPWMFreq(60 * PWM_FACTOR); // Analog servos run at ~60 Hz updates
    delay(200);

    // initialize IR
    irrecv.enableIRIn(); // Start the receiver

    for (int8_t i = DOF - 1; i >= 0; i--) {
      pulsePerDegree[i] = float(PWM_RANGE) / servoAngleRange(i);
      servoCalibs[i] = servoCalib(i);
      calibratedDuty0[i] =  SERVOMIN + PWM_RANGE / 2 + float(middleShift(i) + servoCalibs[i]) * pulsePerDegree[i]  * rotationDirection(i) ;
      //PTL(SERVOMIN + PWM_RANGE / 2 + float(middleShift(i) + servoCalibs[i]) * pulsePerDegree[i] * rotationDirection(i) );
 //     calibratedPWM(i, motion.dutyAngles[i]);
      delay(100);
    }
    randomSeed(analogRead(0));//use the fluctuation of voltage caused by servos as entropy pool

  //****end servo initialize
  beep(30);

  pinMode(BATT, INPUT);
  pinMode(VCC, OUTPUT);
  pinMode(TRIGGER, OUTPUT); // Sets the trigPin as an Output
  pinMode(ECHO, INPUT); // Sets the echoPin as an Input
  digitalWrite(VCC, HIGH);
  float voltage = analogRead(BATT);
  PT("voltage=");
  PTL(voltage);

  PTL("started");
  resetMotorsToRest();
  mpuSetup();   // initialize the MPU. Call after Nybble is in rest position. This does not involve Rowling's MPU library.
}

//***********************************************************************************************
byte GetCmd()
{
    decode_results results;      // create instance of 'decode_results'
    byte iRet = CMD_INVALID;
    if (irrecv.decode(&results)) {  

      switch (results.value) {
          //IR signal    key on IR remote       //abbreviation of gaits   //gait/posture names
        case 0xFFA25D:                        iRet = CMD_SIT;   break;
        case 0xFF629D:                        iRet = CMD_REST;  break;         //shutdown all servos
        case 0xFFE21D:                        iRet = CMD_HI;    break;         //greetings
    
        case 0xFF22DD:                        iRet = CMD_TURNAROUND; break;    //turn around
        case 0xFF02FD:                        iRet = CMD_BALANCE;    break;    //neutral standing
        case 0xFFC23D:                        iRet = CMD_DEBUG;      break;
    
        case 0xFFE01F:                        iRet = CMD_ZERO;  break;   // go to zero settings         //
        case 0xFFA857:                        iRet = CMD_TROT;  break;        //trot
//        case 0xFF906F:                        return("pu");         // push up
    
        case 0xFF6897:                        iRet = CMD_MY_WALK;    break;       // try for a more stable walk
        case 0xFF9867:                        iRet = CMD_WALK;         break;       //walk
        case 0xFFB04F:                        iRet =  CMD_WALK_RIGHT;  break;       //walk right
    
        case 0xFF30CF:                        iRet = CMD_BACK_LEFT;  break;        //back left
        case 0xFF18E7:                        iRet = CMD_BACK;       break;        //back 
        case 0xFF7A85:                        iRet = CMD_BACK_RIGHT; break;        //back right
    
        case 0xFF10EF:                        iRet = CMD_SIT_MEOW;               break;        // sit, then meow
        case 0xFF38C7:                        iRet = CMD_RESET_GYRO;             break;        //kludge: reset gyro until a better solution is found
        case 0xFF5AA5:                        iRet = CMD_WALK_TURNAROUND_SAY_HI; break;        //walk, turn around then say hi
    
        case 0xFF42BD:                        iRet = CMD_SLOWER;  break;     //increase the number of steps per skill entry
        case 0xFF4AB5:                        iRet = CMD_MEOW;    break;       //customed skill
        case 0xFF52AD:                        iRet = CMD_FASTER;  break;      // decrease the number of steps per skill entry
    
        default: {
            Serial.println(results.value, HEX);
          }
      }// End Case
      if (iRet != CMD_INVALID) {
        PT("command=");
        Serial.println(results.value,HEX); 
        PTL(" ");
      }
      irrecv.resume(); // receive the next value //****debug: put this back if decode() is changed back
    } // End if (irrecv.decode)
    return iRet;
}

//*************************************************************************************************
// Initialize a sequence of commands
void runSequence(int sNo)
{  
    seqNo = sNo;
    seqStep = 0;
    bInSequence = true;
    bIntrinsicLoaded = false;
    nSequence = pgm_read_byte(&(sequence[seqNo].nSequence));
}

//*************************************************************************************************
void loadCommon()
{
    for (int i=0; i<8; i++) 
      previousAngle[i] = mostRecentAngle[i];
    unsigned long tCur = millis();
    if (bRunning == false) {
      resetGyro(0);          // if Nybble was not in motion reset the Gyros to clear old values
      bRunning = true;
    }
    tStart = tPrev = tCur;
    prevcnt = loopcnt;
    tPrevMotorCall = tCur; // Time of last motor update
    if (motion.period == 1) {
         firstMotionJoint = DOF - WALKING_DOF;
         calibratedPWM(0,motion.dutyAngles[0]);    // Head
         calibratedPWM(1, motion.dutyAngles[0]);   // neck
         calibratedPWM(2, motion.dutyAngles[0]);   // tail
    }
    else {
        firstMotionJoint = 0;
         calibratedPWM(0,0);   // Head  For walking, head, neck, tail go to zero position (straight ahead)
         calibratedPWM(1,0);   // neck
         calibratedPWM(2,0);   // tail
    }
    #ifdef POSTURE_WALKING_FACTOR
        postureOrWalkingFactor = (motion.period == 1 ? 1 : POSTURE_WALKING_FACTOR);  // if walking, use a 0.5 factor for roll adjustment
    #endif
    iDuty = 0;
    iStep = 1;  
}

//*************************************************************************************************
void loadIntrinsic(unsigned int addr) 
{
    motion.loadDataFromI2cEeprom(addr); 
    loadCommon();
}

//*************************************************************************************************
void loadNewbility(unsigned int addr) 
{
    motion.loadDataFromProgmem(addr); 
    loadCommon();
}

//*************************************************************************************************
// continue the sequence.  I.E.  decide if the next step in the sequence is to be run, or if the 
// sequence is finished.
void continueSequence()
{
  unsigned long tCur = millis();
  if (tCur >= tStopTime) {
    tStopTime = 0xffffffff;
    bIntrinsicLoaded = false;
  }
  else if (tStopTime != 0xffffffff) return;  // wait for tStopTime
  // byte nSequence = pgm_read_byte(&(sequence[seqNo].nSequence));
  if (seqStep >= nSequence && tStopTime == 0xffffffff) {
    bRunning = false;
    bInSequence = false;
  }

  SEQUENCE_ENTRY *entry = pgm_read_word(&sequence[seqNo].entry);
  byte type = pgm_read_byte(&(entry[seqStep].type));
  void (*subr)(int) = pgm_read_word(&(entry[seqStep].subr));
  unsigned int addr = pgm_read_word(&(entry[seqStep].address));
  if (tStopTime == 0xffffffff) {
    tStopTime = tCur + pgm_read_word(&entry[seqStep].wait);
  }
  
  if (type == CMD_SIMPLE) {
    (*subr)(addr);
    seqStep++;
  }
  else if (type == CMD_INTRINSIC_SKILL && !bIntrinsicLoaded) {
    bIntrinsicLoaded = true;
    loadIntrinsic(addr);
    seqStep++;
  } 
}

//*************************************************************************************************
bool powerIsGood()
{
  float voltage = analogRead(BATT);
  if (voltage < 300)  {  // for nyBoard v0, use 650, for nyBoad v1 use 300
    PT("voltage is low: ");
    PTL(voltage);//relative voltage
    meow();
    return false;
  }
  return true;
}

bool bFirstLoop = false;  //******debug: set true while in the first loop through a skill (to limit printing)

//****************************************************************************************************
// Debug function to print whatever is of current interest.  Typically called when bCmdDebug is true
void prtit()
{
  PT("stepsPerDuty=");
  PT(stepsPerDuty);
  PT(" prev: ");
  PTL(" ");
  bFirstLoop = true;
}

//***********************************************************************************************
void loop() {
  loopcnt++;
  if (!powerIsGood()) return;   // Do nothing but meow if power is low
  unsigned long tCur = millis();  // This can be used for many things

  byte cmd;
  cmd = GetCmd();

  if (bNeedToResetGyro) {  // recover balance if something went wrong (Nybble fell over?)
    resetMotorsToRest();
    return;
  }
  if (cmd != CMD_INVALID) {
    byte cmdType = pgm_read_byte(&(cmdInfo[cmd].type));
    void (*subr)(int) = pgm_read_word(&(cmdInfo[cmd].subr));
    unsigned int addr = pgm_read_word(&(cmdInfo[cmd].address));

    if (cmdType == CMD_SIMPLE) {
      bInSequence = false;
      (*subr)(addr);
    }
    else if (cmdType == CMD_INTRINSIC_SKILL) {
      loadIntrinsic(addr);
      bFirstLoop = true;   //****debug
      PTL(" done loading");
    }
    else if (cmdType == CMD_NEWBILITY_SKILL) {
      PTL("newbility"); //***debug
      loadNewbility(addr);
      PTL(" done loading");
    }
    else if (cmdType == CMD_SEQUENCE) {
      for (int i=0; i<8; i++) 
        previousAngle[i] = mostRecentAngle[i];
      runSequence(addr);
      tPrevSeq = tCur;
    }
  }

  if (bInSequence) {
    if (tCur >= tPrevSeq + 40) {  // Don't allow sequence commands more often then every 40ms
      tPrevSeq += 40;
      continueSequence();
    }
  }

  if(bRunning) {
    if (tPrevMotorCall + tMotorWait <= tCur) {
      checkBodyMotion();  // get the roll and pitch, just before it will be used
      tPrevMotorCall += tMotorWait;
      for (int i=0; i<8; i++) {
         int dutyIdx = iDuty * WALKING_DOF + i + firstMotionJoint; 
         float prevAngle = previousAngle[i];
         float targetAngle = motion.dutyAngles[dutyIdx];
         float stepAngle = iStep * (targetAngle-prevAngle)/stepsPerDuty;

         float adj = adjust(i + DOF-WALKING_DOF); 

         mostRecentAngle[i] = prevAngle+stepAngle;       //save most recent angle for this motor (used below) 
         calibratedPWM(i + DOF-WALKING_DOF, prevAngle + stepAngle + adj); 
      }

      iStep++;
      if (iStep > stepsPerDuty) {
        for (int i=0; i<8; i++) 
          previousAngle[i] = mostRecentAngle[i];
        iDuty++;
        iStep = 1;
        if (iDuty == motion.period) iDuty = 0;
        bFirstLoop = false;
      }
    }
  }

//   print how many times loop() is called per second
  if (tCur >= tPrev+1000) {
    tPrev += 1000;
    PT("Number of loops = ");
    PT(loopcnt-prevcnt);
    PT(" iDuty: ");
    PTL(iDuty);
    prevcnt = loopcnt;
  } 
}
