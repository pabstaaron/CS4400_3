/*******************************************
 * Solutions for the CS:APP Performance Lab
 ********************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

/* 
 * Please fill in the following student struct 
 */
student_t student = {
  "Aaron Pabst",     /* Full name */
  "aaron.pabst@utah.edu",  /* Email address */
};

/***************
 * COMPLEX KERNEL
 ***************/

/******************************************************
 * Your different versions of the complex kernel go here
 ******************************************************/

/* 
 * naive_complex - The naive baseline version of complex 
 */
char naive_complex_descr[] = "naive_complex: Naive baseline implementation";
void naive_complex(int dim, pixel *src, pixel *dest)
{
  int i, j;

  for(i = 0; i < dim; i++)
    for(j = 0; j < dim; j++)
    {

      dest[RIDX(dim - j - 1, dim - i - 1, dim)].red = ((int)src[RIDX(i, j, dim)].red +
						      (int)src[RIDX(i, j, dim)].green +
						      (int)src[RIDX(i, j, dim)].blue) / 3;
      
      dest[RIDX(dim - j - 1, dim - i - 1, dim)].green = ((int)src[RIDX(i, j, dim)].red +
							(int)src[RIDX(i, j, dim)].green +
							(int)src[RIDX(i, j, dim)].blue) / 3;
      
      dest[RIDX(dim - j - 1, dim - i - 1, dim)].blue = ((int)src[RIDX(i, j, dim)].red +
						       (int)src[RIDX(i, j, dim)].green +
						       (int)src[RIDX(i, j, dim)].blue) / 3;

    }
}


/* 
 * naive_two - The naive baseline version of complex 
 */
char complex_two_descr[] = "complex_two: Now with 50% less CPE!";
void complex_two(int dim, pixel *src, pixel *dest)
{
  int i, j;
  
  for(i = 0; i < dim; i++)
    for(j = 0; j < dim; j++)
    {

      pixel currPix = src[RIDX(i, j, dim)]; // i and j should be flipped
      int grayscale = ((int)currPix.red +
		      (int)currPix.green +
		      (int)currPix.blue) / 3;
      
      int index = RIDX(dim - j - 1, dim - i - 1, dim);
      
      dest[index].red = grayscale;
      
      dest[index].green = grayscale;
      
      dest[index].blue = grayscale;

    }
}

/* 
 * complex - Your current working version of complex
 * IMPORTANT: This is the version you will be graded on
 */
char complex_descr[] = "complex: Current working version";
void complex(int dim, pixel *src, pixel *dest)
{
  naive_complex(dim, src, dest);
}

/*********************************************************************
 * register_complex_functions - Register all of your different versions
 *     of the complex kernel with the driver by calling the
 *     add_complex_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_complex_functions() {
  add_complex_function(&complex, complex_descr);
  add_complex_function(&naive_complex, naive_complex_descr);
  add_complex_function(&complex_two, complex_two_descr);
}


/***************
 * MOTION KERNEL
 **************/

/***************************************************************
 * Various helper functions for the motion kernel
 * You may modify these or add new ones any way you like.
 **************************************************************/


/* 
 * weighted_combo - Returns new pixel value at (i,j) 
 */
static pixel weighted_combo(int dim, int i, int j, pixel *src) 
{
  int ii, jj;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;

  int num_neighbors = 0;
  for(ii=0; ii < 3; ii++)
    for(jj=0; jj < 3; jj++) 
      if ((i + ii < dim) && (j + jj < dim)) 
      {
	num_neighbors++;
	red += (int) src[RIDX(i+ii,j+jj,dim)].red;
	green += (int) src[RIDX(i+ii,j+jj,dim)].green;
	blue += (int) src[RIDX(i+ii,j+jj,dim)].blue;
      }
  
  current_pixel.red = (unsigned short) (red / num_neighbors);
  current_pixel.green = (unsigned short) (green / num_neighbors);
  current_pixel.blue = (unsigned short) (blue / num_neighbors);
  
  return current_pixel;
}

/* 
 * weighted_combo_2 - Returns new pixel value at (i,j) 
 *  Pulled out some some expressions to imporve operational efficency
 */
static pixel weighted_combo_two(int dim, int i, int j, pixel *src) 
{
  int ii, jj;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;

  int num_neighbors = 0;
  for(ii=0; ii < 3; ii++)
    for(jj=0; jj < 3; jj++) 
      if ((i + ii < dim) && (j + jj < dim)) 
      {
	num_neighbors++;
	pixel currPix = src[RIDX(i+ii,j+jj,dim)];
	red += (int)currPix.red;
	green += (int) currPix.green;
	blue += (int) currPix.blue;
      }
  
  current_pixel.red = (unsigned short) (red / num_neighbors);
  current_pixel.green = (unsigned short) (green / num_neighbors);
  current_pixel.blue = (unsigned short) (blue / num_neighbors);
  // printf("%d", num_neighbors);
  
  return current_pixel;
}

/*
 * Able to remove the if statement into the loop thanks to dimension changes in motion_three
 */
static pixel weighted_combo_three(int dim, int i, int j, pixel *src) 
{
  int ii, jj;
  pixel current_pixel;

  int red, green, blue;
  red = green = blue = 0;

  int num_neighbors = 0;
  for(ii=0; ii < 3; ii++)
    for(jj=0; jj < 3; jj++) 
    {
      num_neighbors++;
      pixel currPix = src[RIDX(i+ii,j+jj,dim)];
      red += (int)currPix.red;
      green += (int) currPix.green;
      blue += (int) currPix.blue;
    }
  
  current_pixel.red = (unsigned short) (red / num_neighbors);
  current_pixel.green = (unsigned short) (green / num_neighbors);
  current_pixel.blue = (unsigned short) (blue / num_neighbors);
  
  return current_pixel;
}


/*
 * Returns the new pixel value for pixels located along a corner exluded border for motion
 */
static pixel weighted_combo_border(int dim, int i, int j, pixel *src){
  
}

/******************************************************
 * Your different versions of the motion kernel go here
 ******************************************************/


/*
 * naive_motion - The naive baseline version of motion 
 */
char naive_motion_descr[] = "naive_motion: Naive baseline implementation";
void naive_motion(int dim, pixel *src, pixel *dst) 
{
  int i, j;
    
  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      dst[RIDX(i, j, dim)] = weighted_combo(dim, i, j, src);
}

/*
 * motion_two - Pulled out some sub expressions in weighted_combo
 */
char motion_two_descr[] = "motion_two - Pulled out some sub expressions in weighted_combo";
void motion_two(int dim, pixel *src, pixel *dst) 
{
  int i, j;
    
  for (i = 0; i < dim; i++)
    for (j = 0; j < dim; j++)
      dst[RIDX(i, j, dim)] = weighted_combo_two(dim, i, j, src);
}


void motion_three(int dim, pixel *src, pixel *dst){
  int i, j;

  // Exclude the two right most columns and the two bottom most rows so that the conditional
  // and associated computations in weighted combo can be removed.
  for(i = 0; i < dim-2; i++)
    for(j = 0; j < dim-2; j++){
      dst[RIDX(i, j, dim)] = weighted_combo_three(dim, i, j, src);
    }

  j = dim-2;
  int k = dim-3;
  for(i = 0; i < dim-2; i++){
    pixel newPix;

    pixel currPix = src[RIDX(j, i, dim)];
    newPix.red += currPix.red;
    newPix.green += currPix.green;
    newPix.blue += currPix.blue;

    currPix = src[RIDX(j, i+1, dim)];
    newPix.red += currPix.red;
    newPix.green += currPix.green;
    newPix.blue += currPix.blue;

    currPix = src[RIDX(k, i, dim)];
    newPix.red += currPix.red;
    newPix.green += currPix.green;
    newPix.blue += currPix.blue;

    currPix = src[RIDX(k, i+1, dim)];
    newPix.red += currPix.red;
    newPix.green += currPix.green;
    newPix.blue += currPix.blue;

    newPix.red /= 4;
    newPix.green /= 4;
    newPix.blue /= 4;
  }
  
  
}

/*
 * motion - Your current working version of motion. 
 * IMPORTANT: This is the version you will be graded on
 */
char motion_descr[] = "motion: Current working version";
void motion(int dim, pixel *src, pixel *dst) 
{
  naive_motion(dim, src, dst);
}

/********************************************************************* 
 * register_motion_functions - Register all of your different versions
 *     of the motion kernel with the driver by calling the
 *     add_motion_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_motion_functions() {
  add_motion_function(&motion, motion_descr);
  add_motion_function(&naive_motion, naive_motion_descr);
  add_motion_function(&motion_two, motion_two_descr);
}
