#include <Adafruit_MotorShield.h>
#include <DFRobot_INA219.h>
#include <Encoder.h>

#define CPR 4480.0f
#define REV2RAD 2.0f*PI

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *motor = AFMS.getMotor(1);

DFRobot_INA219_IIC wattmetter(&Wire, INA219_I2C_ADDRESS4);

int32_t counts_prev = 0;
int32_t counts_act = 0;
float counts_delta = 0;
float pos_rad = 0;
float speed_rads = 0;
Encoder encoder(2, 3);

float dt = 0.025f;
unsigned long timestamp = 0;
unsigned long period_ms = 25; // Freq 40 Hz

// Input Voltaje Average
float input_voltage = 0;
int16_t U_int = 0;
int16_t U = 0;

// Reference - 400 valores - input cada 1 seg
int16_t input_chirp[] = {0,-2,-6,-14,-26,-40,-58,-79,-103,-130,-161,-195,-231,-272,-315,-361,-411,-464,-520,-579,-641,-706,-774,-845,-919,-995,-1075,-1157,-1241,-1328,-1418,-1509,-1603,-1699,-1796,-1895,-1996,-2097,-2200,-2304,-2408,-2512,-2616,-2720,-2823,-2925,-3025,-3124,-3221,-3315,-3406,-3493,-3577,-3656,-3730,-3799,-3862,-3919,-3969,-4011,-4046,-4071,-4088,-4096,-4093,-4080,-4056,-4021,-3974,-3914,-3843,-3758,-3661,-3551,-3427,-3290,-3140,-2976,-2799,-2610,-2408,-2193,-1968,-1731,-1484,-1227,-962,-690,-411,-127,161,451,742,1033,1321,1605,1882,2152,2413,2661,2896,3116,3318,3500,3661,3799,3913,4000,4060,4092,4093,4063,4003,3911,3787,3631,3445,3228,2981,2708,2408,2084,1738,1374,994,601,199,-207,-615,-1020,-1418,-1803,-2173,-2522,-2846,-3141,-3402,-3627,-3811,-3951,-4046,-4092,-4088,-4032,-3926,-3769,-3561,-3305,-3003,-2659,-2276,-1858,-1412,-942,-456,40,539,1033,1514,1975,2408,2805,3160,3466,3717,3908,4034,4092,4080,3997,3843,3619,3329,2976,2566,2106,1603,1067,507,-66,-641,-1206,-1750,-2261,-2728,-3141,-3489,-3765,-3961,-4071,-4093,-4023,-3862,-3613,-3279,-2868,-2387,-1847,-1260,-639,0,642,1272,1872,2428,2925,3347,3685,3926,4064,4093,4011,3818,3518,3119,2629,2063,1434,761,63,-641,-1328,-1979,-2572,-3089,-3513,-3829,-4026,-4096,-4035,-3843,-3525,-3089,-2550,-1922,-1227,-488,272,1025,1745,2408,2987,3462,3814,4028,4096,4012,3779,3402,2895,2276,1566,793,-14,-824,-1605,-2324,-2952,-3462,-3832,-4046,-4091,-3966,-3672,-3221,-2629,-1922,-1129,-283,579,1418,2196,2878,3431,3829,4052,4086,3929,3586,3071,2408,1625,761,-143,-1044,-1895,-2655,-3284,-3749,-4024,-4093,-3950,-3602,-3063,-2360,-1530,-615,336,1272,2142,2896,3493,3898,4084,4041,3769,3279,2600,1767,829,-161,-1144,-2063,-2861,-3489,-3908,-4089,-4021,-3703,-3155,-2408,-1506,-507,528,1532,2440,3193,3739,4041,4078,3843,3350,2631,1731,711,-361,-1412,-2367,-3160,-3734,-4046,-4071,-3806,-3267,-2490,-1530,-456,655,1721,2661,3406,3896,4093,3979,3561,2868,1951,879,-264,-1389,-2408,-3237,-3811,-4079,-4019,-3631,-2946,-2017,-919,259,1418,2461,3299,3859,4091,3973,3512,2746,1738,576,-641,-1803,-2809,-3565,-4003,-4080,-3787,-3147,-2216,-1079,161,1389,2490,3359,3913,4096,3887,3305,2402,1264};
int counter = 0; // 40 veces por input
int max_counter = 40; // 40 veces por input
int counter_chirp = 0;
int max_counter_chirp = 400;

void setup() {
  Serial.begin(115200);
  

  if (!AFMS.begin()) {
    Serial.println("[Error] Could not find Adafruit Motor Shield V2. Check wiring.");
    while (1);
  }

  if (!wattmetter.begin()) {
    Serial.println("[Error] Could not find Gravity I2C digital power meter. Check wiring.");
    while (1);
  }

  motor->setSpeedFine(0);
  motor->run(RELEASE);

  // Un segundo haciendo la media de voltaje de la fuente
  for (int i = 0; i < 1000; i++) {
    input_voltage += wattmetter.getBusVoltage_V();
    delay(10);
//    Serial.print(".");
  }
//  Serial.println();

  input_voltage /= 1000;

//  Serial.print(input_voltage, 2);
//  while(1);

  counts_act = encoder.read();
  timestamp = millis() + period_ms;

  Serial.println("timestamp,U,max_voltage_V,current_mA,pos_rad,vel_rads");
}

void loop() {

  while (timestamp > millis()) {}
  timestamp += period_ms;

  if(counter == max_counter)
  {
    U_int = input_chirp[counter_chirp];
    counter = 0;
    if(counter_chirp == max_counter_chirp) {
      U_int = 0;
    } else  counter_chirp++;
  }
  counter++;

  if(U_int < 0) {
    motor->run(FORWARD);
  } else {
   motor->run(BACKWARD);
  }

  U = abs(U_int);

  motor->setSpeedFine(U);
  
  counts_prev = counts_act;
  counts_act = encoder.read();
  counts_delta = counts_act - counts_prev;
  pos_rad = ((counts_act/CPR) * REV2RAD);
  speed_rads = ((counts_delta/CPR) * REV2RAD) / dt;

  Serial.print(timestamp);
  Serial.print(",");
  Serial.print(U_int);
  Serial.print(",");
  Serial.print(input_voltage);
  Serial.print(",");
  Serial.print(wattmetter.getCurrent_mA());
  Serial.print(",");
  Serial.print(pos_rad);
  Serial.print(",");
  Serial.print(speed_rads);
  Serial.println();
}
