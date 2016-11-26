/*
 * Servo Testing sketch for new code design of hexapod
 * structure:
 *  - Interrupt triggered by the millis timer (timer 0 i think)
 *        - ISR updates the position of the servo
 *  - Use a class to control/update the servos
 *        - Initialiser to setup variables, input = pin number and phase shift
 *        - Setup function to setup the servo (and detach??)
 *        - function to put servo into starting position
 *        - Update function to update the servo position depending on the current time
 *  - Need to test with servo to create an array of positions which when gone through
 *      at 1 millis intervals (or other time period) results in smooth (as poss) motion
 *      - will need to measure speed of the servo movement
 *      - use excel to generate some data to use on servos
 *  - Extra functionality:
 *      - Ability to control the speed of the servos/walking
 *      - fowards/backwards/turning motions
 *      - maybe include expansion/generalisation for the movement angles??
 *      
 *      for servo from arduino starter kit, 180 degrees in 400ms (ish) = 0.45 degrees/ms = 2.22ms/degree so max speed when delay of 3ms between updates
 */

 /* Naming conventions and walking patterns
//
// Naming Convention for servos: (pan servos 1-6, lift servos 7-12, even = RHS, odd = LHS)
//  - FrontLeftPan = 1
//  - FrontRightPan = 2
//  - MidLeftPan = 3
//  - MidRightPan = 4
//  - BackLeftPan = 5
//  - BackRightPan = 6
//  - FrontLeftLift = 7
//  - FrontRightLift = 8
//  - MidLeftLift = 9
//  - MidRightLift = 10
//  - BackLeftLift = 11
//  - BackRightLift = 12

// For RHS servos: adding an angle = lower and forwards
// For LHS servos: adding angle = higher and backwards

// Forwards Walk Pattern:
//  - Lift FL(+), MR(-) and BL(+) legs and pan forwards (-,+,-)
//  - Pan FR(-), ML(+) and BR(-) legs backwards
//  - Lower FL(-), MR(+) and BL(-) legs
//  - Lift FR(-), ML(+) and BR(-)legs and pan forwards (+,-,+)
//  - Pan FL(+), MR(-) and BL(+) legs Backwards
//  - Lower FR(+), ML(-) and BR(+) legs
//  - LOOP!!

// Backwards is the same except bottom to top and backwards:
//  - Lift FR(-), ML(+) and BR(-) legs
//  - Pan FL(-), MR(+) and BL(-) legs forwards
//  - lower FR(+), ML(-) and BR(+)legs and pan backwards (-,+,-)
//  - Lift FL(+), MR(-) and BL(+) legs
//  - Pan FR(+), ML(-) and BR(+) legs forwards
//  - Lower FL(-), MR(+) and BL(-) legs and pan backwards (+,-,+)
//  - LOOP!!
*/


 
// direction/movement definitions
 #define FORWARDS   1
 #define BACKWARDS  2
 #define TURN_CW    3
 #define TURN_CCW   4

// library includes
#include <Servo.h>

// global variables
unsigned char direction;
unsigned char speed;

int servoPins[] = {    // in order of servos(1 to 12)
  /*FLP*/ 12,  /*FRP*/ 13,  /*MLP*/ 8,  /*MRP*/ 9,  /*BLP*/ 4,  /*BRP*/ 5,
  /*FLL*/ 10,  /*FRL*/ 11,  /*MLL*/ 6,  /*MRL*/ 7,  /*BLL*/ 2,  /*BRL*/ 3
};

// array of default servo positions
int defaults[] = {    // in order of servos(1 to 12)
  78,  101,  78,  105,  78,  111,  97,  88,  95,  91,  100,  91
};

// array containing the members of group 1
int group1_array[] = {1, 2, 5, 7, 8, 11};

//movement array(s)
// array for 30 degrees of movement with the extreme 10% of travel at half speed
//#define MOVEMENT_ARRAY_SIZE 36
//int movement_array[MOVEMENT_ARRAY_SIZE + 1] = {0,1,1,2,2,3,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,27,28,28,29,29,30};
// array for 30 degrees of movement with no slowing down
#define MOVEMENT30_ARRAY_SIZE 30
int movement30_array[MOVEMENT30_ARRAY_SIZE + 1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};
// array for 15 degrees of movement with no slowing down
#define MOVEMENT15_ARRAY_SIZE 15
int movement15_array[MOVEMENT15_ARRAY_SIZE + 1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};


/***********************************************************************************************************************************************************/
/* HEXAPOD CLASS *******************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
class hexapod{
  // declare member varibles and initialise servo class
  unsigned char servo_phase;
  unsigned char servo_number;
  unsigned char default_position;
  volatile int position_index;
  volatile char index_increment;
  char polarity;
  boolean pan = false;
  boolean lift = false;
  boolean group1 = false;
  boolean group2 = false;
  boolean left = false;
  boolean right = false;
  
  Servo servo;

  public:
  // class constructor (initialises member variables for particular instances of class, this can include the pin the servo is
  //    connected and maybe even the position on the hexapod for further expandibility)
  hexapod(void){
    position_index = 0;
    index_increment = 1;
    polarity = 1;
  }
  // setup member function
  void begin(int number){
    servo_number = number;
    servo.attach(servoPins[servo_number]);
    default_position = defaults[servo_number];

    // determine characteristics of the servo being setup based on the number
    // lift/pan
    if(servo_number > 5){
      lift = true;
    }
    else{
      pan = true;
    }
    // group 1 or 2
    for(int n = 0; n < 6; n++){
      if(servo_number == group1_array[n]){
        group1 = true; 
      }
    }
    if(!group1){
      group2 = true;      
    }
    // left/right
    if((servo_number % 2) == 0){
      left = true;
    }
    else{
      right = true;
    }
    // determine polarity
    // if lift servo reverse polarity
    if(lift){
      polarity = -polarity;
    }
    // if servo is on left hand side reverse polarity
    if(left){
      polarity = -polarity;
    }

    // set phase (likely go elsewhere in future tests)
    if(group1){
      index_increment = -1;
      if(pan){
        position_index = 30;
      }
    }
  }
  void set_direction(int phase){
    // standstill member function to set servo to standstill postion
    servo_phase = phase;
    // initialise the position index variable based on the phase
    position_index = phase;
    // also set counting direction
    index_increment = 1;
  }
  void standstill(){
    // set servo to standstill position
    servo.write(default_position);
  }
  // member function to set all the servos to their starting positions for the given movement
  void start_pos(){
    // set servo to position indicated in matrix of starting positions
    if(pan){
      servo.write(default_position + polarity*(movement30_array[position_index] - 15));
    }
    else{
      servo.write(default_position);
    }
  }
  // servo update member function
  void update(){
    if(pan){
      servo.write(default_position + polarity*(movement30_array[position_index] - 15)); 
      // increment the index variable and update the servo to the corresponding position as described in the servo position array
      position_index+= index_increment;
      if((position_index == MOVEMENT30_ARRAY_SIZE)|(position_index == 0)){
        index_increment = -index_increment;
      }
    }
    else{
      if(position_index < 0){
        servo.write(default_position);
      }
      else{
        servo.write(default_position + polarity*(movement15_array[position_index])); 
      }
      // increment the index variable and update the servo to the corresponding position as described in the servo position array
      position_index+= index_increment;
      if((position_index == MOVEMENT15_ARRAY_SIZE)|(position_index == -MOVEMENT15_ARRAY_SIZE)){
        index_increment = -index_increment;
      }
    }
  }
};

// setup hexapod class
hexapod hexapod_servos[12];

// function which will setup the required global variables for forward/reverse/turning (likely the position index among others)
//    and also update the phases required for the individual servos
void setMovement(int movement, int movement_speed){
  switch(movement){
    case FORWARDS:
    {
      // forwards motion
    }
    break;
    case BACKWARDS:
    {
      // backwards motion
    }
    break;
    case TURN_CW:
    {
      // turn clockwise
    }
    break;
    case TURN_CCW:
    {
      // turn counter-clockwise
    }
    break;
    default:
    {
      // default behaviour
    }
    break;
  }
  //test_servo.set_direction(0);
}

void setup() { 
  Serial.begin(9600); 
  // setup servos
  for(int n = 0; n < 12; n++){
    hexapod_servos[n].begin(n);
    hexapod_servos[n].standstill();
  }
  // put in standstill position
  //test_servo.standstill();
  delay(1000);
  //setMovement(FORWARDS, 7);
  for(int n = 0; n < 12; n++){
    hexapod_servos[n].start_pos();
  }
  delay(1000);

  /* INTERRUPTS!!
  // setup the compare match register on timer 0 to interrupt half way between the millis interrupts and enable timer interrupt
  //    - may need to use different timer if one millisecond is too short a time period = very likely
  OCR0A = 0x7F;
  TIMSK0 |= (1<<OCIE0A);
  */
} 

/*
// ISR for timer interrupt
SIGNAL(TIMER0_COMPA_vect){
  test_servo.update();
}
*/

void loop() {
  // continuously update the servos
  for(int n = 0; n < 12 ; n++){
    hexapod_servos[n].update();
  }
  delay(5);
}

void serialEvent(){
  Serial.println(Serial.read());
}

