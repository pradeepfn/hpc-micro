#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "access.h"

#define  ARRAY_SIZE	"array.size"
#define	 INIT_LOAD "init.load"
#define  RECOMPUTE_LOAD "recompute.load"
#define	 COMPUTE_LOAD "compute.load"

#define  MASTER		0
#define  MULT_CONSTANT	100000
#define CONFIG_FILE_NAME "micro.input"
#define NVARS 10

float update(int myoffset, int chunk, int myid);

//checkpointable vars - 10
float *alpha,*beta,*phi,*zion,*kappa,*omega,*gamma1,*theta,*zeeta,*xeon;
float mysum, sum;

/*config param*/
int restart;

long array_size;
long init_load;
long recompute_load;
long compute_load;
float *vars[NVARS];

void hpc_init(array_size){

	//varaible sre initialized by the phonix
	alpha = (float *)malloc(sizeof(float) * array_size);
	vars[0] = alpha;
	beta = (float *)malloc(sizeof(float) * array_size);
	vars[1] = beta;
	phi = (float *)malloc(sizeof(float) * array_size);
	vars[2] = phi;
	zion = (float *)malloc(sizeof(float) * array_size);
	vars[3] = zion;
	kappa = (float *)malloc(sizeof(float) * array_size);
	vars[4] = kappa;
	omega = (float *)malloc(sizeof(float) * array_size);
	vars[5] = omega;
	gamma1 = (float *)malloc(sizeof(float) * array_size);
	vars[6] = gamma1;
	theta = (float *)malloc(sizeof(float) * array_size);
	vars[7] = theta;
	zeeta = (float *)malloc(sizeof(float) * array_size);
	vars[8] = zeeta;
	xeon = (float *)malloc(sizeof(float) * array_size);
	vars[9] = xeon;

	int i;
	int temp;
	for(i=1; i < init_load ; i++){
		temp += 100;
		temp -= 100;	
		asm("");
	}

	
}

void hpc_recompute(){
	int i,j;
	int temp;
		for(j=0;j<NVARS;j++){
			for(i=1; i < recompute_load ; i++){
				temp += 100;
				temp -= 100;	
				asm("");
			}
			
			access_var(vars[j],array_size);
		}
	
}

void hpc_iterations(){
	int i,j;
	int temp;
	for(j=1; j < compute_load ; j++){
		for(i=1; i < compute_load ; i++){
			temp += 100;
			temp -= 100;	
			asm("");
		}
	}
	mysum = 10;
}

void read_config(){
	char varname[30];
    char varvalue[32];
	FILE *fp = fopen(CONFIG_FILE_NAME,"r");
    if(fp == NULL){
        printf("error while opening the file\n");
        exit(1);
    }   
    while (fscanf(fp, "%s =  %s", varname, varvalue) != EOF) {
        if(varname[0] == '#'){
            // consuming the input line starting with '#'
            fscanf(fp,"%*[^\n]");  
            fscanf(fp,"%*1[\n]"); 
            continue;
        }else if(!strncmp(ARRAY_SIZE,varname,sizeof(varname))){
			array_size = atoi(varvalue) * MULT_CONSTANT; 
        }else if(!strncmp(INIT_LOAD,varname,sizeof(varname))){
			init_load = atoi(varvalue) * MULT_CONSTANT;
        }else if(!strncmp(RECOMPUTE_LOAD,varname,sizeof(varname))){
			recompute_load = atoi(varvalue) * MULT_CONSTANT;
        }else if(!strncmp(COMPUTE_LOAD,varname,sizeof(varname))){
			compute_load = atoi(varvalue) * MULT_CONSTANT;
        }else {
            printf("unknown varibale %s . please check the config\n", varname);
            exit(1);
        }
    }   
    fclose(fp);

}


int main (int argc, char *argv[]){

	int   numtasks, taskid,ierror; 
	int i;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	if (numtasks % 4 != 0) {
		printf("Quitting. Number of MPI tasks must be divisible by 4.\n");
		MPI_Abort(MPI_COMM_WORLD, ierror);
		exit(0);
	}

	MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
	read_config();
	printf ("MPI task %d has started...\n", taskid);

	

	hpc_init(array_size);
	hpc_recompute();
	hpc_iterations();

	MPI_Reduce(&mysum, &sum, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);
	if(taskid == MASTER){
		printf("*** Final sum= %e ***\n",sum);
	}
	MPI_Finalize();
}   

