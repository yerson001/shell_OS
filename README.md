# Simulacon de terminal linux
![img](https://github.com/yerson001/shell_OS/blob/main/shell.PNG)
## COMPILAR
~~~
##: gcc main.c -std=c99
##: ./a.out
~~~
## COMANDOS SIMPLES
~~~
%> ls
%> date
%> grep u
~~~
## USANDO PIPE 
~~~
%> ls | grep u
~~~
##  '<' ALMACENAR EN UN ARCHIVO 
~~~
%> ls > in.txt
~~~
## '!!' ULTIMA LINEA DE INSTRUCCION (ejecuta la ultima instrucion del shell)
~~~
%> !!
~~~
## HISTORIAL (se almacena en un .txt)
~~~
%>cat history.txt
~~~
## '&' fork()
~~~
%> "instrucion " &
~~~
## SIGNAL (kill())
~~~
//parar (id proces)
%> parar 2546
//continuar
%> continuar 2546
~~~
![img](https://github.com/yerson001/shell_OS/blob/main/kill.gif)
## salir
~~~
%> exit
~~~
