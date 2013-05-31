#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>
struct timeval time_tv;
double function(double x, double y, double z){
	return(x*y*z);
}
int main(int argc, char *argv[]){
double x_min, x_max, x_span;
double y_min, y_max, y_span;
double z_min, z_max, z_span;
double sum_final, sum_local;
double x=0.0, y=0.0, z=0.0;
unsigned long int sample_count, i;
int node_count, node_rank, node_namelen;
char node_name[MPI_MAX_PROCESSOR_NAME];
sample_count = 1000;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &node_count);
MPI_Comm_rank(MPI_COMM_WORLD, &node_rank);
MPI_Get_processor_name(node_name, &node_namelen);
if (!node_rank){
	gettimeofday(&time_tv,NULL);
	printf("montice: start  = %d+%d\n",time_tv.tv_sec,time_tv.tv_usec);
	}
srand(time_tv.tv_sec+node_rank*time_tv.tv_usec);
	for (i=0; i<sample_count; i++){
	x_min = -2;
	x_max = 0;
	y_min = x;
	y_max = z;
	z_min = 0;
	z_max = 17;
 	x_span = x_max - x_min;
	y_span = y_max - y_min;
	z_span = z_max - z_min;
	x = rand()/(double)RAND_MAX*(double)x_span+x_min;
	y = rand()/(double)RAND_MAX*(double)y_span+y_min;
	z = rand()/(double)RAND_MAX*(double)z_span+z_min;
	sum_local += function(x,y,z)/(double)sample_count;
	}
MPI_Reduce(&sum_local,&sum_final,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
if (!node_rank){
	double result=sum_final/(double)node_count;
	printf("montice: result = %.15E\n",result);
	gettimeofday(&time_tv,NULL);
	printf("montice: finish = %d+%d\n",time_tv.tv_sec,time_tv.tv_usec);
	}
MPI_Finalize();
return(0);
}
