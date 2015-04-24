#include "access.h"

/* sequential access */
void sequential_access(float *var, size_t size){
	int i;
	float temp;
	for(i=0;i<size;i++){
		temp = var[i];
		asm("");
	}
}

void access_var(float *var,size_t size){
	sequential_access(var,size);
}
