/*
 * --CS 4400 Assingment 1--
 *
 * Author: Aaron Pabst
 */

#include<stdio.h>
#include<stdlib.h>

/*
 * Determines whether a char is a digit or not.
 * 1 if true
 * 0 otherwise
 */
short isDig(char c){
  if(c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5'
     || c == '6' || c == '7' || c == '8' || c == '9')
    return 1;
  else
    return 0;
}

/*
 * Returns a reformated str if the input matches the pattern.
 * Returns a string containing only a null terminator otherwise.
 */
char* aMode(char* str){
  char currChar = str[0];
  int i = 0;
  short isMatch = 1; // Start by assuming string is a match

  int charCounter = 0; // Used for counting how many times a char occurs
  int region = 0; // 0 for m, 1 for v, 2 for numerical

  char* returnVal = "";
  
  // Loop through all the chars in the array
  while(currChar != 0){ // Keep going until we hit a null terminator
    if(region == 0){
      if(currChar == 'm'){
	charCounter++;
      }
      else{
	if(charCounter % 2 == 0 || currChar != 'v'){ // Even number of m's, invalid
	  isMatch = 0;
	  break;
	}
	else{
	  charCounter = 1; // We know there's a V at this point
	  region++;
	}
      }
    }
    else if(region == 1){
      if(currChar == 'v'){
	charCounter++;
      }
      else{
	if(charCounter <= 1 || !isDig(currChar)){
	  isMatch = 0;
	  break;
	}
	else{
	  region++;
	  charCounter = 1;
	}
      }
    }
    else{
      if(isDig(currChar)){
	charCounter++;
      }
      else{
	isMatch = 0;
	break;
      }
    }

    //printf("%d\n", currChar);
    i++;
    currChar = str[i];
  }

  // Need one final check to make sure that we had the correct number of digits
  if((charCounter >= 1 && charCounter <= 3) && isMatch == 1){ // The string is a match
    return str;
  }
  else{ // The string is not a match
    return "";
  }
}

/*
 * Returns true if the input is a capital letter. False otherwise.
 */
short isCap(char ch){
  if(ch >= 'A' && ch <= 'Z')
    return 1;
  else
    return 0;
}

char* bMode(char* str){
  //printf("In B");
  char currChar = str[0];
  int i = 0;
  short region = 0;
  short isMatch = 1;
  int charCounter = 0;
  //printf("Allocating x");
  char* x = malloc(8 * sizeof(char)); // variable for stashing digit sequence
  //printf("X Allocated");
  short xNum = 1;
  int xInd = 2;

  while(currChar != 0){ // Spin through until we hit a null ptr
    if(region == 0){ // g sequence
      if(currChar == 'g'){
	charCounter++;
      }
      else{
	if(charCounter < 4 || !isDig(currChar)){
	  isMatch = 0;
	  break;
	}
	else{
	  //printf("passed 0 region\n");
	  region++;
	  x[0] = currChar; 
	  charCounter = 1;
	}
      }
    }
    else if (region == 1){ // decimal sequence/x sequence
      if(isDig(currChar)){
	charCounter++;
	x[xNum] = currChar;
	//x++;
	xNum++;

	if(xNum > 5){ // We need this check to avoid segmentation faults
	  isMatch = 0;
	  break;
	}
      }
      else{
	if(charCounter > 3 || currChar != 'n'){ // Invalid
	  isMatch = 0;
	  break;
	}
	else{
	  //printf("passed 1 region\n");
	  region++;
	  charCounter = 1;
	}
      }
    }
    else if(region == 2){ // n sequence
      if(currChar == 'n'){
	charCounter++;
      }
      else{
	if(charCounter < 2 || charCounter > 5 || currChar != x[0]){
	  //printf("%s\n", x);
	  isMatch = 0;
	  break;
	}
	else{
	  //printf("passed 2 region\n");
	  charCounter = 1;
	  region++;
	}
      }
    }
    else if(region == 3){ // even positioned x sequnce
      // Same as even numbered indexes in x
      if(currChar == x[xInd]){
	xInd += 2; // Will I get a seg fault from this??
      }
      else{
	if(!isCap(currChar)){ // Invalid
	  //printf("%d, %d\n", xNum, xInd);
	  isMatch = 0;
	  break;
	}
	else{
	  //printf("passed 3 region\n");
	  region++;
	}
      }
    }
    else if(region == 4){ // odd number of uppercase letters
      // Odd number of uppercase letters
      if(isCap(currChar)){
	charCounter++;
      }
      else{ // Auto fail
	isMatch = 0;
	break;
      }
    }

    i++;
    currChar = str[i];
  }

  if(isMatch == 1){
    // Replacement step
    currChar = str[0];
    i = 0;
    char* newStr = malloc(2*(i+1)*sizeof(char)); // Initilize a new string with twice the elements of the input
    int newStrInd = 0;

    while(currChar != 0){
      newStr[newStrInd] = currChar;
      char insert = (i % 8) + 48; // digits start at 48 on the ASCII table
      newStr[newStrInd+1] = insert;

      newStrInd += 2;
      i++;
      currChar = str[i];
    }

    return newStr;
  }
  else
    return "";
}

char* cMode(char* str){
  int charCounter = 0;
  char currChar = str[0];
  int i = 0;
  short region = 0;
  short isMatch = 1;
  char* X = malloc(64); // Probably should come up with a more robust solution than this...
  int xInd = 1;
  
  while(currChar != 0){
    if(region == 0){
      if(currChar == 'b'){
	charCounter++;
      }
      else{
	if(charCounter % 2 == 0 || !isCap(currChar)){
	  isMatch = 0;
	  break;
	}
	else{
	  region++;
	  //printf("Region 0 passed\n");
	  X[0] = currChar;
	  charCounter = 1;
	}
      }
    }
    else if(region == 1){
      //printf("Entered 1\n");
      if(isCap(currChar)){
	charCounter++;
	X[xInd] = currChar;
	xInd++;
      }
      else{
	if(charCounter % 2 == 0 || currChar != 'x'){
	  isMatch = 0;
	  break;
	}
	else{
	  //printf("region 1 passed\n");
	  region++;
	  charCounter = 1;
	}
      }
    }
    else if(region == 2){
      //printf("Entered 2\n");
      //printf("%s\n", X);
      //printf("%c\n", X[xInd-1]);
      //printf("%c\n", currChar);
      if(currChar == 'x'){
	charCounter++;
	//printf("Found x %d\n", charCounter);
      }
      else{
	if(charCounter > 2 || currChar != X[xInd-1]){
	  isMatch = 0;
	  break;
	}
	else{
	  //printf("Region 2 Passed\n");
	  region++;
	  xInd--;
	  charCounter = 1;
	}
      }
    }
    else if(region == 3){
      if(currChar == X[xInd-1]){
	xInd--;
      }
      else{
	if(xInd < 0 || !isDig(currChar)){
	  isMatch = 0;
	  break;
	}
	else{
	  region++;
	  //printf("Region 3 passed\n");
	}
      }
    }
    else if(region == 4){
      if(isDig(currChar)){
	charCounter++;
      }
      else{
	isMatch = 0; // Auto-fail
      }
    }
     
    i++;
    currChar = str[i];
  }

  if(charCounter > 3)
    isMatch = 0;

  if(isMatch == 1){
    // Run shift sequence
    currChar = str[0];
    char* newStr = malloc(128);
    int newStrInd = 0;
    i = 0;

    while(currChar != 0){
      if(currChar == 'G'){
	newStr[newStrInd] = 'G';
	newStr[newStrInd+1] = 'G';
	newStr[newStrInd+2] = 'G';
	newStrInd += 3;
      }
      else{
	newStr[newStrInd] = currChar;
	newStrInd++;
      }
      

      //printf("%s\n", newStr);
      i++;
      currChar = str[i];
    }
    return newStr;
  }
  else
    return "";
  
}

int main(int argc, char** argv){

  char option; // a, b, or c
  short tSelect = 0; // 1 if -t present
  int strStart; // The starting index of input strings in argv1

  // Parse parameters
  if(argv[1][0] == '-'){
    if(argv[1][1] == 't'){
      tSelect = 1;
      option = argv[2][1];
      strStart = 3;
    }
    else{
      option = argv[1][1];
      if(argv[2][1] == 't'){
	tSelect = 1;
	strStart = 3;
      }
      else{
	strStart = 2;
      }
    }
  }
  else{
    option = 'a';
    strStart = 1;
  }

  // Loop through all input strings and run specified pattern check
  int i;
  for(i = strStart; i < argc; i++){
    char* res;
    
    if(option == 'a'){
      res = aMode(argv[i]);
    }
    else if(option == 'b'){
      res = bMode(argv[i]);
    }
    else if(option == 'c'){
      res = cMode(argv[i]);
    }

    if(res[0] == 0){ // Not a match
      if(!tSelect)
	printf("no\n");
    }
    else{ // Match
      if(!tSelect){
	printf("yes\n");
      }
      else{
	printf("%s\n", res);
      }
    }
  }
} 
