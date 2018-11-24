#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]){
    // randomize rand with timed seed
    srand(time(NULL));

    // verify exactly one argument provided
    if(argc != 2){
        fprintf(stderr, "invalid number of arguments provided\n");
        return 1;
    } else {
        int keySize = atoi(argv[1]);
        int charOut = -1;
        int i = -1;
        // make a key of size given by keySize
        for(i = 0; i<keySize; ++i){
            charOut = ( (rand() % 27) + 64 );
            // 64 is reserved for 'space'
            if(charOut == 64){
                charOut = 32;
            }
            fprintf(stdout, "%c", charOut);
        }
    }
    fprintf(stdout, "\n");
    return 0;
}