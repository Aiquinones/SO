#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "../disc/bindisc.h"


// Representaun archivo abierto
typedef struct czfile{
  char * name;
  char open_mode;                                                                // "r" or "w"
  int size;
  time_t creation;
  time_t modification;
  int index_block;
  int read_ptr;
  int write_ptr;
}czFILE;

void print_file(czFILE * fp);

/* Si mode es 'r' y filename está en el directorio lo retorna. NULL e.o.c.
   Si mode es 'w' se verifica que filename no exista en el directorio y se crea
   un czFILE para retornar. NULL e.o.c.
   Otros errores también retornan NULL*/
czFILE* cz_open(char* filename, char mode);

/* Retorna 1 si el archivo existe,  0 e.o.c.*/
int cz_exists(char* filename);

/* Lee los siguientes nbytes de file_desc y los guarda en buffer.
Funciona como readlines.
Retorna la cantidad de Bytes efectivamente leidos.
Retorna -1 si se produce un error. */
int cz_read(czFILE*  file_desc, void* buffer, int nbytes);

/* Escribe en el archivo descrito por file_desc los nbytes que se encuentren
en la direcciónindicada en buffer.
Retorna el número de Bytes escritos si no hay errores.
Retorna 0 si se produjo un error porque no se pudo seguir escribiendo (por falta
de espacio en disco o tamaño máximo del archivo alcanzado).
Retorna -1 e.o.c. */
int cz_write(czFILE*  file_desc, void* buffer, int nbytes);

/* Cierra el archivo file_desc ssi este ya se encuentra actualizado en el
disco. */
int cz_close(czFILE* file_desc);

/* Renombra el archivo origin con el nuevo nombre destination, cuidando que
destination no esté siendo ya utilizado por otro archivo. */
int cz_mv(char* origin, char* destination);

/* Copia el archivo de nombre origin a uno de nombre destination.
Solo acepta nombres distintos. Hacer "hardlink". */
int cz_cp(char* origin, char* destination);

/* Elimina, si existe, el archivo de nombre filename del bloque de directorio
 y deja libres los bloques que estaban siendo usados por el archivo. */
int cz_rm(char* filename);

/* Escribe en pantalla los nombres de todos los archivos contenidos en el
directorio principal. */
void cz_ls();
