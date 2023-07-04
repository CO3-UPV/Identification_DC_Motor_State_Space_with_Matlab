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

// Reference - 88 valores - input cada 1 seg
int16_t input_steps[] = {0,0,0,0,0,512,512,512,512,512,512,0,0,0,0,0,1024,1024,1024,1024,1024,1024,0,0,0,0,0,1536,1536,1536,1536,1536,1536,0,0,0,0,0,2048,2048,2048,2048,2048,2048,0,0,0,0,0,2560,2560,2560,2560,2560,2560,0,0,0,0,0,3072,3072,3072,3072,3072,3072,0,0,0,0,0,3584,3584,3584,3584,3584,3584,0,0,0,0,0,4096,4096,4096,4096,4096,4096};
int counter = 0; // 40 veces por input
int max_counter = 40; // 40 veces por input
int counter_steps = 0;
int max_counter_steps = 88;
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
    U_int = input_steps[counter_steps];
    counter = 0;
    if(counter_steps == max_counter_steps) {
      U_int = 0;
      if(finish == 1) while(1);
      finish = 1;
    } else  counter_steps++;
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
