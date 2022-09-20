#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <fcntl.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 150
#define MAX_LINE 50
#define NUM_REQUESTS 150

void handle_first_shake(int *arg) {

  int new_s = *arg;
  char buf[MAX_LINE];
  char hello[30];
  char strReturn[MAX_LINE];
  int num;
  socklen_t len;

  if((len = recv(new_s, buf, sizeof(buf), 0)) > 0){

    fputs(buf, stdout);
    fputs("\n", stdout);
    fflush(stdout);
    
    sscanf(buf, "%s %d", hello, &num);
    num=num+1;
    sprintf(strReturn, "%s %d", hello, num);

    send(new_s, strReturn, len, 0);

    memset(buf, '\0', sizeof(buf));
    memset(hello, '\0', sizeof(hello));
    memset(strReturn, '\0', sizeof(strReturn));
  }    
    
  if (len < 0){
    perror("Did not receive any messages");
    exit(1);
  }
}

void handle_second_shake(int *arg) {

  int new_s = *arg;
  char buf[MAX_LINE];
  socklen_t len;

  if((len = recv(new_s, buf, sizeof(buf), 0) > 0)){

    fputs(buf, stdout);
    fputs("\n", stdout);
    fflush(stdout);
    memset(buf, '\0', sizeof(buf));
  }    
    
  if (len < 0){
    perror("Did not receive any messages");
    exit(1);
  }

  free(arg);
}


int main(int argc, char *argv[]) {

  int clientSockets[NUM_REQUESTS];
  int clientStates[NUM_REQUESTS];

  if (argc >= 3){
    perror("Too many arguments!");
    exit(1);
  }

  char* host_addr = "127.0.0.1";
  int port = atoi(argv[1]);  
  
  /*setup passive open*/
  int listener;
  if( (listener = socket(PF_INET, SOCK_STREAM, 0) ) <0) {
    perror("simplex-talk: socket");
    exit(1);
  }

  /* Config the server address */
  struct sockaddr_in sin;
  sin.sin_family = AF_INET; 
  sin.sin_addr.s_addr = inet_addr(host_addr);
  sin.sin_port = htons(port);
  // Set all bits of the padding field to 0
  memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

  /* Bind the socket to the address */
  if((bind(listener, (struct sockaddr*)&sin, sizeof(sin)))<0) {
    perror("simplex-talk: bind");
    exit(1);
  }

  // connections can be pending if many concurrent client requests
  listen(listener, MAX_PENDING);  

  /* wait for connection, then receive and print text */
  int i=0;
  int new_s;
  int socketDesc;
  int maxSocketDesc;
  socklen_t len = sizeof(sin);

  fd_set readfds; // need to be initialized everytime we call p/select
  fd_set master;
  FD_SET(listener, &master);
    
  for(i = 0; i < NUM_REQUESTS; i++){
    clientSockets[i] = 0;
  }

  //Set the states array to 0, will track which stage each fd is at
  for(i = 0; i < NUM_REQUESTS; i++){
    clientStates[i] = 0;
  }

  maxSocketDesc = listener;

  while(1) {

    FD_ZERO(&readfds);     //Clearing the set at the start of each iteration
    readfds = master;

    for(i = 0; i< NUM_REQUESTS; i++){
      socketDesc = clientSockets[i];
      if(socketDesc > maxSocketDesc){
        maxSocketDesc = socketDesc;      //Replacing the current max file descriptor with the new descriptor if its larger
      }
    }
		
    int selection = select(maxSocketDesc+1, &readfds, NULL, NULL, NULL);

    if (selection < 0){
      perror("Select error!");
      exit(1);
    }

    if(FD_ISSET(listener, &readfds)){
      //new incoming connection
      if((new_s = accept(listener, (struct sockaddr *)&sin, &len)) <0){
        perror("simplex-talk: accept");
        exit(1);
      } else {
        fcntl(new_s, F_SETFL, O_NONBLOCK);  //Setting server socket to NON_BLOCKING
        FD_SET(new_s, &master);      //Adding socket to the set of file descriptors
      }

      for(i = 0; i < NUM_REQUESTS; i++){

        if(clientSockets[i] == 0){
          clientSockets[i] = new_s;
          clientStates[i] += 1;     //Update client states array to track the next stage, value should now be 1
          break;
        }
      }

    } else {   //Receiving from another socket

      for(i = 0; i < NUM_REQUESTS; i++){

        socketDesc = clientSockets[i];
        int* allocatedSock =  malloc(sizeof(int));
        * allocatedSock = socketDesc;

        if(FD_ISSET(socketDesc, &readfds)){

          if(clientStates[i] == 1){
            handle_first_shake(allocatedSock);      //Perform the first handshake
            clientStates[i] += 1;       //Update client states array to track the next stage, value should now be 2

          } else if (clientStates[i] == 2){           
            handle_second_shake(allocatedSock);
            
            close(socketDesc);
            FD_CLR(socketDesc, &master);     //Removes the file descriptor from the set

            clientSockets[i] = -1;  //Sets the array to -1
            clientStates[i] += 1; 
          } 
        }
      }
    }
  }

  return 0;
}