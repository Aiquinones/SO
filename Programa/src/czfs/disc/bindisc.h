#include <stdlib.h>
#include <stdio.h>

extern FILE* input_file;                                                         // declaración global de la variable

////////////////////////////////////////////////////////////////////////////////
//               FUNCIONES AUXILIARES DE LECTURA DE DISCO                     //
////////////////////////////////////////////////////////////////////////////////

int file_address_to_int(unsigned char* buf, int size);

int uc_to_int(unsigned char* buf, int size);

unsigned char* get_bloques(int block_number_offset, int n);
