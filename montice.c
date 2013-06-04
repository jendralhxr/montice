#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *worker_c;

char command[500]; // command buffer for system()
char var_printed[3];
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
);\n}\n\
int main(int argc, char *argv[]){\n\
double x_min, x_max, x_span;\n\
double y_min, y_max, y_span;\n\
double z_min, z_max, z_span;\n\
double sum_final, sum_local;\n\
double x=0.0, y=0.0, z=0.0;\n\
unsigned long int sample_count, i;\n\
int node_count, node_rank, node_namelen;\n\
char node_name[MPI_MAX_PROCESSOR_NAME];\n";

char template_c[]="\
MPI_Init(&argc, &argv);\n\
MPI_Comm_size(MPI_COMM_WORLD, &node_count);\n\
MPI_Comm_rank(MPI_COMM_WORLD, &node_rank);\n\
MPI_Get_processor_name(node_name, &node_namelen);\n\
if (!node_rank){\n\
	gettimeofday(&time_tv,NULL);\n\
	printf(\"montice: start  = %d+%d\\n\",time_tv.tv_sec,time_tv.tv_usec);\n\
	}\n\
	srand(time_tv.tv_sec*node_rank+time_tv.tv_usec);\n\
	for (i=0; i<sample_count; i++){\n";
	
char template_d[]="\
 	sum_local += function(x,y,z)/(double)sample_count*x_span*y_span*z_span;\n\
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
	
void var_printed_x(char *min, char *max){
	fprintf(worker_c,"\tx_min = %s;\n",min);
	fprintf(worker_c,"\tx_max = %s;\n",max);
	fprintf(worker_c,"\tx_span = x_max - x_min;\n");
	fprintf(worker_c,"\tx = rand()/(double)RAND_MAX*(double)x_span+x_min;\n");
	var_printed[0]=1;
	}
	
void var_printed_y(char *min, char *max){
	fprintf(worker_c,"\ty_min = %s;\n",min);
	fprintf(worker_c,"\ty_max = %s;\n",max);
	fprintf(worker_c,"\ty_span = y_max - y_min;\n");
	fprintf(worker_c,"\ty = rand()/(double)RAND_MAX*(double)y_span+y_min;\n");
	var_printed[1]=1;
	}

void var_printed_z(char *min, char *max){
	fprintf(worker_c,"\tz_min = %s;\n",min);
	fprintf(worker_c,"\tz_max = %s;\n",max);
	fprintf(worker_c,"\tz_span = z_max - z_min;\n");
	fprintf(worker_c,"\tz = rand()/(double)RAND_MAX*(double)z_span+z_min;\n");
	var_printed[2]=1;
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
	fprintf(worker_c,"%s",template_c);
	// x,y,z -- strstr and stuff
	// independent
	if (!strstr(argv[2],"y") && !strstr(argv[2],"z") \
		&& !strstr(argv[3],"y")&& !strstr(argv[3],"z"))
		var_printed_x(argv[2],argv[3]);
 	if (!strstr(argv[4],"x") && !strstr(argv[4],"z") \
		&& !strstr(argv[5],"x")&& !strstr(argv[5],"z"))
		var_printed_y(argv[4],argv[5]);
	if (!strstr(argv[6],"x") && !strstr(argv[6],"y") \
		&& !strstr(argv[7],"x")&& !strstr(argv[7],"y"))
		var_printed_z(argv[6],argv[7]);
	// dependent to one
	// y(x)
	if (!strstr(argv[4],"z") && !strstr(argv[5],"z")\
		&& (var_printed[0]==1) && (var_printed[1]==0))
		var_printed_y(argv[4],argv[5]);
	// z(x)
	if (!strstr(argv[6],"y") && !strstr(argv[7],"y")\
		&& (var_printed[0]==1) && (var_printed[2]==0))
		var_printed_z(argv[4],argv[5]);
	// x(y)
	if (!strstr(argv[2],"z") && !strstr(argv[3],"z")\
		&& (var_printed[1]==1) && (var_printed[0]==0))
		var_printed_x(argv[2],argv[3]);
	// z(y)
	if (!strstr(argv[6],"x") && !strstr(argv[7],"x")\
		&& (var_printed[1]==1) && (var_printed[2]==0))
		var_printed_z(argv[6],argv[7]);
	// x(z)
	if (!strstr(argv[2],"y") && !strstr(argv[3],"y")\
		&& (var_printed[2]==1) && (var_printed[0]==0))
		var_printed_x(argv[2],argv[3]);
	// y(z)
	if (!strstr(argv[4],"x") && !strstr(argv[5],"x")\
		&& (var_printed[2]==1) && (var_printed[1]==0))
		var_printed_y(argv[4],argv[5]);
	if (var_printed[0]==0) var_printed_x(argv[2],argv[3]);
	if (var_printed[1]==0) var_printed_y(argv[4],argv[5]);
	if (var_printed[2]==0) var_printed_z(argv[6],argv[7]);
	fprintf(worker_c,"%s",template_d);
	fclose(worker_c);

	// compile time
	if (system("mpicc worker.c -o worker")) return(1);
	system("sync");
	
	// run time
	printf("montice: expression = %s\n",argv[1]);
	printf("montice: x = [%s .. %s]\n",argv[2],argv[3]);
	printf("montice: y = [%s .. %s]\n",argv[4],argv[5]);
	printf("montice: z = [%s .. %s]\n",argv[6],argv[7]);
	printf("montice: %ld samples\n",count_sample);
	sprintf(command,"mpirun -np %d -hostfile %s worker",count_proc,argv[10]);	
	printf("montice: %s\n",command);
	if (system(command)) return(2);
	
	return(0);
	}
