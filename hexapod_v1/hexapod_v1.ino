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
 #define FORWARDS       1
 #define BACKWARDS      2
 #define TURN_CW        3
 #define TURN_CCW       4
 #define STANDSTILL     5
 #define START_POSITION 6

// library includes
#include <Servo.h>

// global variables
unsigned char movement_state;
unsigned char movement_delay = 4;
boolean active = false;
boolean initialised = false;

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
//#define PAN_MOVEMENT_ARRAY_SIZE 30
//int pan_movement_array[PAN_MOVEMENT_ARRAY_SIZE + 1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30};
//// array for 15 degrees of movement with no slowing down
//#define LIFT_MOVEMENT_ARRAY_SIZE 15
//int lift_movement_array[LIFT_MOVEMENT_ARRAY_SIZE + 1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
#define PAN_MOVEMENT_ARRAY_SIZE 40
int pan_movement_array[PAN_MOVEMENT_ARRAY_SIZE + 1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40};
// array for 15 degrees of movement with no slowing down
#define LIFT_MOVEMENT_ARRAY_SIZE 20
int lift_movement_array[LIFT_MOVEMENT_ARRAY_SIZE + 1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};




/***********************************************************************************************************************************************************/
/* HEXAPOD CLASS *******************************************************************************************************************************************/
/***********************************************************************************************************************************************************/
class hexapod{
  // declare member varibles and initialise servo class
  unsigned char servo_phase;
  unsigned char servo_number;
  volatile int position_index;
  volatile char index_increment;
    
  Servo servo;

  public:
  boolean left = false;
  boolean right = false;
  unsigned char default_position;
  char polarity;
  boolean pan = false;
  boolean lift = false;
  boolean group1 = false;
  boolean group2 = false;
  // class constructor (initialises member variables for particular instances of class, this can include the pin the servo is
  //    connected and maybe even the position on the hexapod for further expandibility)
  hexapod(void){
    position_index = 0;
    index_increment = 0;
    polarity = 1;
  }
  // setup member function
  void begin(int number){
    servo_number = number;
    servo.attach(servoPins[servo_number]);
    default_position = defaults[servo_number];
    servo.write(default_position);

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
  }
  void set_direction(int walk_direction){
    initialised = true;
    // reset variables
    position_index = 0;
    index_increment = 1;
    if(group1){
      index_increment = -1;
      if(pan){
        position_index = PAN_MOVEMENT_ARRAY_SIZE;
      }
    }
    if(walk_direction == BACKWARDS){
      if(pan){
        index_increment = -index_increment;
        if(position_index == 0){
          position_index = PAN_MOVEMENT_ARRAY_SIZE;
        }
        else if(position_index == PAN_MOVEMENT_ARRAY_SIZE){
          position_index = 0;
        }
      }
    }
  }
  void servo_write(unsigned char servo_position){
    servo.write(servo_position);
  }
  // member function to set all the servos to their starting positions for the given movement
  unsigned char get_start_pos(){
    // set servo to position indicated in matrix of starting positions
    if(pan){
      return default_position + polarity*(pan_movement_array[position_index] - (PAN_MOVEMENT_ARRAY_SIZE/2));
    }
    else{
      return default_position;
    }
  }
  // servo update member function
  void update(){
    if(pan){
      servo.write(default_position + polarity*(pan_movement_array[position_index] - (PAN_MOVEMENT_ARRAY_SIZE/2))); 
      // increment the index variable and update the servo to the corresponding position as described in the servo position array
      position_index+= index_increment;
      if((position_index == PAN_MOVEMENT_ARRAY_SIZE)|(position_index == 0)){
        index_increment = -index_increment;
      }
    }
    else{
      if(position_index < 0){
        servo.write(default_position);
      }
      else{
        servo.write(default_position + polarity*(lift_movement_array[position_index])); 
      }
      // increment the index variable and update the servo to the corresponding position as described in the servo position array
      position_index+= index_increment;
      if((position_index == LIFT_MOVEMENT_ARRAY_SIZE)|(position_index == -LIFT_MOVEMENT_ARRAY_SIZE)){
        index_increment = -index_increment;
      }
    }
  }
};

// setup hexapod class
hexapod hexapod_servos[12];

// function which will setup the required global variables for forward/reverse/turning (likely the position index among others)
//    and also update the phases required for the individual servos
void setMovement(int movement){
  movement_state = movement;
  if((movement == FORWARDS) | (movement == BACKWARDS)){
    for(int n = 0; n < 12; n++){
      hexapod_servos[n].set_direction(movement);
    }
  }
  else if(movement == TURN_CW){
    for(int n = 0; n < 12; n++){
      if(hexapod_servos[n].left){
        hexapod_servos[n].set_direction(FORWARDS);
      }
      else{
        hexapod_servos[n].set_direction(BACKWARDS);
      }
    }
  }
  else if(movement == TURN_CCW){
    for(int n = 0; n < 12; n++){
      if(hexapod_servos[n].left){
        hexapod_servos[n].set_direction(BACKWARDS);
      }
      else{
        hexapod_servos[n].set_direction(FORWARDS);
      }
    }
  }
}

void setPosition(int servo_position){
  movement_state = servo_position;
  // lower all lift legs
  for(int n = 0; n < 12; n++){
    if(hexapod_servos[n].lift){
      hexapod_servos[n].servo_write(hexapod_servos[n].default_position);
    }
  }
  // delay to let all legs get to position
  delay(60);
  /***** GROUP 1 *********/
  // move group1 legs into position
  // lift legs
  for(int n = 0; n < 12; n++){
    if(hexapod_servos[n].lift & hexapod_servos[n].group1){
      hexapod_servos[n].servo_write(hexapod_servos[n].default_position + (hexapod_servos[n].polarity*(PAN_MOVEMENT_ARRAY_SIZE/2)));
    }
  }
  delay(60);
  // pan group1 legs to standstill position
  if(servo_position == STANDSTILL){
    // group1 lift, pan, drop
    for(int n = 0; n < 12; n++){
      if(hexapod_servos[n].pan & hexapod_servos[n].group1){
        hexapod_servos[n].servo_write(hexapod_servos[n].default_position);
      }
    }
  }
  // pan group1 legs to start position
  else if(servo_position == START_POSITION){
    //stuff here
    for(int n = 0; n < 12; n++){
      if(hexapod_servos[n].pan & hexapod_servos[n].group1){
        hexapod_servos[n].servo_write(hexapod_servos[n].get_start_pos());
      }
    }
  }
  delay(120);
  // lower group1 legs
  for(int n = 0; n < 12; n++){
    if(hexapod_servos[n].lift & hexapod_servos[n].group1){
      hexapod_servos[n].servo_write(hexapod_servos[n].default_position);
    }
  }
  delay(60);

  /***** GROUP 2 *********/
  // move group2 legs into position
  // lift legs
  for(int n = 0; n < 12; n++){
    if(hexapod_servos[n].lift & hexapod_servos[n].group2){
      hexapod_servos[n].servo_write(hexapod_servos[n].default_position + (hexapod_servos[n].polarity*(PAN_MOVEMENT_ARRAY_SIZE/2)));
    }
  }
  delay(60);
  // pan group2 legs to standstill position
  if(servo_position == STANDSTILL){
    // group1 lift, pan, drop
    for(int n = 0; n < 12; n++){
      if(hexapod_servos[n].pan & hexapod_servos[n].group2){
        hexapod_servos[n].servo_write(hexapod_servos[n].default_position);
      }
    }
  }
  // pan group2 legs to start position
  else if(servo_position == START_POSITION){
    //stuff here
    for(int n = 0; n < 12; n++){
      if(hexapod_servos[n].pan & hexapod_servos[n].group2){
        hexapod_servos[n].servo_write(hexapod_servos[n].get_start_pos());
      }
    }
  }
  delay(120);
  // lower group2 legs
  for(int n = 0; n < 12; n++){
    if(hexapod_servos[n].lift & hexapod_servos[n].group2){
      hexapod_servos[n].servo_write(hexapod_servos[n].default_position);
    }
  }
  delay(60);
}

void setup() { 
  Serial.begin(9600); 
  // setup servos
  for(int n = 0; n < 12; n++){
    hexapod_servos[n].begin(n);
  }
  Serial.println("Hexapod Robot MKII\n\nCommands:\n- p = pause\n- c = continue\n- s = standstill\n- f = forwards\n- b = backwards\n- r = turn right (CW)\n- l = turn left (CCW)\n- 0-9 = change speed\n");
} 

void loop() {
  // block if not in active state
  if(active){
    // continuously update the servos
    for(int n = 0; n < 12 ; n++){
      hexapod_servos[n].update();
    }
    delay(movement_delay);
  }
}

void serialEvent(){
  if(Serial.available() > 1){
    Serial.println("Error!! - too many commands");
    // clear buffer
    while(Serial.available()){
      Serial.read();
    }
  }
  else{
    uint8_t data_in = Serial.read();
    if(isdigit(data_in)){
      Serial.print("Changing speed to ");
      Serial.write(data_in);
      Serial.println("");
      movement_delay = 13 - (data_in -48);
    }
    else{
      switch(data_in){
        case 'p':{
          Serial.println("Pause");
          active = false;
          break;
        }
        case 'c':{
          if(movement_state == STANDSTILL){
            Serial.println("Error!! - Can't continue from standstill");
          }
          else if(!initialised){
            Serial.println("Error!! - Not initialised");
          }
          else{
            Serial.println("Continue");
            active = true;
          }
          break;
        }
        case 's':{
          Serial.println("Standstill");
          active = false;
          setPosition(STANDSTILL);
          break;
        }
        case 'f':{
          if(movement_state == FORWARDS){
            Serial.print("Error!! - Already moving forwards");
          }
          else{
            Serial.println("Forwards");
            active = false;
            setMovement(FORWARDS);
            setPosition(START_POSITION);
            active = true;
          }
          break;
        }
        case 'b':{
          if(movement_state == BACKWARDS){
            Serial.print("Error!! - Already moving forwards");
          }
          else{
            Serial.println("Backwards");
            active = false;
            setMovement(BACKWARDS);
            setPosition(START_POSITION);
            active = true;
          }
          break;
        }
        case 'r':{
          if(movement_state == TURN_CW){
            Serial.print("Error!! - Already turning right (CW)");
          }
          else{
            Serial.println("Turn Right (CW)");
            active = false;
            setMovement(TURN_CW);
            setPosition(START_POSITION);
            active = true;
          }
          break;
        }
        case 'l':{
          if(movement_state == TURN_CCW){
            Serial.print("Error!! - Already turning left (CCW)");
          }
          else{
            Serial.println("Turn Left (CCW)");
            active = false;
            setMovement(TURN_CCW);
            setPosition(START_POSITION);
            active = true;
          }
          break;
        } 
        default:{
          Serial.println("Error!! - Invalid Command");
        }
      }
    }
  }
}


