// C program to caculate the movement positions for a servo motor to use on 
// hexpod robot

#include <stdio.h>

// define constants for creating the array of movement
#define ANGLE			30
#define SLOW_PERCENTAGE		10
#define SLOW_SPEED		2

#define ARRAY_SIZE		(1 + ANGLE + (((SLOW_PERCENTAGE*ANGLE)/100)*SLOW_SPEED))

int movement_array[ARRAY_SIZE];
int lower_bound, upper_bound;
int array_index = 0;

int main(){
	lower_bound = (ANGLE*SLOW_PERCENTAGE)/100;
	upper_bound = ANGLE - lower_bound;

	int n;


	for(n = 0; n<=ANGLE; n++){
		if((n==0) | (n==ANGLE)){
			movement_array[array_index] = n;
			array_index++;
		}
		else if((n <= lower_bound) | (n >= upper_bound)){
			movement_array[array_index] = n;
			array_index++;
			movement_array[array_index] = n;
			array_index++;
		}
		else{
			movement_array[array_index] = n;
			array_index ++;
		}
	}
	//printf("Array size = %d, array index = %d\n", ARRAY_SIZE, array_index);

	printf("\n#define movement_%ddeg_size %d\nint movement_positions_%ddeg[movement_%ddeg_size] = {", ANGLE, ARRAY_SIZE, ANGLE, ANGLE);

	for(n = 0; n<= ARRAY_SIZE; n++){
		printf("%d,", movement_array[n]);
	}

	printf("\b};\n\n");
}


