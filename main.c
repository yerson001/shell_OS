#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINE 1024            
int EsperaPadreWait = 1;           
int input_ = 0;    
int output_ = 0;   
int saved_stdin, saved_stdout;      
int in, infile_index, out, outfile_index; //direccion de los files
int Pipe_start, Pipe_Simbolo, Second_comand;      

int quit = 0;
char *P_I_D;
int p_i_d;
int empty_command = 0;
int command_count = 0;

void SplitCommand(char * command, char ** args);
void Directorio( char ** args);
void Init();
void VerificarPipes(char ** args);
void Escribir(char *a);


int main(void) { 

    int should_run = 1; 
    pid_t pid;              
    char **args = malloc(sizeof(char) * (MAX_LINE/2 + 1));          //guarda los comandos
    char *command = malloc(sizeof(char) * MAX_LINE);                //comando actual
    char *command_history_buffer = malloc(sizeof(char) * MAX_LINE); //para el historial
    int state, state2;      //pipe                          
    int fd[2];          //canales
    char ** args1 = malloc(sizeof(char) * (MAX_LINE/2 + 1));    //primer argumento para pipe
    char ** args2 = malloc(sizeof(char) * (MAX_LINE/2 + 1));    //segundo argumento para pipe
    char *ComandActual = malloc(sizeof(char) * MAX_LINE);       // comando actual

    while(should_run && quit == 0) {   
        Init();
        //kill(2539,SIGSTOP);
        printf("$>> ");
        memset(command, 0, MAX_LINE);
        fgets (command, MAX_LINE, stdin);
        fflush(stdout);       

        if(strlen(command) <= 0) {   
            return 0;      
        } 
        else {
            strcpy(ComandActual, command);
            SplitCommand(ComandActual, args);
  
            //=============history=================
            if(strcmp(args[0],"parar")==0) {
                    //print_history();
                    printf("stop: %s\n",args[1]);
                    P_I_D = args[1]; 
                    p_i_d = atoi(P_I_D);
                    kill(p_i_d,SIGSTOP);
             continue;
            }
             else if(strcmp(args[0],"continuar")==0) {
                    //print_history();
                    printf("restart: %s\n",args[1]);
                    P_I_D = args[1];
                    p_i_d = atoi(P_I_D);
                    kill(p_i_d,SIGCONT);
             continue;
            }

            int history_cmd = 0;
            for(int i=0; args[i] != NULL; i++) {
                if(strcmp(args[i], "!!") == 0){// 0 si son iguales
                    history_cmd = 1;/*
                        for(int i=0; i< it; i++)
                            printf("args[%d]:%s\n", i, args[i]);*/
                }
                        
            }

            if(history_cmd == 1) { 
                if(command_count <= 0) { 
                      printf("No hay historial \n");
                    continue;
                }
                printf("%s\n", command_history_buffer);
                strcpy(command, command_history_buffer); 
                strcpy(ComandActual, command);
                SplitCommand(ComandActual, args);
            }
            // almacenamos command en el historial=========0
            strcpy(command_history_buffer, command);
            command_count++;

            //==========verifivar=========
            Directorio(args);
            VerificarPipes(args);

            //====verificar si los archivos txt estan creados
            if(input_ == 1 ) {
                in = open(args[infile_index], O_RDONLY);
                if(in < 0) {
                    printf("no se puede abrir %s \n", args[infile_index]);
                    return 1;
                }
                else {
                    saved_stdin = dup(0);
                    dup2(in, 0);// cerrado para lectura
                    close(in);
                }                
                
            }

            if(output_ == 1) {
                out = open(args[outfile_index], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
                if(in < 0) {
                    printf("no se puede abrir %s \n", args[outfile_index]);
                    return 1;
                }
                else {
                    saved_stdout = dup(1);
                    dup2(out, 1);//cerrado para escritura
                    close(out);
                }                
            }

            //====las tuverias=== si las hay======= se activa cuando se se encuantra  "|"
            if(Pipe_start == 1) {
                for(int i =0; i <= Pipe_Simbolo; i++) {
                    if(i == Pipe_Simbolo) {
                        args1[i] = NULL;
                        break;
                    }
                    args1[i] = args[i];
                }
                int last_i = 0;
                for(int i = Pipe_Simbolo + 1; args[i] != NULL; i++) {
                    args2[i - Pipe_Simbolo - 1] = args[i];  
                    last_i = i;                 
                }
                args2[last_i + 1] = NULL;                
            }

            //=======verificamos el exit==========
            for(int i=0; args[i] != NULL;i ++){
                if(strcmp(args[i], "exit") == 0)
                        quit = 1;// free_history();
            }

            //=======verificar un commando vacio
            if(args[0] == NULL || strcmp(args[0], "\0") == 0 || strcmp(args[0], "\n") ==0)
                        empty_command = 1;

            if(empty_command == 0){
                if(quit != 1){
                    pid = fork(); //create process
                    if(pid == -1) {
                        printf("No se pudo crear el PID");
                        return 0;
                    }    
                    else if(pid == 0){
                        if(input_ == 1 | output_ == 1) {    
                            char *args_new[MAX_LINE/2 + 1];
                            for(int i=0; !(strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0); i++)
                                args_new[i] = args[i]; 
                            state = execvp(args_new[0], args_new);
                            printf("Error en execvp()");
                            exit(-1);
                        }
                        else if(Pipe_start == 1) {
                            pid_t pid_2;
                            pipe(fd);
                            pid_2 = fork();
                            
                            if(pid_2 < 0) {    
                                printf("Fallo -1 .");
                                return 0;
                            }// create hijo
                            else if(pid_2 == 0) {
                                saved_stdout = dup(1);
                                dup2(fd[1], 1);     //Remplazar salida standar abierto para escritura
                                close(fd[0]);       //Cerramos
                                execvp(args1[0], args1);//ingresa arg1 que sera salida para el segundo
                                printf("Commando %s no encontrado \n", args1[0]);
                                exit(-1);
                            }//padre
                            else if(pid_2 > 0) {
                                wait(NULL);
                                saved_stdin = dup(0);
                                dup2(fd[0], 0); //reemplazar stdin 
                                close(fd[1]);   //cerramos para escritura
                                state2 = execvp(args2[0], args2); //entrada 
                                
                                if(state2 < 0 )
                                  printf("Commando %s no encontrado \n", args1[0]);
                                exit(-1);
                            }
                        } else {// no tenemos pipes
                            state = execvp(args[0], args);
                        }
                        
                        if(state < 0)
                         printf("Commando %s no encontrado \n", args[0]);
                        exit(0);
                    } else{ // proceso padre
                        if(EsperaPadreWait == 1) { 
                            wait(NULL);
                        }
                    }
                }
                else{
                    should_run  = 0;
                }
            }
        }
        //restaurar
        dup2(saved_stdout, 1);
        dup2(saved_stdin, 0);
    }  

return 0;
}
//tratamiento al comando ingresado separarlo 
void SplitCommand(char * command, char ** args) {
    Escribir(command);
    int it =0;                  //indice para recorrer args
    int command_len = strlen(command);     
    int arg_i = -1;                     
    int prev_command = 0;
    int history = 0;
    

    for(int i =0; i < command_len; i++) {                     
        switch(command[i]) {
            case ' ':
            case '\t': 
            if(arg_i != -1) {
                args[it] = &command[arg_i];
                it++;
            }
            command[i] = '\0';         
            arg_i = -1;   
            break;

            case '\n':
            if(arg_i != -1) {
                args[it] = &command[arg_i];
                it++;
            }
            command[i] = '\0'; 
            //args[it+1] = NULL;   
            break;

            default:                     

            if(arg_i == -1)
              arg_i = i;
            if(command[i] == '&') {
                EsperaPadreWait = 0;    //padre no invoca wait
                command[i] = '\0'; 
                i++;
                //args[it+1] = NULL; 
            }
            break;
                           

        }    
    }  
/*    
    for(int i=0; i< it; i++)
        printf("args[%d]:%s\n", i, args[i]);
*/
args[it] = NULL;
}

void Directorio(char ** args) {
    for(int i=0; args[i] != NULL; i++) {
        //printf(" %d %s \n",i, args[i]);
        if(strcmp(args[i], "<") == 0){
            input_ = 1;
            if(args[i+1] != NULL)
                infile_index = i+1;
            else{
                printf("Comando %s no valido. ", args[0]);
            }
            
        }

        if(strcmp(args[i], ">") ==0){
            output_ = 1;
            if(args[i+1] != NULL)
                outfile_index = i+1;
            else{
                 printf("Comando %s no valido. ", args[0]);
            }
        }
    }
    //printf("Redirection I O %d %d \n", input_, output_);   
}
void Init() {
        input_ = 0;
        output_ = 0;
        EsperaPadreWait = 1;
        Pipe_start = 0;
        quit  = 0;
        empty_command = 0;
}

void VerificarPipes(char ** args) {
    for(int i=0; args[i] != NULL; i++) {
        if(strcmp(args[i], "|") == 0) {
            Pipe_start = 1;
            Pipe_Simbolo = i;
            //printf("pipe_simbol: %i ",i);
            if(args[i+1] == NULL) {
                printf("comando invalido despues de  |. \n");   
            }
        }
    }
}



void Escribir(char *str) {
     FILE * fptr;
     fptr = fopen("history.txt", "a"); 
     for (int i = 0; str[i] != '\0'; i++) {
         fputc(str[i], fptr);
    }
    //fputc('\n', fptr);
    fclose(fptr);
}
