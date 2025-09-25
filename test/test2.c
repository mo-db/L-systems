#include <stdio.h>


// The reason is the internal representation of signed numbers
// 	-> see two's complement
void add() {
	int addend1 = 10, addend2 = -11;
	unsigned int result = addend1 + addend2;
	printf("The result is %u", result);
}

// cast also truncate
void cast() {
	float initialValue = 250.789655;
	int targetValue;
	targetValue = (int) initialValue;
	printf("The result is %u", targetValue);
}

// dividing integers -> remainder is truncated
void divide() {
	// int a = 5;
	// float b = a;
	float c = 5.9723;
	int d = (int)c;


	bool result = a == b;
	if (!result) {
		printf("a is bigger than b\n");
	}
	// int result = a / b;
	// int result2 = a % b;
	// printf("The result is %u\n", result);
	// printf("The result2 is %u\n", result2);
}


int main() {
	// add();
	divide();
	// cast();
}



// there are dedicated functions that implement rounding
// 	-> see round(), floor(), etc...

// 9.6.4
// write 3 functions for those mathematical expressions
// if you run the program, input, print all 3 results
