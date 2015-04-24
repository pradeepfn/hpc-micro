#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "access.h"
#include "phoenix.h"

#define  ARRAY_SIZE	"array.size"
#define	 INIT_LOAD "init.load"
#define  RECOMPUTE_LOAD "recompute.load"
#define	 COMPUTE_LOAD "compute.load"

#define  MASTER		0
#define  MULT_CONSTANT	100000	
#define  COMPUTE_CONSTANT	10000000
#define CONFIG_FILE_NAME "micro.input"
#define NVARS 10

float update(int myoffset, int chunk, int myid);

int   numtasks, taskid,ierror; 
//checkpointable vars - 10
float *alpha,*beta,*phi,*zion,*kappa,*omega,*gamma1,*theta,*zeeta,*xeon;
float mysum, sum;

/*config param*/
int restart;

unsigned long array_size;
unsigned long init_load;
unsigned long recompute_load;
unsigned long compute_load;
float *vars[NVARS];

void compute_unit(unsigned long amp){
	int temp;
	unsigned long i,j;
	for(j=0; j < amp; j++){
		for(i=0; i < COMPUTE_CONSTANT ; i++){
			temp += 100;
			temp -= 100;	
			asm("");
		}
	}
}



void hpc_init(array_size){

	//varaible sre initialized by the phonix
	alpha = (float *)alloc("alpha",sizeof(float) * array_size,0,taskid);
	vars[0] = alpha;
	beta = (float *)alloc("beta",sizeof(float) * array_size,0,taskid);
	vars[1] = beta;
	phi = (float *)alloc("phi",sizeof(float) * array_size,0,taskid);
	vars[2] = phi;
	zion = (float *)alloc("zion",sizeof(float) * array_size,0,taskid);
	vars[3] = zion;
	kappa = (float *)alloc("kappa",sizeof(float) * array_size,0,taskid);
	vars[4] = kappa;
	omega = (float *)alloc("omega",sizeof(float) * array_size,0,taskid);
	vars[5] = omega;
	gamma1 = (float *)alloc("gamma1",sizeof(float) * array_size,0,taskid);
	vars[6] = gamma1;
	theta = (float *)alloc("theta",sizeof(float) * array_size,0,taskid);
	vars[7] = theta;
	zeeta = (float *)alloc("zeeta",sizeof(float) * array_size,0,taskid);
	vars[8] = zeeta;
	xeon = (float *)alloc("xeon",sizeof(float) * array_size,0,taskid);
	vars[9] = xeon;


	compute_unit(init_load);
	
}

void hpc_recompute(){
	int i;
		for(i=0;i<NVARS;i++){
			compute_unit(recompute_load);	
			access_var(vars[i],array_size);
		}
	
}

void hpc_iterations(){
	unsigned long i,j;

	compute_unit(compute_load);	

	//increment arrays
	for(i=0;i<NVARS;i++){
		for(j=0;j<array_size;j++){
			if(vars[i][j]==0){
				vars[i][j] = taskid*10;
			}else{
				vars[i][j] += 1.0;	
			}
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
			init_load = atoi(varvalue);
        }else if(!strncmp(RECOMPUTE_LOAD,varname,sizeof(varname))){
			recompute_load = atoi(varvalue);
        }else if(!strncmp(COMPUTE_LOAD,varname,sizeof(varname))){
			compute_load = atoi(varvalue);
        }else {
            printf("unknown varibale %s . please check the config\n", varname);
            exit(1);
        }
    }   
    fclose(fp);

}


int main (int argc, char *argv[]){

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
	init(taskid,numtasks);
	read_config();

	hpc_init(array_size);
	hpc_recompute();
	hpc_iterations();
	chkpt_all(taskid);

    MPI_Barrier(MPI_COMM_WORLD);
	MPI_Reduce(&mysum, &sum, 1, MPI_FLOAT, MPI_SUM, MASTER, MPI_COMM_WORLD);
	//sample printing
	printf("[%d] : %f , %f , %f , %f , %f , %f , %f , %f , %f , %f \n",taskid,vars[0][1],vars[1][1],vars[2][1],vars[3][1],vars[4][1],
																			  vars[5][1],vars[6][1],vars[7][1],vars[8][1],vars[9][1]);
	if(taskid == MASTER){
		printf("*** Final sum= %e ***\n",sum);
	}
	MPI_Finalize();
}   

