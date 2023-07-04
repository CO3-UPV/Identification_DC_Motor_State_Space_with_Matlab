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
int16_t input_chirp[] = {0,0,0,0,0,0,0,0,1,1,2,2,3,5,6,8,10,13,17,21,25,31,37,44,52,61,72,83,96,111,127,144,163,184,207,232,260,289,321,355,391,430,472,517,564,614,667,723,783,845,910,979,1050,1125,1202,1283,1366,1453,1542,1633,1728,1824,1923,2023,2125,2229,2333,2439,2545,2651,2757,2862,2966,3069,3170,3268,3363,3455,3543,3627,3705,3778,3844,3904,3957,4002,4039,4067,4086,4095,4094,4084,4062,4030,3987,3932,3867,3790,3703,3605,3496,3377,3249,3112,2966,2813,2653,2488,2317,2144,1968,1791,1614,1439,1267,1100,940,787,644,511,391,285,194,119,61,22,2,3,23,65,127,209,312,434,575,733,908,1096,1297,1508,1728,1952,2180,2407,2632,2850,3060,3258,3441,3606,3751,3873,3971,4041,4083,4096,4078,4030,3951,3842,3705,3540,3351,3139,2908,2662,2403,2137,1868,1600,1339,1089,855,641,452,291,163,71,16,0,25,91,196,340,520,733,975,1241,1526,1824,2128,2433,2730,3013,3275,3510,3712,3876,3997,4071,4096,4071,3995,3869,3697,3482,3228,2942,2632,2304,1968,1632,1306,999,720,478,279,130,36,0,25,111,255,454,703,995,1321,1671,2035,2401,2757,3091,3393,3651,3856,4002,4081,4091,4031,3901,3705,3449,3143,2795,2419,2028,1636,1258,908,599,345,156,39,0,42,164,361,628,953,1325,1728,2145,2560,2955,3313,3618,3856,4017,4091,4075,3969,3777,3505,3166,2775,2349,1907,1470,1059,693,391,168,36,1,68,232,489,824,1223,1665,2128,2589,3023,3408,3722,3948,4073,4089,3995,3794,3496,3117,2678,2201,1715,1246,821,465,200,42,2,82,279,582,975,1434,1932,2439,2923,3354,3705,3952,4080,4079,3947,3693,3331,2884,2381,1854,1339,870,478,192,31,8,125,377,745,1206,1728,2274,2805,3283,3674,3948,4083,4070,3907,3605,3186,2680,2125,1563,1036,586,247,48,4,121,391,793,1297,1862,2444,2996,3471,3831,4044,4092,3969,3685,3262,2735,2148,1550,994,528,194,20,25,209,557,1040,1614,2229,2829,3359,3770,4024,4094,3975,3674,3220,2653,2028,1403,838,389,100};
int counter = 0; // 40 veces por input
int max_counter = 40; // 40 veces por input
int counter_chirp = 0;
int max_counter_chirp = 400;
int finish = 0;

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

  Serial.println("timestamp_ms,U,max_voltage_V,pos_rad,vel_rads,current_mA");
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
      if(finish == 1) while(1);
      finish = 1;
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
  Serial.print(pos_rad);
  Serial.print(",");
  Serial.print(speed_rads);
  Serial.print(",");
  Serial.print(wattmetter.getCurrent_mA());
  Serial.println();
}
