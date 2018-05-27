#include "cz_API.h"
#include <stdbool.h>
#include "../disc/bindisc.h"
#include <string.h>
#include <time.h>

bool debugging = true;

void print_file(czFILE * fp){
  printf("name:%s\n", fp -> name);
  printf("open mode: %c\n", fp -> open_mode);
  printf("size:%i\n", fp -> size);
  printf("creation date: %ld\n", fp -> creation);
  printf("modifcication date: %ld\n", fp -> modification);
  printf("index block: %i\n", fp -> index_block);
  printf("write_ptr: %i\n", fp -> write_ptr);
  printf("read_ptr: %i\n", fp -> read_ptr);
}

void append(char* s, char c) {
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

int get_empty_block_ptr(){
  char * c = malloc(sizeof(char));

  fseek(input_file, 0, SEEK_SET); // 1024 * 1 = bloque 1
  for (int bitmap_ptr = 0; bitmap_ptr < 200; bitmap_ptr++){//1024 * 8; bitmap_ptr++){
    fread(c, 1, 1, input_file);

    printf("%s\n", c);
    /*
    for (int bit = 0; bit < 8; bit++){
      printf("%i\n", (int) c >> bit & 1);
    }
    */
  }
  free(c);
  return 0;
}

int get_data_ptr(czFILE* file_desc){
  unsigned char * ptr_buffer[4];
  int data_ptr = 0;
  int line_ptr = file_desc -> read_ptr / 1024;
  //indirect..
  if (line_ptr > 251){

    if(debugging){printf("line_ptr > 251... INDIRECT\n");}

    //line_ptr ahora es la linea DENTRO del indirect block
    line_ptr = line_ptr - 252;

    // buscamos el indirect_ptr
    fseek(input_file, file_desc -> index_block * 1024 + 12 + 1008, SEEK_SET);
    fread(ptr_buffer, 1, 4, input_file);
    int indirect_ptr = 0;
    //indirect_ptr = file_address_to_int(ptr_buffer[0], 4);

    // buscamos el data ptr
    fseek(input_file, indirect_ptr * 1024 + line_ptr, SEEK_SET);
  }
  //direct..
  else{

    if(debugging){printf("line_ptr < 251... DIRECT\n");}

    // buscamos el data ptr
    fseek(input_file, file_desc -> index_block * 1024 + 12 + line_ptr, SEEK_SET);
  }

  // leemos data_ptr y retornamos
  fread(ptr_buffer, 1, 4, input_file);
  data_ptr = file_address_to_int(ptr_buffer[0], 4);
  return data_ptr;
}

int min2(int choice1, int choice2){
  if (choice1 < choice2){return choice1;}
  return choice2;
}

void update_bitmap(int block, int valid_bit){                                    // actualiza el bit del bloque block en el bitmap
  int bitmap_block_number = (block / (1024*8)) + 1;                              // obtengo el índice respecto al disco del bloque del bitmap
  int bitmap_block_byte = (block % (1024*8))/8;                                  // obtengo el indice del byte en el bloque correspondiente
  int bitmap_block_byte_bit = block % 8;                                         // obtengo el índice del bit en el byte del bloque correspondiente
  unsigned char *bitmap_block = get_bloques(bitmap_block_number, 1);
  unsigned char byte_to_change = bitmap_block[bitmap_block_byte];
  unsigned char aux;
  if(valid_bit){
    aux = 0x01 << (7-bitmap_block_byte_bit);
    byte_to_change = byte_to_change | aux;
  }
  else{
    aux = ((0x01 << (7-bitmap_block_byte_bit)) ^ 0xFF);
    byte_to_change = byte_to_change & aux;
  }

  int offset = (bitmap_block_number)*1024 + bitmap_block_byte;
  fseek(input_file, offset, SEEK_SET);
  fwrite(&byte_to_change, 1, 1, input_file);
}

static int position_in_directory(char* origin){
  unsigned char* bloque = get_bloques(0, 1);
  int len_origin = strlen(origin);
  int file_entry = -1;
  char character;

  for(int i=0; i<64; i++){                                                       // Para cada fila del bloque
    char file_name[11]="";                                                       // la usaré para ir revisando los nombres de los archivos
    bool se_agrega = true;
    for(int j=0; j<12; j++){                                                     // Trabajo con los primeros 12 bytes (validez y nombre)
      if(!j){if(!bloque[i*16+j]){se_agrega = false;}}                           // S es que el bloque es válido o no
      if(j>0 && se_agrega){if(!bloque[i*16+j]){se_agrega = false;}}            // Solo imprime hasta q encuentra un null
      if(se_agrega && j>0){
        character = (char)bloque[i*16+j];
        append(file_name, character);
      }
    }

    // Comparar file_name con origin.
    bool identicas = true;
    if(strlen(file_name) == len_origin){
      for(int l=0; l<len_origin; l++){
        if(origin[l] != file_name[l]){identicas = false;}
      }
      if(identicas){file_entry = i*16+1;}
      }
    }
    return file_entry;
}

void update_mod_time(czFILE * file){
  int update_ptr = file -> index_block * 1024 + 8; //+4 se salta el size +4 el creation date
  time_t * timestamp = malloc(4);
  fseek(input_file, update_ptr, SEEK_SET);
  fwrite(timestamp, 4, 1, input_file);
  free(timestamp);
}

/* Retorna la primera fila del directorio disponible y -1 si no hay*/
static int first_available_dir(){
  unsigned char* bloque = get_bloques(0, 1);
  int available_dir_entry = -1;
  for (int i = 0; i < 64; i++){
    if(bloque[i*16]){
      available_dir_entry = i;
      break;
    }
  }
  return available_dir_entry;
  printf("available dir entry chosen: %i\n", available_dir_entry);
}

//DEBUGGED
static int first_available_bitmap(){
  int fa_block = -1;

  for(int bm_block = 1; bm_block < 9; bm_block++){                               // Para cada uno de los bloques del bitmap

    unsigned char* bloque = get_bloques(bm_block, 1);                            // Obtengo el bloque en un buffer llamado bloque
    for(int block_byte = 0; block_byte < 1024; block_byte++){                    // Para cada byte de ese bloque
      unsigned char byte = bloque[block_byte];                       // Obtengo el byte en base al puntero al bloque y el numero de byte
      for(int bit = 0; bit < 8; bit++){                                          // Para cada bit de ese byte
        if( (byte & ( 1 << (7 - bit) )) >> (7 - bit) == 0){
          fa_block = bm_block * 1024 * 8 + block_byte * 8 + bit;
          return fa_block;
        };
      }
    }
  }
  return fa_block;
}

/*czFILE* cz\_open(char* filename, char mode). Si mode es ‘r’, busca filename en el directorio
y retorna un czFILE* que lo representa, en caso de existir el archivo; si el archivo no existe, se retorna
NULL. Si mode es ‘w’, se verifica que el archivo no exista en el directorio y se retorna un nuevo czFILE*
que lo representa. Si el archivo ya exist´ıa, se retorna NULL. Cualquier otro tipo de error tambien debe retornar ´
NULL.*/

// DEBUGGED
czFILE* cz_open(char* filename, char mode){
  if(debugging){printf("<CZ_OPEN>\n");}

  czFILE* new_file = malloc(sizeof(czFILE));
  int len_filename = strlen(filename);
  if(len_filename>10){                                                           // TODO: manejo de errores
    fprintf(stderr, "Nombre de largo inválido\n");
    if(debugging){printf("</CZ_OPEN>\n");}
    return NULL;}

  int file_exists = cz_exists(filename);
  if(mode == 'r'){
    if(!file_exists){                                                            // TODO: manejo de errores
      fprintf(stderr, "El archivo que intenta leer no existe\n");
      if(debugging){printf("</CZ_OPEN>\n");}
      return NULL;
    }
    else{                                                                   // Abro el archivo en modo lectura

      int file_position_in_dir = position_in_directory(filename);
      unsigned char index_block_pointer[2];
      unsigned char file_size[4];
      unsigned char file_creation[4];
      unsigned char file_modification[4];

      new_file -> name = filename;
      new_file -> open_mode = mode;

      fseek(input_file, file_position_in_dir + 14 , SEEK_SET);                   // Para estar en los dos últimos bytes de la entrada, correspondientes al pointer to index_block
      fread(index_block_pointer, 2, 1, input_file);
      new_file -> index_block = uc_to_int(index_block_pointer, 2);
      new_file -> read_ptr = new_file -> index_block*1024 + 12;

      fseek(input_file, new_file -> index_block*1024, SEEK_SET);
      fread(file_size, 4, 1, input_file);
      fread(file_creation, 4, 1, input_file);
      fread(file_modification, 4, 1, input_file);
      new_file -> size = uc_to_int(file_size, 4);
      new_file -> creation = uc_to_int(file_creation, 4);                        // TODO: Es necesario convertirlo?
      new_file -> modification = uc_to_int(file_modification, 4);                // TODO: Es necesario convertirlo?
      /*Asignar a new_file filename y mode.
        Ir a buscar al index_block el tamaño del archivo, y ambos Timestamps,
        (12 primeros bytes, 4 por atributo), asignar el índex_block al atributo
        correspondiente y setear el read_ptr en index_block + 12(?)*/
    }
  }
  else if(mode == 'w'){
    if(file_exists){                                                             // TODO: manejo de errores
      fprintf(stderr, "El archivo que intenta crear ya existe\n");
      if(debugging){printf("</CZ_OPEN>\n");}
      return NULL;
    }
    else{

      int dir_entry = first_available_dir();
      if(dir_entry < 0){                                                         // TODO: manejo de errores
        fprintf(stderr, "No hay espacio disponible en el directorio para más entradas\n");
        if(debugging){printf("</CZ_OPEN>\n");}
        return NULL;
      }                                                                                           // Si es que no queda espacio en el directorio
      int bitmap_entry = first_available_bitmap();
      if(bitmap_entry < 0){                                                      // TODO: manejo de errores
        fprintf(stderr, "No hay bloques disponibles para más archivos\n");
        if(debugging){printf("</CZ_OPEN>\n");}
        return NULL;
      }

      unsigned char valid_bit = 0x01;
      int timestamp = (unsigned)time(NULL);                                      // Registro el timestamp de creación
      new_file -> name = filename;
      new_file -> open_mode = mode;
      new_file -> index_block = bitmap_entry;
      new_file -> read_ptr = new_file -> index_block*1024 + 12;
      new_file -> size = 0;
      new_file -> creation = timestamp;                                          // TODO: Es necesario convertirlo?
      new_file -> modification = timestamp;                                      // TODO: Es necesario convertirlo?
      fseek(input_file, dir_entry, SEEK_SET);                                    // Ubico el puntero en la entrada q asignaré
      fwrite(&valid_bit, 1, 1, input_file);                                      // Cambio a uno el byte de validez
      unsigned char file_name_format[11];
      for(int m = 0; m<len_filename;m++){
        file_name_format[m]=filename[m];
      }
      file_name_format[len_filename] = '\0';
      fwrite(file_name_format, len_filename+1 , 1,input_file );                  // Escribo el nombre del archivo en la entrada
      unsigned char index_block_number[2];                                       // Paso el index de int a binario
      index_block_number[0]= (unsigned char)(bitmap_entry >> 8);
      index_block_number[1]= (unsigned char)bitmap_entry;
      fwrite(index_block_number, 2 , 1, input_file );                            // Escribo el index_block_number del archivo en la entrada

      unsigned char creation_TS[4];
      creation_TS[0]= (unsigned char)(timestamp >> 24);
      creation_TS[1]= (unsigned char)(timestamp >> 16);
      creation_TS[2]= (unsigned char)(timestamp >> 8);
      creation_TS[3]= (unsigned char)timestamp;
      fseek(input_file, new_file->index_block*1024 + 4, SEEK_SET);               // Voy al byte inicial del index block y avanzo 4 bytes
      fwrite(&creation_TS, 4, 1, input_file);                                    // Escribo los cuatro bytes correspondientes al timestamp

      /*Buscar en el directorio la primera entrada con el primer byte en cero y
        escribir en esta el nombre del archivo deseado. Buscar en el bitmap el
        primer bloque con bit de validez en cero, cambiarlo a 1, tomar su número
        y registrarlo como bloque índice del nuevo archivo.
        Asignar a new_file filename y mode, asignar el Timestamp de creación,
        asignar index_block y fijar read_ptr en cero. (size y modification se
        fijan al hacer write)*/
    }
  }
  else{
    fprintf(stderr,"Modo de apertura inválido, elija 'r' o 'w'");                        // TODO: manejo de errores
    return NULL;
  }

  if(debugging){printf("</CZ_OPEN>\n");}
  return new_file;
}

// DEBUGGED
int cz_exists(char* filename){
  if(debugging){printf("<CZ_EXISTS>\n");}

  int size = strlen(filename);
  //printf("%i\n", size);

  unsigned char* dir = get_bloques(0, 1);                                        // busco en el directorio
  for (int i = 0; i < 64; i++){                                                  // busco en cada linea
    int offset = i * 16;                                                         //byte inicial de cada linea

    bool equal = true;

    if(dir[offset]){                                                             // if validated

      int j;
      for (j = 1; j < size + 1; j++){                                            // if hay un char distinto, este no es el archivo
        // printf("bin: %c\n", dir[offset + j]);
        // printf("name: %c\n", filename[j - 1]);
        if(dir[offset + j] != filename[j - 1]){
          // printf("1 break!\n");
          equal = false;
          break;
        }
      };

      if ((j == 10 || !dir[offset + j + 1]) && equal){                           // si después del nombre, hay más caracteres, entonces el archivo partía igual, pero no tiene el mismo nombre
        if(debugging){printf("</CZ_EXISTS>\n");}
        return 1;
      };
      //TODO: works for texto.txt, debug a.o.c
    }
  }
  if(debugging){printf("</CZ_EXISTS>\n");}
  return 0;
}

int cz_read(czFILE*  file_desc, void* buffer, int nbytes){
  if(debugging){printf("<CZ_READ>\n");}

  if (file_desc == NULL){
    printf("Este czFILE no existe!\n");
    //TODO: stderr
    if(debugging){printf("</CZ_READ>\n");}
    return 0;
  }

  if (file_desc -> open_mode != 'r'){
    printf("Archivo está en modo %c\n", file_desc -> open_mode);
    //TODO stderr
    if(debugging){printf("</CZ_READ>\n");}
    return -1;
  }

  char * buf[nbytes];

  int line_ptr = file_desc -> read_ptr % 1024;
  int data_ptr;

  int currently_read = 0;
  int will_read = 0;

  while (currently_read < nbytes && file_desc -> read_ptr < file_desc -> size) { //mientras que

     data_ptr = get_data_ptr(file_desc);

    // Definimos cunatos bytes vamos a leer esta iteración (aka will_read)
    if ((file_desc -> size / 1024) - (file_desc -> read_ptr / 1024) != 0){ //si no estoy en la último block..
      will_read = min2(nbytes - currently_read,  // leo cuantas me pidan
                       1024 - file_desc -> read_ptr % 1024);  //si me piden mucho, leo lo que haya en el block actual no más
    }
    else{  // si estoy en el último block...
      will_read = min2(nbytes - currently_read, //leo cuantas me pidan
                      file_desc -> size - file_desc -> read_ptr);  // a menos que con eso me pase
    }

    if (debugging){printf("will read: %i\n", will_read);}

    // leemos will_read bytes del bin
    char * pre_buf[will_read];
    fseek(input_file, data_ptr * 1024 + line_ptr, SEEK_SET);
    fread(pre_buf, 1, will_read, input_file);

    // lo pasamos a nuestro ptr_buffer
    for (int i = 0; i < will_read; i++){
      buf[currently_read + i] = pre_buf[i];
    }

    /*TODO: creo que en fread, si en vez de pasar pre_buf, paso buf[currently_read],
    no tengo que pasar de pre_buf a buf. check.

    Also, creo que fread paro solo (alone) si la página terminó, el código, thus,
    podría ser más simple*/

    currently_read = currently_read + will_read;
    file_desc -> read_ptr = file_desc -> read_ptr + will_read;
  }

  buffer = buf;

  if(debugging){printf("</CZ_READ>\n");}

  return currently_read;
}

int cz_write(czFILE*  file_desc, void* buffer, int nbytes){
  if(debugging){printf("<CZ_WRITE>\n");}

  if (file_desc == NULL){
    printf("Este czFILE no existe!\n");
    if(debugging){printf("</CZ_WRITE>\n");}
    //TODO: stderr
    return 0;
  }

  if (file_desc -> open_mode != 'w'){
    printf("Archivo está en modo %c\n", file_desc -> open_mode);
    //TODO stderr
    if(debugging){printf("</CZ_WRITE>\n");}
    return -1;
  }

  if (nbytes == 0){
    update_mod_time(file_desc);
    return 0;}

  char myBuffer[nbytes];
  for (int i = 0; i < nbytes; i++){
    myBuffer[i] = ((char *) buffer)[i];
  }

  int bytes_written = 0;
  int bytes_left = nbytes;
  int bytes_to_max_size = 508 * 1024 - file_desc -> size; // (252 + 256) ptrs en index + indirect
  int space_in_page;
  int writing_ptr;

  while (bytes_written < nbytes) {

    //si no tiene página o página está llena
    if (file_desc -> write_ptr == -1 || file_desc -> write_ptr % 1024 == 0){
      int bm = first_available_bitmap();
      update_bitmap(bm, 1);

      //TODO: if bm = -1 entonces no quedan páginas ERROR
      writing_ptr = bm * 1024;
      space_in_page = 1024;

      file_desc -> write_ptr = writing_ptr;
    }

    //si sí tiene página para escribir
    else{
      writing_ptr = file_desc -> write_ptr;
      space_in_page = 1024 - file_desc -> write_ptr % 1024;
    }

    //veo cuánto voy a escribir esta iteración
    fseek(input_file, writing_ptr, SEEK_SET);
    int will_write = min2(bytes_left, space_in_page);
    will_write =     min2(will_write, bytes_to_max_size);

    char * writing_buffer = malloc(sizeof(char)*will_write);
    for (int i = 0; i < will_write; i++ ){
      writing_buffer[i] = myBuffer[bytes_written + i];
    }

    //escribo
    fwrite(writing_buffer, 1, will_write, input_file);

    //update vars
    file_desc -> write_ptr = file_desc -> write_ptr + will_write;
    bytes_left = bytes_left - will_write;
    bytes_written = bytes_written + will_write;
    file_desc -> size = file_desc -> size + will_write;
    bytes_to_max_size = bytes_to_max_size - will_write;

    free(writing_buffer);

    if (bytes_to_max_size == 0 && will_write == 0){
      //ERROR: se llegó al tamaño máximo
      break;
    }
  }
  /*Además de modificar el disco, registrar el tamaño en file_desc y la fecha de
    modificación del archivo como el Timestamp al finalizar la ejecución de esta
    función.*/

  update_mod_time(file_desc);
  if(debugging){printf("</CZ_WRITE>\n");}
  return bytes_written;
}

//DEBUGGED
int cz_close(czFILE* file_desc){
  if(debugging){printf("<CZ_CLOSE>\n");}
  free(file_desc);
  if(debugging){printf("</CZ_CLOSE>\n");}
  return 0;
}

//DBUGGED
int cz_mv(char* origin, char* destination){
  if(debugging){printf("<CZ_MV>\n");}
  unsigned char* bloque = get_bloques(0, 1);
  /*Reviso si origen y destino son de menos o igual a 11 bytes.*/
  int len_origin = strlen(origin);
  int len_destination = strlen(destination);
  if(len_origin > 11 || len_destination > 11){
    fprintf(stderr, "Largo del nombre incorrecro! Move no realizado\n");
    return 1;                                                                    // TODO: Manejo de errores. Nombre de origen o destino tiene largo inválido.
  }

  bool destination_used = false;                                                 // Bool que me dice si el nombre de destino esta siendo ya ocupado.
  char character;
  int file_entry;

  for(int i=0; i<64; i++){                                                       // Para cada fila del bloque
    char file_name[11]="";                                                       // la usaré para ir revisando los nombres de los archivos
    bool se_imprime = true;
    for(int j=0; j<12; j++){                                                     // Trabajo con los primeros 12 bytes (validez y nombre)
      if(!j){if(!bloque[i*16+j]){se_imprime = false;}}                           // S es que el bloque es válido o no
      if(j>0 && se_imprime){if(!bloque[i*16+j]){se_imprime = false;}}            // Solo imprime hasta q encuentra un null
      if(se_imprime && j>0){
        character = (char)bloque[i*16+j];
        append(file_name, character);
      }
    }

    // Comparar file_name con origin y destination.
    bool identicas = true;
    if((strlen(file_name) == len_destination) && (!destination_used)){
      for(int k=0; k<len_destination; k++){
        if(destination[k] != file_name[k]){identicas = false;}
      }
      if(identicas){destination_used = true;}
    }
    identicas = true;
    if(strlen(file_name) == len_origin){
      for(int l=0; l<len_origin; l++){
        if(origin[l] != file_name[l]){identicas = false;}
      }
      if(identicas){file_entry = i*16+1;}
      }
    }
  if(!destination_used){
    fseek(input_file, file_entry, SEEK_SET);                                     // move to offset file_entry from begin of file
    unsigned char new_name[11];
    for(int m = 0; m<len_destination;m++){new_name[m]=destination[m];}
    new_name[len_destination] = '\0';
    fwrite(new_name, len_destination+1 , 1,input_file );
  }
  if(debugging){printf("</CZ_MV>\n");}
  return 0;
}

int cz_cp(char* origin, char* destination){
  if(debugging){printf("<CZ_CP>\n");}
  if (!strcmp(origin, destination)){
    printf("SON IGUALES!!!\n");
    // TODO: stderr
    return 1;
  }

  czFILE * origin_file = cz_open(origin, 'r');
  czFILE * destination_file = cz_open(destination, 'w');

  char * copy_buffer = malloc(origin_file -> size);

  cz_read(origin_file, copy_buffer, origin_file -> size);
  cz_close(origin_file);

  cz_write(destination_file, copy_buffer, origin_file -> size);
  cz_close(destination_file);


  free(copy_buffer);

  if(debugging){printf("</CZ_CP>\n");}
  return 0;
}

int cz_rm(char* filename){
  unsigned char index_block_pointer[2];                                          // Puntero al bloque indice
  unsigned char* valid_bit = 0x00;                                               // Bit de validez cero
  int file_address = position_in_directory(filename);                            // Busco la direccion en el bloque de dirección del archivo

  fseek(input_file, file_address*64, SEEK_SET);
  fwrite(valid_bit, 1, 1, input_file);                                           // Anulo el file en el directorio
  fseek(input_file, file_address*64 + 14 , SEEK_SET);                            // Para estar en los dos últimos bytes de la entrada, correspondientes al pointer to index_block

  int index_block_number = uc_to_int(index_block_pointer, 2);

  fseek(input_file, index_block_number*1024 + 12, SEEK_SET);

  for(int puntero = 0; puntero<252; puntero++){

    unsigned char pointer_to_data_block[4];
    fread(pointer_to_data_block, 4, 1, input_file);

    int pointer_as_int = uc_to_int(pointer_to_data_block, 4);
    update_bitmap(pointer_as_int, 0);
  }

  unsigned char file_size_bytes[4];
  fseek(input_file, index_block_number*1024, SEEK_SET);
  fread(file_size_bytes, 4, 1, input_file);
  int file_size = uc_to_int(file_size_bytes, 4);
  if(file_size > 252*1024){
    unsigned char* indirect_block_pointer[2];
    fseek(input_file, index_block_number*1024 + 1020, SEEK_SET);
    fread(indirect_block_pointer, 2, 1, input_file);
    int indirect_block_number = uc_to_int(indirect_block_pointer, 4);

    fseek(input_file, indirect_block_number*1024, SEEK_SET);
    for(int puntero = 0; puntero<256; puntero++){

      unsigned char pointer_to_data_block[4];
      fread(pointer_to_data_block, 4, 1, input_file);

      int pointer_as_int = uc_to_int(pointer_to_data_block, 4);
      update_bitmap(pointer_as_int, 0);
    }
    update_bitmap(index_block_number, 0);
  }
  return 0;
}

//DEBUGGED
void cz_ls(){
  if(debugging){printf("<CZ_LS>\n");}
  unsigned char* bloque = get_bloques(0, 1);
    for(int i=0; i<64; i++){                                                       // Para cada fila del bloque
      bool se_imprime = true;
      bool se_imprimio = true;
      for(int j=0; j<12; j++){                                                     // Trabajo con los primeros 12 bytes (validez y nombre)
        if(!j){if(!bloque[i*16+j]){se_imprime = false; se_imprimio = false;}}      // S es que el bloque es válido o no
        if(j>0){if(!bloque[i*16+j]){se_imprime = false;}}                          // Solo imprime hasta q encuentra un null
        if(se_imprime && j>0){
          printf("%c", bloque[i*16+j]);
        }
      }

      if(se_imprimio){printf("\n");}
    }
    if(debugging){printf("</CZ_LS>\n");}
}
