
#include "ELO2.h"
#include <iostream>
// reused code form prac3 MPI

/** This is the master node function, describing the operations
    that the master will be doing */
void Master () {
 //! <h3>Local vars</h3>
 // The above outputs a heading to doxygen function entry
 int  j;             //! j: Loop counter
 char buff[BUFSIZE]; //! buff: Buffer for transferring message data
 MPI_Status stat;    //! stat: Status of the MPI application

 printf("0: We have %d processors\n", numprocs);

 // Read the input image
 if(!Input.Read("Data/greatwall.jpg")){
  printf("Cannot read image\n");
  return;
 }

 // declaring all constants
 int width = Input.Width;
 int comp = Input.Components;
 int numcols = Input.Width * Input.Components;
 int stop;
 int start = 0; // inital start constant
 int rows = Input.Height;
 
tic();
// splitting and sending data to threads.
for(int j = 1; j < numprocs; j++){
    //tic();
    stop = Input.Height*j/(numprocs-1);

    //printf("%d %d %d \n",j,start,stop); // getting end condition
    int numrows = stop - start; // getting width of section
    int split = numrows*numcols; // getting total number of elements
    unsigned char subsection[split]; // create buffer to store all elements 
    int datavals[4] = {start,stop, numcols, rows }; //data values to be passed to mpi thread

     //convert to 1D array:
    int offset = 0; // offset
    for (int y = start; y < stop; y++){
        for (int x = 0; x < numcols; x++, offset++){
            subsection[offset] = Input.Rows[y][x]; // populate 1d array
        }
    }

    MPI_Send(datavals, 4, MPI_INT, j, TAG, MPI_COMM_WORLD); // using MPI_Int so i dont need to worry about sizes
    MPI_Send(subsection, split, MPI_CHAR, j, TAG, MPI_COMM_WORLD);

  
    start = stop-1; 
}
 // Allocated RAM for the output image
 if(!Output.Allocate(Input.Width, Input.Height, Input.Components)) return;
// receiving and compiling picture from threads.

for (int j=1; j < numprocs; j++){
    //tic();
    int datavals[4]; // for return array of data values
    MPI_Recv(datavals, 4, MPI_INT, j, TAG, MPI_COMM_WORLD, &stat);

    // pull variables out of array to make life easy with resusing names of variables, for continuity .
    start = datavals[0];
    stop = datavals[1];
    int numrows = stop - start;
    int split = numrows*numcols;
    unsigned char subsection[split];

    MPI_Recv(subsection, split, MPI_CHAR, j, TAG, MPI_COMM_WORLD, &stat);

    int offset = numcols; // offset used to not include the empty "row 0" each subsection has.
    for (int x = start; x < stop-1; x++){
        for (int y = 0; y< numcols; y++, offset++){
            Output.Rows[rows-x-1][y] = subsection[offset];
        }
    }
}

 // Write the output image
 if(!Output.Write("Data/Output.jpg")){
  printf("Cannot write image\n");
  return;
 }

printf("Node %d Time = %lg ms\n",j, (double)toc()/1e-3);
printf("Done");

}

/** This is the Slave function, the workers of this MPI application. */
void Slave(int ID){

    MPI_Status stat;
    int datavals[4]; // array to receiving data values

    MPI_Recv(datavals, 4, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); // using MPI_Int so i dont need to worry about sizes
    
    int start = datavals[0]; // taking variables out of array for ease of association.  can copy past code from Master without to much editing
    int stop = datavals[1];
    int numcols = datavals[2];
    int rows = datavals[3];
    int numrows = stop - start;
    int split = numrows*numcols;
    unsigned char subsection[split];
    //printf("%d \n",rows);
    MPI_Recv(subsection, split, MPI_BYTE, 0, TAG, MPI_COMM_WORLD, &stat);
    unsigned char ** image = new unsigned char*[numrows];
    unsigned char ** output = new unsigned char*[numrows];
    unsigned char filter[9]; // array to be send to median filter

    int offset = 0;
    int pos = 0;
    for (int x = 0; x < numrows; x++){
        image[x] = new unsigned char[numcols];
        output[x] = new unsigned char[numcols];
        for (int y = 0; y < numcols; y++, offset++){
            image[x][y] = subsection[offset]; // converting from 1D back to 2D
        }
    }
    //starting at pos 3, since the previous 3 values make up datapoint 1, so starting at datapoint 2 to avoid edge issues
    double pix = 0;
    double factor =0;
    for (int x = 0; x < numrows ; x++){ 
        for (int y = 0; y < numcols ; y++){
            factor =  0.6+0.4*( (double) x+start)/( (double) rows); //was having type errors with int, results in a factor of 0, so type casted everything to double
            pix = (double) image[x][y] * factor;
            output[x][y] =  pix;
        }
       //printf("%f \n", factor);
    }
    
    offset = 0;
    for (int x = 0; x < numrows; x++){
        for (int y = 0; y< numcols; y++, offset++){
            subsection[offset] = output[x][y]; //converting again from 2D to 1D
        }
    }

    MPI_Send(datavals, 4, MPI_INT, 0, TAG, MPI_COMM_WORLD);
    MPI_Send(subsection, split, MPI_CHAR, 0, TAG, MPI_COMM_WORLD);
}
//------------------------------------------------------------------------------

/** This is the entry point to the program. */
int main(int argc, char** argv){
 int myid;

 // MPI programs start with MPI_Init
 MPI_Init(&argc, &argv);

 // find out how big the world is
 MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

 // and this processes' rank is
 MPI_Comm_rank(MPI_COMM_WORLD, &myid);

 // At this point, all programs are running equivalently, the rank
 // distinguishes the roles of the programs, with
 // rank 0 often used as the "master".
 if(myid == 0) Master();
 else Slave (myid);

 // MPI programs end with MPI_Finalize
 MPI_Finalize();
 return 0;
}
//------------------------------------------------------------------------------
