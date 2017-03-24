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
 * complex_two - The second iteration of complex 
 */
char complex_two_descr[] = "complex_2: The second iteration of complex";
void complex_two(int dim, pixel *src, pixel *dest) // Likely memory aliasing occuring here
{
  int i, j;

  // Swapping these loop variables will increase locality in the loop body, leading to more cach hits.
  for(i = 0; i < dim; i++)
    for(j = 0; j < dim; j++)
    {
      
      
      int index = RIDX(dim - j - 1, dim - i - 1, dim); // The location of the pixel that we're currently dealing with.
      int value = ((int)src[RIDX(i, j, dim)].red + // The grayscale value of the pixel
		  (int)src[RIDX(i, j, dim)].green +
	          (int)src[RIDX(i, j, dim)].blue) / 3;

      dest[index].red = value;
      
      dest[index].green = value;
      
      dest[index].blue = value;

    }
}


/*
 * complex_three - The third iteration of complex
 */

char complex_three_descr[] = "complex_three: The third iteration of complex";
void complex_three(int dim, pixel *src, pixel *dest) // Likely memory aliasing occuring here
{
  int i, j;

  for(j = 0; j < dim; j++)
    for(i = 0; i < dim; i++)
    {
      
      
      int index = RIDX(dim - j - 1, dim - i - 1, dim); // The location of the pixel that we're currently dealing with.

      int value = ((int)src[RIDX(i, j, dim)].red + // The grayscale value of the pixel
		  (int)src[RIDX(i, j, dim)].green +
	          (int)src[RIDX(i, j, dim)].blue) / 3;

      dest[index].red = value;
      
      dest[index].green = value;
      
      dest[index].blue = value;

    }
}


/*
 * complex_three - The third iteration of complex
 */

char complex_four_descr[] = "complex_four: Now with 50% more speedup!";
void complex_four(int dim, pixel *src, pixel *dest) // Likely memory aliasing occuring here
{
  int i, j;

  int dimOne = dim - 1;
  for(j = 0; j < dim; j++){
    int indOne = dimOne - j;
    for(i = 0; i < dim; i++)
    {
      
      
      int index = RIDX(indOne, dimOne - i, dim); // The location of the pixel that we're currently dealing with.


      pixel currPix = src[RIDX(i, j, dim)];
      int value = ((int)currPix.red + // The grayscale value of the pixel
		  (int)currPix.green +
	          (int)currPix.blue) / 3;

      pixel newPix;
      newPix.red = value;
      newPix.green = value;
      newPix.blue = value;

      dest[index] = newPix;
    }
  }
}
/* 
 * naive_complex - The naive baseline version of complex 
 */
char naive_complex_descr[] = "naive_complex - The naive baseline version of complex";
void naive_complex(int dim, pixel *src, pixel *dest) // Likely memory aliasing occuring here
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
 * complex - Your current working version of complex
 * IMPORTANT: This is the version you will be graded on
 */
char complex_descr[] = "complex: Current working version";
void complex(int dim, pixel *src, pixel *dest)
{
  complex_four(dim, src, dest);
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
  add_complex_function(&complex_three, complex_three_descr);
  add_complex_function(&complex_four, complex_four_descr);
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

  for(ii=0; ii < 3; ii++)
    for(jj=0; jj < 3; jj++) 
    {
      pixel currPix = src[RIDX(i+ii,j+jj,dim)];
      red += (int)currPix.red;
      green += (int) currPix.green;
      blue += (int) currPix.blue;
    }
  
  current_pixel.red = (unsigned short) (red / 9);
  current_pixel.green = (unsigned short) (green / 9);
  current_pixel.blue = (unsigned short) (blue / 9);
  
  return current_pixel;
}

/*
 * Able to remove the if statement into the loop thanks to dimension changes in motion_three
 */
static pixel weighted_combo_four(int dim, int i, int j, pixel *src) 
{
  pixel current_pixel;

  int red, green, blue;

  // Bundle all of the inicies calculations together to try and leverage the fact that there are multiple adders
  int indBase = RIDX(i, j, dim);
  int indOne = RIDX(i+1, j, dim);
  int indTwo = RIDX(i+2, j, dim);

  int indThree = RIDX(i, j+1, dim);
  int indFour = RIDX(i+1, j+1, dim);
  int indFive = RIDX(i+2, j+1, dim);

  int indSix = RIDX(i, j+2, dim);
  int indSeven = RIDX(i+1, j+2, dim);
  int indEight = RIDX(i+2, j+2, dim);
  
  pixel currPix = src[indBase];
  red = (int)currPix.red;
  green = (int)currPix.green;
  blue = (int)currPix.blue;

  currPix = src[indOne];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[indTwo];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[indThree];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[indFour];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[indFive];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[indSix];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[indSeven];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[indEight];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  current_pixel.red = (unsigned short) (red / 9);
  current_pixel.green = (unsigned short) (green / 9);
  current_pixel.blue = (unsigned short) (blue / 9);
  
  return current_pixel;
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

char motion_three_descr[] = "motion_three - Unrolled the loops";
void motion_three(int dim, pixel *src, pixel *dst){
  int i, j;

  // Exclude the two right most columns and the two bottom most rows so that the conditional
  // and associated computations in weighted combo can be removed.
  for(i = 0; i < dim-2; i++)
    for(j = 0; j < dim-2; j++){
      dst[RIDX(i, j, dim)] = weighted_combo_three(dim, i, j, src);
    }


  int red, green, blue;
  red = green = blue = 0;

  // Go across the bottom two rows.
  j = dim-2;
  int k = dim-1;
  for(i = 0; i < dim-2; i++){
    pixel newPix;

    pixel currPix = src[RIDX(j, i, dim)];
    red = (int)currPix.red;
    green = (int)currPix.green;
    blue = (int)currPix.blue;

    currPix = src[RIDX(j, i+1, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(j, i+2, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(k, i, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(k, i+1, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(k, i+2, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    newPix.red = (unsigned short)(red/6);
    newPix.green = (unsigned short)(green/6);
    newPix.blue = (unsigned short)(blue/6);

    dst[RIDX(j, i, dim)] = newPix;
  }
  
  j = dim-1;
  for(i = 0; i < dim-2; i++){
    pixel newPix;
    
    pixel currPix = src[RIDX(j, i, dim)];
    red = (int)currPix.red;
    blue = (int)currPix.blue;
    green = (int)currPix.green;

    currPix = src[RIDX(j, i+1, dim)];
    red += (int)currPix.red;
    blue += (int)currPix.blue;
    green += (int)currPix.green;

    currPix = src[RIDX(j, i+2, dim)];
    red += (int)currPix.red;
    blue += (int)currPix.blue; 
    green += (int)currPix.green;

    newPix.green = (unsigned short)(green/3);
    newPix.blue = (unsigned short)(blue/3);
    newPix.red = (unsigned short)(red/3);

    dst[RIDX(j, i, dim)] = newPix;
  }

  // Go down the rightmost two rows
  j = dim-2;
  k = dim-1;
  for(i = 0; i < dim-2; i++){
    pixel newPix;

    pixel currPix = src[RIDX(i, j,  dim)];
    red = (int)currPix.red;
    green = (int)currPix.green;
    blue = (int)currPix.blue;

    currPix = src[RIDX(i+1, j, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(i+2, j, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(i, k, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(i+1, k, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(i+2, k, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    newPix.red = (unsigned short)(red/6);
    newPix.green = (unsigned short)(green/6);
    newPix.blue = (unsigned short)(blue/6);

    dst[RIDX(i, j, dim)] = newPix;
  }
  
  j = dim-1;
  for(i = 0; i < dim-2; i++){
    pixel newPix;
    
    pixel currPix = src[RIDX(i, j, dim)];
    red = (int)currPix.red;
    blue = (int)currPix.blue;
    green = (int)currPix.green;

    currPix = src[RIDX(i+1, j, dim)];
    red += (int)currPix.red;
    blue += (int)currPix.blue;
    green += (int)currPix.green;

    currPix = src[RIDX(i+2, j, dim)];
    red += (int)currPix.red;
    blue += (int)currPix.blue;
    green += (int)currPix.green;

    newPix.green = (unsigned short)(green/3);
    newPix.blue = (unsigned short)(blue/3);
    newPix.red = (unsigned short)(red/3);

    dst[RIDX(i, j, dim)] = newPix;
  }

  pixel newPix;
  red = green = blue = 0;

  // Deal with the bottom-right-most square of pixels
  pixel currPix = src[RIDX(dim-2, dim-2, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[RIDX(dim-2, dim-1, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;
  
  currPix = src[RIDX(dim-1, dim-2, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[RIDX(dim-1, dim-1, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  newPix.red = (red/4);
  newPix.green = (green/4);
  newPix.blue = (blue/4);

  dst[RIDX(dim-2, dim-2, dim)] = newPix;

  // Deal with horizontal dim-2
  currPix = src[RIDX(dim-2, dim-1, dim)];
  red = (int)currPix.red;
  green = (int)currPix.green;
  blue = (int)currPix.blue;
  
  currPix = src[RIDX(dim-1, dim-1, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  newPix.red = (unsigned short)(red/2);
  newPix.green = (unsigned short)(green/2);
  newPix.blue = (unsigned short)(blue/2);

  dst[RIDX(dim-2, dim-1, dim)] = newPix;

  // Deal with vertical dim-2
  currPix = src[RIDX(dim-1, dim-2, dim)];
  red = (int)currPix.red;
  green = (int)currPix.green;
  blue = (int)currPix.blue;
  
  currPix = src[RIDX(dim-1, dim-1, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  newPix.red = (unsigned short)(red/2);
  newPix.green = (unsigned short)(green/2);
  newPix.blue = (unsigned short)(blue/2);

  dst[RIDX(dim-1, dim-2, dim)] = newPix;

  // Deal with last pixel
  dst[RIDX(dim-1, dim-1, dim)] = src[RIDX(dim-1, dim-1, dim)];
}

char motion_four_descr[] = "motion_four: Pulled some sub-expressions out of motion_three";
void motion_four(int dim, pixel *src, pixel *dst){
  int i, j;

  // Exclude the two right most columns and the two bottom most rows so that the conditional
  // and associated computations in weighted combo can be removed.
  for(i = 0; i < dim-2; i++)
    for(j = 0; j < dim-2; j++){
      dst[RIDX(i, j, dim)] = weighted_combo_four(dim, i, j, src);
    }


  int red, green, blue;
  red = green = blue = 0;

  // Go across the bottom two rows.
  j = dim-2;
  int k = dim-1;
  for(i = 0; i < dim-2; i++){
    pixel newPix;

    int pixInd = RIDX(j, i, dim);
    pixel currPix = src[pixInd];
    red = (int)currPix.red;
    green = (int)currPix.green;
    blue = (int)currPix.blue;

    currPix = src[RIDX(j, i+1, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(j, i+2, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(k, i, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(k, i+1, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(k, i+2, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    newPix.red = (unsigned short)(red/6);
    newPix.green = (unsigned short)(green/6);
    newPix.blue = (unsigned short)(blue/6);

    dst[pixInd] = newPix;
  }
  
  j = dim-1;
  for(i = 0; i < dim-2; i++){
    pixel newPix;
    
    int pixInd = RIDX(j, i, dim);
    pixel currPix = src[pixInd];
    red = (int)currPix.red;
    blue = (int)currPix.blue;
    green = (int)currPix.green;

    currPix = src[RIDX(j, i+1, dim)];
    red += (int)currPix.red;
    blue += (int)currPix.blue;
    green += (int)currPix.green;

    currPix = src[RIDX(j, i+2, dim)];
    red += (int)currPix.red;
    blue += (int)currPix.blue; 
    green += (int)currPix.green;

    newPix.green = (unsigned short)(green/3);
    newPix.blue = (unsigned short)(blue/3);
    newPix.red = (unsigned short)(red/3);

    dst[pixInd] = newPix;
  }

  // Go down the rightmost two rows
  j = dim-2;
  k = dim-1;
  for(i = 0; i < dim-2; i++){
    pixel newPix;

    int pixInd = RIDX(i, j,  dim);
    pixel currPix = src[pixInd];
    red = (int)currPix.red;
    green = (int)currPix.green;
    blue = (int)currPix.blue;

    currPix = src[RIDX(i+1, j, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(i+2, j, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(i, k, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(i+1, k, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    currPix = src[RIDX(i+2, k, dim)];
    red += (int)currPix.red;
    green += (int)currPix.green;
    blue += (int)currPix.blue;

    newPix.red = (unsigned short)(red/6);
    newPix.green = (unsigned short)(green/6);
    newPix.blue = (unsigned short)(blue/6);

    dst[pixInd] = newPix;
  }
  
  j = dim-1;
  for(i = 0; i < dim-2; i++){
    pixel newPix;
    
    int pixInd = RIDX(i, j, dim);
    pixel currPix = src[pixInd];
    red = (int)currPix.red;
    blue = (int)currPix.blue;
    green = (int)currPix.green;

    currPix = src[RIDX(i+1, j, dim)];
    red += (int)currPix.red;
    blue += (int)currPix.blue;
    green += (int)currPix.green;

    currPix = src[RIDX(i+2, j, dim)];
    red += (int)currPix.red;
    blue += (int)currPix.blue;
    green += (int)currPix.green;

    newPix.green = (unsigned short)(green/3);
    newPix.blue = (unsigned short)(blue/3);
    newPix.red = (unsigned short)(red/3);

    dst[pixInd] = newPix;
  }

  pixel newPix;
  red = green = blue = 0;
  int pixInd = RIDX(dim-2, dim-2, dim);

  // Deal with the bottom-right-most square of pixels
  pixel currPix = src[pixInd];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[RIDX(dim-2, dim-1, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;
  
  currPix = src[RIDX(dim-1, dim-2, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  currPix = src[RIDX(dim-1, dim-1, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  newPix.red = (red/4);
  newPix.green = (green/4);
  newPix.blue = (blue/4);

  dst[pixInd] = newPix;

  // Deal with horizontal dim-2
  pixInd = RIDX(dim-2, dim-1, dim);
  currPix = src[pixInd];
  red = (int)currPix.red;
  green = (int)currPix.green;
  blue = (int)currPix.blue;
  
  currPix = src[RIDX(dim-1, dim-1, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  newPix.red = (unsigned short)(red/2);
  newPix.green = (unsigned short)(green/2);
  newPix.blue = (unsigned short)(blue/2);

  dst[pixInd] = newPix;

  // Deal with vertical dim-2
  pixInd = RIDX(dim-1, dim-2, dim);
  currPix = src[pixInd];
  red = (int)currPix.red;
  green = (int)currPix.green;
  blue = (int)currPix.blue;
  
  currPix = src[RIDX(dim-1, dim-1, dim)];
  red += (int)currPix.red;
  green += (int)currPix.green;
  blue += (int)currPix.blue;

  newPix.red = (unsigned short)(red/2);
  newPix.green = (unsigned short)(green/2);
  newPix.blue = (unsigned short)(blue/2);

  dst[pixInd] = newPix;

  // Deal with last pixel
  dst[RIDX(dim-1, dim-1, dim)] = src[RIDX(dim-1, dim-1, dim)];
}

/*
 * motion - Your current working version of motion. 
 * IMPORTANT: This is the version you will be graded on
 */
char motion_descr[] = "motion: Current working version";
void motion(int dim, pixel *src, pixel *dst) 
{
  motion_four(dim, src, dst);
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
  add_motion_function(&motion_three, motion_three_descr);
  add_motion_function(&motion_four, motion_four_descr);
}

