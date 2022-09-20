#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_LINE 20

int main (int argc, char *argv[]) {

  if (argc < 4){
    perror("Insufficient number of arguments!");
    exit(1);
  }

  char* host_addr = argv[1];
  int port = atoi(argv[2]);
  int num = atoi(argv[3]);
  int read_size;

  char buf[MAX_LINE];
  char response[MAX_LINE];
  char hello[] = "HELLO";

  char strReturn[30] = { 0 };  


  /* Open a socket */
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0){
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

  /* Connect to the server */
  if(connect(s, (struct sockaddr *)&sin, sizeof(sin))<0){
    perror("simplex-talk: connect");
    close(s);
    exit(1);
  }

  sprintf(buf, "%s %d", hello, num);
  int len = strlen(buf)+1;
  send(s, buf, len, 0);  

  read_size = recv(s, response, MAX_LINE, 0);

  if(read_size==0){
    perror("Did not receive anything from the server!");
    exit(1);
  }

  sscanf(response, "%s %d", hello, &num);
  fputs(response, stdout);
  fputs("\n", stdout);  
  fflush(stdout);  

  memset(response, '\0', sizeof(response));

  num+=1;
  sprintf(strReturn, "%s %d", hello, num);
  len = strlen(strReturn)+1;
  send(s, strReturn, len, 0); 

  memset(strReturn, '\0', sizeof(strReturn));
 
  close(s);  

  return 0;
}
