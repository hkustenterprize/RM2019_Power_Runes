#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(&Wire, 0x40);

// Depending on your servo make, the pulse width min and max may vary, you 
// want these to be as small/large as possible without hitting the hard stop
// for max range. You'll have to tweak them as necessary to match the servos you
// have!
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)

// our servo # counter
uint8_t servonum = 0;
uint16_t start_time =0; //should change to 0
bool fan[5] = {false, false, false, false, false};//all false
bool flip = true;
uint8_t randomnum;
uint8_t randomselected= 5;// 5
bool fanpre[5] = {false, false, false, false, false};
uint8_t past_num = -1;

void setup() {
  Serial.begin(9600);
  Serial.println("8 channel Servo test!");

  pwm.begin();
  
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates
  pinMode(LED_BUILTIN, OUTPUT);
  for(uint16_t i=0; i<16; i++)pwm.setPWM(i, 4000, 0);
  delay(10);
}

// you can use this function if you'd like to set the pulse length in seconds
// e.g. setServoPulse(0, 0.001) is a ~1 millisecond pulse width. its not precise!
void setServoPulse(uint8_t n, double pulse) {
  double pulselength;
  
  pulselength = 1000000;   // 1,000,000 us per second
  pulselength /= 60;   // 60 Hz
  Serial.print(pulselength); Serial.println(" us per period"); 
  pulselength /= 4096;  // 12 bits of resolution
  Serial.print(pulselength); Serial.println(" us per bit"); 
  pulse *= 1000000;  // convert to us
  pulse /= pulselength;
  Serial.println(pulse);
  pwm.setPWM(n, 0, pulse);
  randomSeed(analogRead(0));
}

void time_count() {
  Serial.println(start_time);
//  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(500);                       // wait for a second
//  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
//  delay(500); 
  if(flip){
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);                       // wait for a second
    flip = false;
  }
  else{
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(500); 
    flip = true;
    start_time++;
  }
//  Serial.println(random(0,5));
}

void actived() {
  if(((start_time %5 ==0)&&(flip==true))||(start_time %5==2)&&(flip==false)){
    Serial.print("Select");
    randomselected=0;
    for(int i=0;i<5;i++)if(fan[i]==false)randomselected++;
    if(past_num>-1&&randomselected>1)randomselected--;
    else past_num=-1;
    randomnum = random(0,randomselected);
    Serial.println(randomnum);
    for(int i=0;i<5;i++){
      if(fan[i]){
        randomnum++;
        pwm.setPWM(i+1,0,4000);
        pwm.setPWM(i+6,0,4000);
        fanpre[i]=true;
      }
      else if(i==past_num){
        randomnum++;
        pwm.setPWM(i+6,4000,0);
        pwm.setPWM(i+1,4000,0);
        fanpre[i]=false;
      }
      else{
        if(i==randomnum){
          pwm.setPWM(i+6,0,4000);
          pwm.setPWM(i+1,4000,0);
          fanpre[i]=true;
          past_num = i;
        }
        else{
          pwm.setPWM(i+6,4000,0);
          pwm.setPWM(i+1,4000,0);
          fanpre[i]=false;
        }
      }
    }
    Serial.print("fan:  ");
    for(int i=0;i<5;i++)Serial.print(fan[i]);
    Serial.println();
    Serial.print("fanpre: ");
    for(int i=0;i<5;i++)Serial.print(fanpre[i]);
    Serial.println();
  }
}

void loop() {
  // Drive each servo one at a time
  /*Serial.println(servonum);
  for (uint16_t pulselen = SERVOMIN; pulselen < SERVOMAX; pulselen++) {
    pwm.setPWM(servonum, 0, pulselen);
  }

  delay(500);
  for (uint16_t pulselen = SERVOMAX; pulselen > SERVOMIN; pulselen--) {
    pwm.setPWM(servonum, 0, pulselen);
  }

  delay(500);

  servonum ++;
  if (servonum > 7) servonum = 0;*/
//  for (uint16_t i=0; i<16; i++)pwm.setPWM(i, 0, 4000);
  if(start_time<60){
    for(uint16_t i=0; i<16; i++)pwm.setPWM(i, 4000, 0);
    Serial.println("unable to activate");
  }
  else if(start_time >=60 && start_time <180){
    pwm.setPWM(0, 0, 4000);
    Serial.println("able to activate");
    actived();
    Serial.println(past_num);
  }
  else if(start_time >=180 && start_time <240){
    for(uint16_t i=0; i<16; i++)pwm.setPWM(i, 4000, 0);
    for(int i=0;i<5;i++)fan[i]=false;
//    past_num =-1;
    Serial.println("unable to activate");
  }
  else if(start_time >=240 && start_time <420){
    pwm.setPWM(0, 0, 4000);
    Serial.println("able to activate");
    actived();
  }
  else Serial.println("end the game, please reset");
  time_count();
}
