#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *worker_c;

char command[500]; // command buffer for system()

char template_a[]="\
#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <sys/time.h>\n\
#include <mpi.h>\n\
#include <math.h>\n\
struct timeval time_tv;\n\
double function(double x, double y, double z){\n\
	return(";
	
char template_b[]="\
);}\n\
int main(int argc, char *argv[]){\n\
double x_min, x_max, x_span;\n\
double y_min, y_max, y_span;\n\
double z_min, z_max, z_span;\n\
double sum_final, sum_local;\n\
double x, y, z;\n\
unsigned long int sample_count, i;\n\
int node_count, node_rank, node_namelen;\n\
char node_name[MPI_MAX_PROCESSOR_NAME];";

char template_c[]="\
x_span = x_max - x_min;\n\
y_span = y_max - y_min;\n\
z_span = z_max - z_min;\n\
MPI_Init(&argc, &argv);\n\
MPI_Comm_size(MPI_COMM_WORLD, &node_count);\n\
MPI_Comm_rank(MPI_COMM_WORLD, &node_rank);\n\
MPI_Get_processor_name(node_name, &node_namelen);\n\
if (!node_rank){\n\
	gettimeofday(&time_tv,NULL);\n\
	printf(\"montice: start  = %d+%d\\n\",time_tv.tv_sec,time_tv.tv_usec);\n\
}\n\
srand(time_tv.tv_sec+node_rank*time_tv.tv_usec);\n\
	for (i=0; i<sample_count; i++){\n\
	x = rand()/(double)RAND_MAX*(double)x_span+x_min;\n\
	y = rand()/(double)RAND_MAX*(double)y_span+y_min;\n\
	z = rand()/(double)RAND_MAX*(double)z_span+z_min;\n\
	sum_local += function(x,y,z)/(double)sample_count;\n\
	}\n\
MPI_Reduce(&sum_local,&sum_final,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);\n\
if (!node_rank){\n\
	double result=sum_final/(double)node_count;\n\
	printf(\"montice: result = %.15E\\n\",result);\n\
	gettimeofday(&time_tv,NULL);\n\
	printf(\"montice: finish = %d+%d\\n\",time_tv.tv_sec,time_tv.tv_usec);\n\
}\n\
MPI_Finalize();\n\
return(0);\n\
}\n";

void print_usage(){
	printf("montice: USAGE: montice EXPRESSION X_MIN X_MAX Y_MIN Y_MAX Z_MIN Z_MAX SAMPLE_COUNT PROC_COUNT HOSTFILE\n");
	}
	
	
int main(int argc, char *argv[]){
	if (argc!=11){
		print_usage();
		return(-1);
		}
		
	unsigned long int count_sample = atol(argv[8]);
	unsigned int count_proc = atol(argv[9]);
	unsigned long int sample = count_sample/count_proc;
	
	// building worker source file
	worker_c = fopen("worker.c","w");
	fprintf(worker_c,"%s",template_a);
	fprintf(worker_c,"%s",argv[1]);
	fprintf(worker_c,"%s",template_b);
	fprintf(worker_c,"sample_count = %ld;\n",sample);
	fprintf(worker_c,"x_min = %s;\n",argv[2]);
	fprintf(worker_c,"x_max = %s;\n",argv[3]);
	fprintf(worker_c,"y_min = %s;\n",argv[4]);
	fprintf(worker_c,"y_max = %s;\n",argv[5]);
	fprintf(worker_c,"z_min = %s;\n",argv[6]);
	fprintf(worker_c,"z_max = %s;\n",argv[7]);
	fprintf(worker_c,"%s",template_c);
	fclose(worker_c);

	// compile time
	if (system("mpicc worker.c -o worker")) return(1);
	
	// run time
	printf("montice: expression = %s\n",argv[1]);
	printf("montice: x = [%s .. %s]\n",argv[2],argv[3]);
	printf("montice: y = [%s .. %s]\n",argv[4],argv[5]);
	printf("montice: z = [%s .. %s]\n",argv[6],argv[7]);
	printf("montice: %ld samples\n",count_sample);
	printf("montice: %d processes using %s hostfile\n",count_proc,argv[10]);
	
	sprintf(command,"mpirun worker -hostfile %s -np %d",argv[10],count_proc);	
	if (system(command)) return(2);
	
	return(0);
	}
