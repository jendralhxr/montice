#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *worker_c;

char command[500]; // command buffer for system()
char flag[3]; // variables' dependencies flag

char template_a[]="\
#include <stdio.h>\n\
#include <stdlib.h>\n\
#include <sys/time.h>\n\
#include <mpi.h>\n\
#include <math.h>\n\
struct timeval start_time, stop_time;\n\
double function(double x, double y, double z){\n\
	return(";
	
char template_b[]="\
);\n}\n\
int main(int argc, char *argv[]){\n\
double x_min, x_max, x=0.0, x_div;\n\
double y_min, y_max, y=0.0, y_div;\n\
double z_min, z_max, z=0.0, z_div;\n\
unsigned long int x_num, y_num, z_num;\n\
double sum_local, sum_final;\n\
unsigned long int i, j, k;\n\
int node_count, node_rank, node_namelen;\n\
char node_name[MPI_MAX_PROCESSOR_NAME];\n\
x_min=0.0; x_max=1.0; y_min=0.0; y_max=1.0; z_min=0.0; z_max=1.0;\n";

char template_c[]="\
MPI_Init(&argc, &argv);\n\
MPI_Comm_size(MPI_COMM_WORLD, &node_count);\n\
MPI_Comm_rank(MPI_COMM_WORLD, &node_rank);\n\
MPI_Get_processor_name(node_name, &node_namelen);\n\
if (!node_rank){\n\
	gettimeofday(&start_time,NULL);\n\
	printf(\"blokice: start  = %d+%d\\n\",start_time.tv_sec,start_time.tv_usec);\n\
	}\n";
	
char template_d[]="MPI_Reduce(&sum_local,&sum_final,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);\n\
if (!node_rank){\n\
	printf(\"blokice: result = %.15E\\n\",sum_final);\n\
	gettimeofday(&stop_time,NULL);\n\
	printf(\"blokice: finish = %d+%d\\n\",stop_time.tv_sec,stop_time.tv_usec);\n\
	printf(\"blokice: elapsed = %f\\n\",stop_time.tv_sec-start_time.tv_sec+(double)(stop_time.tv_usec-start_time.tv_usec)/1000000);\n\
	}\n\
MPI_Finalize();\n\
return(0);\n\
}\n";

void print_usage(){
	printf("blokice: USAGE: blokice EXPRESSION X_MIN X_MAX X_NUM \
Y_MIN Y_MAX Y_NUM Z_MIN Z_MAX Z_NUM PROC_COUNT HOSTFILE\n");
	}
	
void print_field(char var, char *min, char *max){
	fprintf(worker_c,"\t%c_min = %s;\n",var,min);
	fprintf(worker_c,"\t%c_max = %s;\n",var,max);
	fprintf(worker_c,"\t%c_div=(%c_max-%c_min)/(double)%c_num;\n",var,var,var,var);
	}
	
int main(int argc, char *argv[]){
	if (argc!=13){
		print_usage();
		return(-1);
		}
		
	unsigned int count_proc = atol(argv[11]);
	unsigned long int sample = atol(argv[4])*atol(argv[7])*atol(argv[10])*atol(argv[10]);
	
	// building worker source file
	worker_c = fopen("worker-blokice.c","w");
	fprintf(worker_c,"%s",template_a);
	fprintf(worker_c,"%s",argv[1]);
	fprintf(worker_c,"%s",template_b);
	fprintf(worker_c,"x_num=%s;\n",argv[4]);
	fprintf(worker_c,"y_num=%s;\n",argv[7]);
	fprintf(worker_c,"z_num=%s;\n",argv[10]);
	fprintf(worker_c,"%s",template_c);
			
	//~ // dependencies check
	//~ // y(x)
	//~ if (strstr(argv[5],"x") || strstr(argv[6],"x")) flag[1]|=0b0001;
	//~ // z(x)
	//~ if (strstr(argv[8],"x") || strstr(argv[9],"x")) flag[2]|=0b0001;
	//~ // x(y)
	//~ if (strstr(argv[2],"y") || strstr(argv[3],"y")) flag[0]|=0b0010;
	//~ // z(y)
	//~ if (strstr(argv[8],"y") || strstr(argv[9],"y")) flag[2]|=0b0010;
	//~ // x(z)
	//~ if (strstr(argv[2],"z") || strstr(argv[3],"z")) flag[0]|=0b0100;
	//~ // y(z)
	//~ if (strstr(argv[5],"z") || strstr(argv[6],"z")) flag[1]|=0b0100;
	
	fprintf(worker_c,"x_min=%f;\n",atof(argv[2]));
	fprintf(worker_c,"x_max=%f;\n",atof(argv[3]));
	fprintf(worker_c,"x_div=x_max-x_min;\n");
	fprintf(worker_c,"x_div=x_div/(double)node_count;\n");
	fprintf(worker_c,"x_min=x_min+x_div*node_rank;\n");
	fprintf(worker_c,"x_max=x_min+x_div;\n");
	fprintf(worker_c,"x_div=x_max-x_min;\n");
	
	fprintf(worker_c,"for (i=0; x<x_max; i++){\n");
	fprintf(worker_c,"x=x_min+i*x_div;\n");
	print_field('y',argv[5],argv[6]);
	print_field('z',argv[8],argv[9]);
	fprintf(worker_c,"y=y_min; z=z_min;\n");
	
	fprintf(worker_c,"for (j=0; y<y_max; j++){\n");
	fprintf(worker_c,"y=y_min+j*y_div;\n");
	print_field('z',argv[8],argv[9]);
	fprintf(worker_c,"z=z_min;\n");
	
	fprintf(worker_c,"for (k=0; z<z_max; k++){\n");
	fprintf(worker_c,"z=z_min+k*z_div;\n");
	fprintf(worker_c,"if ((x<x_max) && (y<y_max) && (z<z_max)) \
	sum_local+=function(x+0.5*x_div,y+0.5*y_div,z+0.5*z_div)*x_div*y_div*z_div;\n");
	fprintf(worker_c,"}}}\n");
			
	fprintf(worker_c,"%s",template_d);
	fclose(worker_c);

	// compile time
	if (system("mpicc worker-blokice.c -o worker-blokice")) return(1);
	system("sync");
	
	// run time
	printf("blokice: expression = %s\n",argv[1]);
	printf("blokice: x = [%s .. %s] %s div\n",argv[2],argv[3],argv[4]);
	printf("blokice: y = [%s .. %s] %s div\n",argv[5],argv[6],argv[7]);
	printf("blokice: z = [%s .. %s] %s div\n",argv[8],argv[9],argv[10]);
	printf("blokice: samples = %ld\n",sample);
	sprintf(command,"mpirun -np %d -hostfile %s worker-blokice",count_proc,argv[12]);	
	printf("blokice: %s\n",command);
	if (system(command)) return(2);
	
	return(0);
	}
