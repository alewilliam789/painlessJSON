#include <arena.h>
#include <hashtable.h>
#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"

int main(int argc, char** argv) {

  FILE* current_file;

  
  

  for(int i = 1; i < argc; i++) {

    current_file = fopen(argv[i], "r");
    
    if(current_file == NULL) {
      return 0;
    }
    
    json_scanner(current_file,argv[i]);

    fclose(current_file);
  }
  
  return 1;
}
