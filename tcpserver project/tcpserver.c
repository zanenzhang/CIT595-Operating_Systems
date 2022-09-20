#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 10
#define MAX_LINE 30

int main(int argc, char *argv[]) {

  if (argc >= 3){
    perror("Too many arguments!");
    exit(1);
  }

  char* host_addr = "127.0.0.1";
  int port = atoi(argv[1]);  
  
  char hello[15];
  char strReturn[MAX_LINE];
  int num;

  /*setup passive open*/
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0) {
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
  if((bind(s, (struct sockaddr*)&sin, sizeof(sin)))<0) {
    perror("simplex-talk: bind");
    exit(1);
  }

  // connections can be pending if many concurrent client requests
  listen(s, MAX_PENDING);  

  /* wait for connection, then receive and print text */
  int new_s;
  socklen_t len = sizeof(sin);
  char buf[MAX_LINE];
  while(1) {
    if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
      perror("simplex-talk: accept");
      exit(1);
    }
    while((len = recv(new_s, buf, sizeof(buf), 0))){

        fputs(buf, stdout);
        fputs("\n", stdout);
        fflush(stdout);
        
        sscanf(buf, "%s %d", hello, &num);
        num=num+1;
        sprintf(strReturn, "%s %d", hello, num);
        
        memset(buf, '\0', sizeof(buf));

        send(new_s, strReturn, len, 0);
    }    
    
    if (len < 0){
      perror("Did not receive any messages");
      exit(1);
    }
  }

  return 0;
}
