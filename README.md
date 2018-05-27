# **Tarea 3**

**_Integrantes:_**  _José Luis Haddad (15635899) y Alejandro Quiñones (1463399J)_

----

### Decisiones de diseño:
- Modificar el nombre de archivo solo varía el directorio. Debido a esto, como
no es necesario acceder al bloque del archivo, no se considera una modificación
del nombre como una modificación del archivo (de la misma forma que ocurre en
  Ubuntu por ejemplo).

### Funciones adicionales:
### Observaciones:
- En cz_open el modo de apertura va en minúscula (r,w).
### Recordatorios:
- Manejo de errores.
- Timestamps de creación y modificación.
- Revisar casos borde (último bloque, último puntero, etc.)
- Si al esribir un archivo se acaba la memoria, se da aviso y luego se lanza un
error, pero No de borra el archivo.
- Revisar que el sistema no entregue archivos si estos fueron ya borrados
