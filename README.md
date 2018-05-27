# **Tarea 3**

**_Integrantes:_**  _José Luis Haddad (15635899) y Alejandro Quiñones (1463399J)_

----

### Decisiones de diseño:
- Para trabajar sobre el disco virtual se accede a este en modo lectura y escritura de bytes (r+b) y se lee y modifica mediante las funciones `fread()` y `fwrite()`.  Decidimos enfrentar el problema de esta manera en lugar de leer bloques y asignarlos a estructuras debido a que de esta forma se actualizan los resultados de manera inmediata.
- Debido a esto mismo, el contenido del archivo no forma parte en ningún momento de la estructura czFILE y por lo tanto esta no presenta mayores complicaciones al momento de ser cerrada.
- Modificar el nombre de archivo solo varía el directorio. Debido a esto, como
no es necesario acceder al bloque del archivo, no se considera una modificación
del nombre como una modificación del archivo (de la misma forma que ocurre en Ubuntu por ejemplo) y por lo tanto, no se actualiza el timestamp de modificación.
- El problema se enfrentó bajo una lógica de administración de archivos Big-Endian.



### Funciones adicionales:
1. `uc_to_int()` y `file_address_to_int()`: permiten convertir un arreglo de bytes a un entero. Se usan para reconocimiento de número de bloques, size, etc.
2. `get_bloques()`: Dado un número de bloque y una cantidad de bloques (no se usa) entrega un bloque.
3. `update_bitmap()`: actualiza el estado de un bit del bitmap equivalente a un bloque.
4. `position_in_directory()`: entrega la entrada del directorio en la cual se entrega el archivo de nombre filename
5. `first_available_dir()` y `first_available_bitmap()`: retornan, respectivamente, la primera entrada disponible den el directorio y el primer bloque disponible respresentado por un bit de valor cero en el bitmap.
6. `print_file()`: imprime todos los atributos de la estructura czFILE.

### Observaciones:
- En cz_open el modo de apertura va en minúscula (r,w).
