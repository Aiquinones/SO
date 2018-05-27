#include "bindisc.h"

int file_address_to_int(unsigned char* buf, int size){                                     // TODO: debug
  int number = 0;
  number =  buf[size-2] << 8 | buf[size-1];
  return number;
}


int uc_to_int(unsigned char* buf, int size){                                     // TODO: debug
  int number = 0;
  if (size == 1){number = buf[size-1];}
  else if(size == 2){number =  buf[size-2] << 8 | buf[size-1];}
  //else if(size == 4){number =  buf[size-4] << 24 | buf[size-3] << 16 | buf[size-2] << 8 | buf[size-1];}
  //else if(size == 8){number =  buf[size-2] << 8 | buf[size-1];}
  return number;
}

/*Recibe un entero con el número del block a revisar, la cantidad de bloques a
 retornar  y el archivo en el cuál se buscan los bloques.*/                      // Aún no estoy seguro que funcione para más de un bloque
unsigned char* get_bloques(int block_number_offset, int n){                      // n es el número de bloques
  int block_byte = block_number_offset*1024;                                             // Número de Byte en que inicia el bloque
  unsigned char* block = malloc(1024*sizeof(unsigned char));
  fseek(input_file, block_byte, SEEK_SET);
  fread(block, 1024, n, input_file);                                                   // No entiendo por que no funciona si pongo sizeof(block) en lugar de 1024
  return block;                                                                  // Cano: block es un ptr, no tiene que ver con eso?
}
