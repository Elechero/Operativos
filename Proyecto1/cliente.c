#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include "chat.h"
#include <time.h>


#define ALTO 5 // Alto de la ventana 2
#define LINES_MIN 10 // Alto mínimo que debe tener el terminal
#define COLS_MIN 25 // Ancho mínimo que debe tener el terminal
#define TECLA_RETURN 0xD
#define TAM 2048 // Tamaño de buffer

WINDOW *ventana1, *ventana2;

/* Mueve el cursor al punto de inserción actual de la ventana 2. */
// Funcion para concatenar.
char* concat(char *s1, char *s2){
	char *result;
    result = (char*)malloc(strlen(s1)+strlen(s2)+1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

//Crear pipe de wscritura.
char* crearPipe_w(char* usuario){
	char* dir_w = "/tmp/w_";
    char* escritura = concat(dir_w,usuario);
    if(unlink(escritura)>=0){
		printf("Unlink hechor\n");
	}
    if(mkfifo(escritura, 0666)<0){
    	printf("No se creo el pipe");
    }
    return escritura;
}

//Crear pipe de lectura.
char* crearPipe_r(char* usuario){

	char* dir_l = "/tmp/r_";
	char* lectura = concat(dir_l,usuario);
	if(unlink(lectura)>=0){
		printf("Unlink hechor\n");
	}
    if(mkfifo(lectura, 0666)<0){
    	printf("No se creo el pipe");
    }
    return lectura;
}

//Conectarse al servidor (Enviar nombre del usuario por el pipe com)
int conectarServidor(char * usuario,char * pipe_serv){
	int fd;
	if((fd = open(pipe_serv, O_WRONLY |O_NONBLOCK))<0){
		fprintf(stderr, "Error al abrir pipe de comunicacion.\n");
		return 0;
	}	
    if(write(fd, usuario, strlen(usuario)+1)<=0){
    	fprintf(stderr, "Error al escribir en pipe de comunicacion. Descriptor %d\n",fd);
		return 0;
    }
    close(fd);
    return 1;
}

int main(int argc, char *argv[]){					// argc lo asigna solo, es el numero de argumentos que se pasan por terminal.
	int fd_w,fd_r,aux;									// Filedescriptors de los dos pipes que se crean.
	size_t tmp_part=strlen("/tmp/");				
	size_t nam_given_size;							// Tamanio del nombre proporcionado
	size_t dflt_usr_len=strlen(getenv("USER"));		// Tamanio del nombre de usuario por defecto
	char * usuario;									// usuario y pipe_com son apuntadores.
	char * pipe_com;
	char* dir_wr = "/tmp/w_";
    char* pwrite;
    char* pread;
    char* dir_re = "/tmp/r_";
	char com_buff[TAM];

	int comm_success,fdread_aux,fdwrite_aux,leido;
	fd_set readfds,writefds,comm,comm_cpy,readfds_cpy,writefds_cpy;
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	FD_ZERO(&comm);
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);


	if(argc==1){									// Si solo se proporciona un argumento, entonces el pipe y el usuario se toman por defecto
		//Acciones por defecto
		usuario=(char *)malloc(dflt_usr_len+1);		// Pedimos espacio en memoria para el nombre del usuario por defecto y el caracter nulo
		usuario=getenv("USER");						// Asignamos el nombre de usuario
		pipe_com="/tmp/servidor1210761-1210796";					// asignamos el nombre del pipe por defecto segun lo especificado.

	}else{
		if(argc==2){
			if(argv[1][0]=='-'){					// Si hay 2 argumentos y el primer argumento empieza con "-" dar error
				//Mensaje de error
				fprintf(stderr, "Uso esperado: %s [-p pipe] [usuario]\n", argv[0]);
				return 1;

			}else{									// En caso contrario, asignar el nombre del usuario.
				//Asignamos al usuario el nombre proporcionado
				usuario = (char *)malloc(strlen(argv[1])+1);
				memcpy(usuario,argv[1],strlen(argv[1]));
				usuario[strlen(usuario)] = '\0';

				//Pipe predeterminado
				pipe_com="/tmp/servidor1210761-1210796";
			}
		}else{
			if(argc==3)
				if(strcmp(argv[1],"-p")==0){
					//Colocamos el nombre del pipe de comunicacion dado por el usuario
					nam_given_size = strlen(argv[2]);
					pipe_com = (char *)malloc(tmp_part+nam_given_size+1);
					memcpy(pipe_com,"/tmp/",tmp_part);
					memcpy(pipe_com + tmp_part,argv[2],nam_given_size+1); /////////////////
					pipe_com[strlen(pipe_com)] = '\0';
					printf("%s\n",pipe_com);

					//Usuario predeterminado
					usuario=(char *)malloc(dflt_usr_len+1);
					usuario=getenv("USER");

				}else{
					//Mensaje de error
					fprintf(stderr, "Uso esperado: %s [-p pipe] [usuario]\n", argv[0]);
					return 1;
				}
			else{
				if(argc!=4){
					//Mensaje de error
					fprintf(stderr, "Uso esperado: %s [-p pipe] [usuario]\n", argv[0]);
					return 1;
				}else{
					if(strcmp(argv[1],"-p")==0){
						//Colocamos el nombre del pipe de comunicacion dado por el usuario
						nam_given_size = strlen(argv[2]);
						pipe_com = (char *)malloc(tmp_part+nam_given_size+1);
						memcpy(pipe_com,"/tmp/",tmp_part);
						memcpy(pipe_com + tmp_part,argv[2],nam_given_size+1);
						pipe_com[strlen(pipe_com)-1] = '\0';

						//Asignamos al usuario el nombre proporcionado
						usuario = (char *)malloc(strlen(argv[3])+1);
						memcpy(usuario,argv[3],strlen(argv[3]));
					}else{
						//Mensaje de error
						fprintf(stderr, "Uso esperado: %s [-p pipe] [usuario]\n", argv[0]);
						return 1;	
					}
				}
			}
		}
	}
	printf("Se ha creado %s\n",pwrite = crearPipe_w(usuario));
	printf("Se ha creado %s\n",pread = crearPipe_r(usuario));
	if(conectarServidor(usuario,pipe_com)){
	}else{
		fprintf(stderr,"Error al conectarServidor\n");
		return -1;
	}
	//Obtenemos la direccion de los pipes

	// los abrimos y los metemos en cada file descriptor

	if((fd_w = open(pwrite, O_WRONLY|O_NONBLOCK))<0){
		printf("No se abrio %s\n",pwrite);
		return 1;
	}
	fd_r = open(pread,O_RDONLY|O_NONBLOCK);
	FD_SET(fd_r,&comm);


	/////////////////////////////////////////////////////////////////////
	    
	    initscr(); // Inicializar la biblioteca ncurses
    
    if (LINES < LINES_MIN || COLS < COLS_MIN) {
        endwin(); // Restaurar la operación del terminal a modo normal
        printf("El terminal es muy pequeño para correr este programa.\n");
        exit(0);
    }

    // Opciones de la biblioteca ncurses
    cbreak();
    nonl();

    int alto1 = LINES - ALTO; // Alto de la ventana 1
    ventana1 = newwin(alto1, 0, 0, 0); // Crear la ventana 1
    ventana2 = newwin(ALTO, 0, alto1, 0); // Crear la ventana 2
    scrollok(ventana1, TRUE); //Activar el corrimiento automático en la ventana 1
    scrollok(ventana2, TRUE);
    limpiarVentana2(); // Dibujar la línea horizontal

    while(1) {
        char buffer[TAM];
        wgetnstr(ventana2, buffer, TAM); // Leer una línea de la entrada
        aux = write(fd_w,buffer,strlen(buffer)+1);
		wprintw(ventana1, concat(usuario,"'Escribiste al pipe: %d lineas' \n"), fd_w);

        if (strcmp(buffer, "-salir") == 0) {
            break;
        }

		comm_cpy = comm;
		comm_success = select(fd_r+1,&comm_cpy,NULL,NULL, &tv);
		if(comm_success == -1){
			perror("Error de comunicacion");
		
		}else if(comm_success){
			leido = read(fd_r,com_buff,TAM);
			com_buff[leido]='\0';
			wprintw(ventana1, concat(usuario,": %s\n"), com_buff);
		}

        //Escribir al servidor

        //if(strlen(buffer)>0){


        //}

        //Escribir a la pantalla lo que acaba de escribir.
        wprintw(ventana1, concat(usuario,": %s\n"), buffer);

        //Refrescar la pantalla.
        wrefresh(ventana1);
        limpiarVentana2();
    }

    endwin(); // Restaurar la operación del terminal a modo normal
    exit(0);
}
void enfocarVentana2() {
    int y, x;
    getyx(ventana2, y, x);
    wmove(ventana2, y, x);
    wrefresh(ventana2);
}

/* Borra el contenido de la ventana 2 y ubica el cursor en la esquina 
 * superior izquierda de esta ventana.
 */
void limpiarVentana2() {
    wclear(ventana2);
    mvwhline(ventana2, 0, 0, 0, 20); // Dibujar la línea horizontal
    wmove(ventana2, 1, 0);
    wrefresh(ventana2);
}


