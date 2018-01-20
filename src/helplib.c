#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "helplib.h"

void assert(int cond, char * msg){
  if(!cond){
    perror(msg);
    exit(1);
  }
}

void test_save_map(){
	int file = open("maps/map_blocks.save", O_RDONLY);
	int n;
	char q;
	for(int i=0 ; i<10 ; i++){
    	read(file, &n, sizeof(int));
    	while(n<48 && n>57){
      		printf("%d", n);
      		fflush(0);
    	}
    	for(int j=0 ; j<n ; j++){
      		read(file, &q, 1);
      		write(1, &q, 1);
    	}
    	for(int k=0 ; k<5 ; k++){
      		read(file, &n, sizeof(int));
      		printf(" %5d ", n);
      		fflush(0);
    	}
    	printf("\n");
  	}
}