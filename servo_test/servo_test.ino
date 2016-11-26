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
// array to include positons for smooth servo motion (may need multiple depending on the scalability of the array for different speeds)
const unsigned char servo_positions[4] = {10,20,30};
// array of standstill positions (index = pin number)
const unsigned char standstill_positions[4] = {1,2,3};

//movement array(s)
#define movement_30deg_2slow_size 36
int movement_positions_30deg_2slow[movement_30deg_2slow_size + 1] = {0,1,1,2,2,3,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,27,28,28,29,29,30};
#define movement_30deg_size 30
int movement_positions_30deg[movement_30deg_size + 1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};


/***********************************************************************************************************************************************************/
/* HEXAPOD CLASS *******************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
class hexapod{
  // declare member varibles and initialise servo class
  unsigned char servo_pin;
  unsigned char servo_phase;
  volatile int position_index;
  volatile char index_increment;
  
  Servo servo;

  public:
  // class constructor (initialises member variables for particular instances of class, this can include the pin the servo is
  //    connected and maybe even the position on the hexapod for further expandibility)
  hexapod(int pin){
    servo_pin = pin;
    position_index = 0;
    index_increment = 1;
  }
  // setup member function
  void begin(){
    servo.attach(servo_pin);
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
    servo.write(standstill_positions[servo_pin]);
  }
  // member function to set all the servos to their starting positions for the given movement
  void start_pos(){
    // set servo to position indicated in matrix of starting positions
    servo.write(servo_positions[position_index]);
  }
  // servo update member function
  void update(){
    // increment the index variable and update the servo to the corresponding position as described in the servo position array
    position_index+= index_increment;
    if((position_index == movement_30deg_size)|(position_index == 0)){
      index_increment = -index_increment;
    }
    servo.write(movement_positions_30deg[position_index]);    
  }
};

// setup hexapod class on pin 5
hexapod test_servo(5);

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
  test_servo.set_direction(0);
}

void setup() {  
  // setup servos
  test_servo.begin();
  // put in standstill position
  //test_servo.standstill();
  //delay(10000);
  //setMovement(FORWARDS, 7);
  //test_servo.start_pos();

  /*
  // INTERRUPTS!!
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
  // continuously update the servo
  test_servo.update();
  delay(3);
}
