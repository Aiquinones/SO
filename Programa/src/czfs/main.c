#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "API/cz_API.h"

FILE*input_file;

int main(int argc, char *argv[]){
  if(argc != 2){                                                                 // Si no se recibe un path de archivo
		printf("El uso correcto es: %s <sist.bin>\n", argv[0]);
		printf("Donde <sist.bin> es la ruta al archivo .bin a abrir\n");
		return 1;
	}


///////////////////////////////////////////////////////////////////////////////
//                      ENTENDIENDO LA LECTURA DEL ARCHIVO                   //
//                 Este código podría servir para hacer el ls                //
///////////////////////////////////////////////////////////////////////////////
	/* Abrimos el archivo en modo lectura */
  input_file = fopen(argv[1], "r+b");                                             // Abro el archivo asignandoselo a una variable global
  /* Si es que falló la apertura del archivo */
	if(!input_file){
		printf("¡El archivo %s no existe!\n", argv[1]);
		return 2;
	}

  /*  ESTE FUNCIONA PERFECT
  char new_file_name[11]  = "texto.txt";
  char old_file_name[11] = "hola.wav";
  printf("%d\n",cz_exists(new_file_name));
  cz_ls();
  cz_mv(old_file_name, new_file_name);
  cz_ls();
  printf("%d\n",cz_exists(new_file_name));
  */

  /* ESTE FUNCIONA PERFECT
  czFILE * fp = cz_open("texto.txt", 'r');
  printf("name:%s\n", fp -> name);
  printf("open mode: %c\n", fp -> open_mode);
  printf("size:%i\n", fp -> size);
  printf("creation date: %ld\n", fp -> creation);
  printf("modifcication date: %ld\n", fp -> modification);
  printf("index block: %i\n", fp -> index_block);
  printf("write_ptr: %i\n", fp -> write_ptr);
  printf("read_ptr: %i\n", fp -> read_ptr);

  cz_close(fp);*/

  czFILE * fp = cz_open("tet.txt", 'w');
  print_file(fp);
  char * lines = "hola picas haddad";
  int bytes_written = cz_write(fp, lines, strlen(lines));
  cz_close(fp);
  printf("%i bytes written\n", bytes_written);

  cz_ls();


  fp = cz_open("tet.txt", 'r');
  char read_lines[strlen(lines)];
  int bytes_read = cz_read(fp, read_lines, strlen(lines));
  printf("%i bytes read\n", bytes_read);

  printf("just read file, it said: %s\n", read_lines);

  cz_close(fp);

  /* Leemos las dimensiones del  a partir del archivo */
	fclose(input_file);

  return 0;
}
