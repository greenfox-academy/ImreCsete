#include <stdio.h>
#include <stdlib.h>

int main()
{
    int i_array[10] = {260, 259, 258, 257, 256, 255, 254, 253, 252, 251};
    unsigned char uc_array[10];

	//TODO: write a program, which copies the values from the given array to an unsigned char array
    //print out the elements of the unsigned char array, with the "%d" format specifier
    //what do you see? - unsigned starts from 0 when it reaches 256

    for (int i = 0; i < 10; i++) {
        uc_array[i] = i_array[i];
    }

    for (int i = 0; i < 10; i++) {
        printf("%u ", uc_array[i]);
    }

    return 0;
}
