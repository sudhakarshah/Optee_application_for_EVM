/*
 * server.c
 * Version 20161003
 * Written by Harry Wong (RedAndBlueEraser)
 */

#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>
/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* To the the UUID (found the the TA's h-file(s)) */
#include <serverForEVM_ta.h>

#define BACKLOG 10
#define MAX 200

pthread_mutex_t lock1;

typedef struct pthread_arg_t {
    int new_socket_fd;
    struct sockaddr_in client_address;
    /* TODO: Put arguments passed to threads here. See lines 116 and 139. */
} pthread_arg_t;

uint32_t total = 0;
uint32_t curr = 0;
/* Thread routine to serve connection to client. */
void *pthread_routine(void *arg);

void *pthread_routine(void *arg) 
{

  /*  CREATING TEE SESSION RELATED OBJECTS */
  pthread_mutex_lock(&lock1);

  TEEC_Result res;
  TEEC_Context ctx;
  TEEC_Session sess;
  TEEC_Operation op = { 0 };
  TEEC_UUID uuid = TA_RANDOM_UUID;
  uint32_t err_origin;
  total = total + 1;
  curr = curr + 1;
  printf("Total: %d Curr: %d\n", total, curr);

  uint32_t first[1] = { 0 };
  uint32_t second[1] = { 0 };
  uint32_t third[1] = { 0 };

  res = TEEC_InitializeContext(NULL, &ctx);

  if (res != TEEC_SUCCESS)
    errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

  res = TEEC_OpenSession(&ctx, &sess, &uuid,
      TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);

  if (res != TEEC_SUCCESS)
    errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x",
        res, err_origin);

  pthread_mutex_unlock(&lock1);

  memset(&op, 0, sizeof(op));
  op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT,
      TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE);

  pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
  int sockfd = pthread_arg->new_socket_fd;
  struct sockaddr_in client_address = pthread_arg->client_address;

  free(arg);

 // printf("client thread\n");

  int n; 
  // infinite loop for chat 
  for (;;) { 
    char buff[MAX]; 
    bzero(buff, MAX); 

    // read the message from client and copy it in buffer 
    ssize_t cc =    read(sockfd, buff, sizeof(buff)); 

    if (cc <=0) {

      printf("empty buffer with value %d\n", cc);
      break;
    }

    // print buffer which contains the client contents 
    //printf("From client: %c\n", buff[1]);
    int i = 4;
    int a = 0;
    while (buff[i] != ' ')
    {	
      int temp = buff[i] - '0';
      a = a*10 + temp;
      i++;	

    }
    printf("a is: %d\n", a);  
    i++;
    int b = 0;

    while (buff[i] != ' ')
    {	
      int temp = buff[i] - '0';
      b = b*10 + temp;
      i++;
    }	

    first[0] = a;
    second[0] = b;
    op.params[0].tmpref.buffer = third;
    op.params[0].tmpref.size = sizeof(third);
    op.params[1].tmpref.buffer = first;
    op.params[1].tmpref.size = sizeof(first);
    op.params[2].tmpref.buffer = second;
    op.params[2].tmpref.size = sizeof(second);

    if (strncmp(buff, "ADD", 3) == 0) {
      res = TEEC_InvokeCommand(&sess, TA_ADD, 
                              &op, &err_origin);
    
    }
    else if (strncmp(buff, "SUB", 3) == 0) {
      res = TEEC_InvokeCommand(&sess, TA_SUBTRACT, 
                              &op, &err_origin);
    }
    else if (strncmp(buff, "MUL", 3) == 0) {
      res = TEEC_InvokeCommand(&sess, TA_MULTIPLY, 
                              &op, &err_origin);
    }
    else {
      printf("Incorrect operation send to the OPTEE server");

    }
    
    if (res != TEEC_SUCCESS)
      errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
          res, err_origin);


    int ans = third [0];
    //printf(" third is %d\n", third[0]);

    bzero(buff, MAX); 
    snprintf(buff, sizeof(buff), "%d\n", ans);


    //printf("ans is: %d\n", ans);  
    write(sockfd, buff, sizeof(buff)); 


    bzero(buff, MAX); 
  } 


  close(sockfd);

	pthread_mutex_lock(&lock1);
	TEEC_CloseSession(&sess);

  curr = curr - 1;
  TEEC_FinalizeContext(&ctx);
	pthread_mutex_unlock(&lock1);
  
  return NULL;

}

/* Signal handler to handle SIGTERM and SIGINT signals. */
void signal_handler(int signal_number);

int main(int argc, char *argv[]) {
    int port, socket_fd, new_socket_fd;
    struct sockaddr_in address;
    pthread_attr_t pthread_attr;
    pthread_arg_t *pthread_arg;
    pthread_t pthread;
    socklen_t client_address_len;

    /* Get port from command line arguments or stdin. */
    port = 12345; 

    /* Initialise IPv4 address. */
    memset(&address, 0, sizeof address);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    /* Create TCP socket. */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    /* Bind address to socket. */
    if (bind(socket_fd, (struct sockaddr *)&address, sizeof address) == -1) {
        perror("bind");
        exit(1);
    }

    /* Listen on socket. */
    if (listen(socket_fd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    /* Assign signal handlers to signals. */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("signal");
        exit(1);
    }

    /* Initialise pthread attribute to create detached threads. */
    if (pthread_attr_init(&pthread_attr) != 0) {
        perror("pthread_attr_init");
        exit(1);
    }
    if (pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(1);
    }

    while (1) {
        /* Create pthread argument for each connection to client. */
        /* TODO: malloc'ing before accepting a connection causes only one small
         * memory when the program exits. It can be safely ignored.
         */
        pthread_arg = (pthread_arg_t *)malloc(sizeof *pthread_arg);
        if (!pthread_arg) {
            perror("malloc");
            continue;
        }
	
        /* Accept connection to client. */
        client_address_len = sizeof pthread_arg->client_address;
        new_socket_fd = accept(socket_fd, (struct sockaddr *)&pthread_arg->client_address, &client_address_len);
        if (new_socket_fd == -1) {
            perror("accept");
            free(pthread_arg);
            continue;
        }

        //printf("client port %d\n", new_socket_fd);
        /* Initialise pthread argument. */
        pthread_arg->new_socket_fd = new_socket_fd;
        /* TODO: Initialise arguments passed to threads here. See lines 22 and
         * 139.
         */

        /* Create thread to serve connection to client. */
        if (pthread_create(&pthread, &pthread_attr, pthread_routine, (void *)pthread_arg) != 0) {
            perror("pthread_create");
            free(pthread_arg);
            continue;
        }
    }

    /* close(socket_fd);
     * TODO: If you really want to close the socket, you would do it in
     * signal_handler(), meaning socket_fd would need to be a global variable.
     */
    return 0;
}

void signal_handler(int signal_number) {
    /* TODO: Put exit cleanup code here. */
    exit(0);
}
