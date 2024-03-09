#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/mman.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

struct log
{
  int client;
  int id;
  char user[100];
  char mesaj[1000];
};


int main(int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  /*char msg[100];*/		// mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if(argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);
  
  /* ne conectam la server */
  if(connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
  {
      perror ("[client]Eroare la connect().\n");
      return errno;
  }

   printf ("Bine ati venit in Offline messenger, introduceti o comanda:");
   fflush(stdout);  
   struct log mesaj;
   mesaj.id = 0;
   mesaj.mesaj[0] = '\0';
   mesaj.mesaj[0] = '\0';

   struct log* msg = mmap(NULL, sizeof(struct log), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
   msg->mesaj[0] = '\0';
   msg->user[0] = '\0';
   msg->id = 0;

    int pid;
    pid = fork();
    if(pid == 0)
    {
      while(1)
      {
        int bytes = read(sd, &mesaj, sizeof(struct log)) ;
        if (bytes < 0)
        {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }
        else if (bytes == 0)
        {
          break;
        }
        msg->id = mesaj.id;
        strcpy(msg->user, mesaj.user);
        
        /* afisam mesajul primit */
          printf("%s\n", mesaj.mesaj);
          /*if(strcmp(mesaj.mesaj, "Ai iesit din Offline Messenger!") == 0);*/
      }
      exit(0);
    }
  int stare = 1;
  while(stare)
  {
    bzero(msg->mesaj, 1000);
    fflush(stdout);
    read(0, msg->mesaj, 1000);
    //printf("%s",msg);
    
    /*strcpy(mesaj.mesaj, msg.mesaj);*/
    /* trimiterea mesajului la server */
    if (write(sd, &*msg, sizeof(struct log)) <= 0)
    {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
    }

    if(strcmp(msg->mesaj, "quit\n") == 0)
    {
          break;
    }
    /* citirea raspunsului dat de server 
      (apel blocant pina cind serverul raspunde) */
    
  }
  close (sd);
}