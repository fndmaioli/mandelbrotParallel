#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h> 
#include <omp.h>

# define MAXITER 2000


struct complex{
  double real;
  double imag;
};

int main(int argc, char *argv[]){

  int NPOINTS, id, p;
  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  MPI_Comm_size(MPI_COMM_WORLD, &p);
  if (id == 0) {
    printf("--bynode -np %d\n",p);
  }
  for(NPOINTS = 500; NPOINTS <=5000; NPOINTS +=500) {

    int i, j, iter, numoutside = 0;
    double area, error, ztemp;
    double start, finish;
    struct complex z, c;

    int counter = 0;

    //MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    #pragma omp parallel for private(c, z, ztemp, j, iter)
    for (i=id; i<NPOINTS; i+=p) {
      for (j=0; j<NPOINTS; j++) {
        c.real = -2.0+2.5*(double)(i)/(double)(NPOINTS)+1.0e-7;
        c.imag = 1.125*(double)(j)/(double)(NPOINTS)+1.0e-7;
        z=c;
        for (iter=0; iter<MAXITER; iter++){
          ztemp=(z.real*z.real)-(z.imag*z.imag)+c.real;
          z.imag=z.real*z.imag*2+c.imag; 
          z.real=ztemp; 
          if ((z.real*z.real+z.imag*z.imag)>4.0e0) {
            #pragma omp critical
            {
              counter++;
            }
            break;
          }
        }
      }
    }
    MPI_Reduce(&counter, &numoutside, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    finish = MPI_Wtime();  

    //Calculate area and error and output the results

    if (id == 0) {
      area=2.0*2.5*1.125*(double)(NPOINTS*NPOINTS-numoutside)/(double)(NPOINTS*NPOINTS);
      error=area/(double)NPOINTS;

      printf("Area of Mandlebrot set = %12.8f +/- %12.8f\n",area,error);
      printf("Time = %12.8f seconds\n",finish-start);
    }
  }
  MPI_Finalize();
  return 0;
}