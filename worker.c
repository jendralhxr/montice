#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>
struct timeval start_time, stop_time;
double function(double x, double y, double z){
	return(pow(x*x+y*y+z*z,1.5));
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
sample_count = 5;
MPI_Init(&argc, &argv);
MPI_Comm_size(MPI_COMM_WORLD, &node_count);
MPI_Comm_rank(MPI_COMM_WORLD, &node_rank);
MPI_Get_processor_name(node_name, &node_namelen);
if (!node_rank){
	gettimeofday(&start_time,NULL);
	printf("montice: start  = %d.%d\n",start_time.tv_sec,start_time.tv_usec);
	}
	srand(start_time.tv_sec*node_rank+start_time.tv_usec);
	for (i=0; i<sample_count; i++){
	x_min = -3;
	x_max = 3;
	x_span = x_max - x_min;
	x = rand()/(double)RAND_MAX*(double)x_span+x_min;
	y_min = -1*pow(9-x*x,0.5);
	y_max = pow(9-x*x,0.5);
	y_span = y_max - y_min;
	y = rand()/(double)RAND_MAX*(double)y_span+y_min;
	z_min = -1*pow(9-x*x-y*y,0.5);
	z_max = pow(9-x*x-y*y,0.5);
	z_span = z_max - z_min;
	z = rand()/(double)RAND_MAX*(double)z_span+z_min;
 	sum_local += function(x,y,z)/(double)sample_count*x_span*y_span*z_span;
	}
MPI_Reduce(&sum_local,&sum_final,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
if (!node_rank){
	double result=sum_final/(double)node_count;
	printf("montice: result = %.15E\n",result);
	gettimeofday(&stop_time,NULL);
	printf("montice: finish = %d.%d\n",stop_time.tv_sec,stop_time.tv_usec);
	printf("montice: elapsed = %f\n",stop_time.tv_sec-start_time.tv_sec+(double)(stop_time.tv_usec-start_time.tv_usec)/1000000);
	}
MPI_Finalize();
return(0);
}
