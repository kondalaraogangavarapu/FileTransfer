/*
 * tcpclient.c - A simple TCP client a file from FTP server
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>

#define BUFSIZE 1024
#define REQUEST_SIZE 1024
#define CLIENT_REQUEST_POINTER 1
#define IS_FILE_EXIST '1'
#define FILE_DOWNLOAD '2'
#define JUST_BEFORE_FILE_EXTENTION '.'
#define DOWNLOADED "*Yes.!"
#define NEG_ACK "no"
#define TERMINATE "0\n"


void display_menu(void);
/*
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n,i,k,dot_reminder=0;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE],client_request[REQUEST_SIZE],filename[50],extention_copy[10],file_ack[3];
    bool do_again=false,dot_found=false;
    FILE *output_fd;

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
        /* connect: create a connection with the server */
        if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
                error("ERROR connecting");
                display_menu();
        /* get message line from the user */
                printf("Please enter: ");
                bzero(buf, BUFSIZE);
                bzero(client_request, REQUEST_SIZE);
                fgets(buf,BUFSIZE, stdin);
                strcpy(client_request,buf);
                if(strcmp(buf,TERMINATE)==0){
                        printf("Client Terminated\n");
                        return 0;
                }

/* send the message line to the server */
        n = write(sockfd, buf, strlen(buf));
        if (n < 0)
                error("ERROR writing to socket");

/* print the server's reply */
        if(client_request[CLIENT_REQUEST_POINTER]==IS_FILE_EXIST){
                bzero(buf, BUFSIZE);
                n = read(sockfd, buf, BUFSIZE);
                if (n < 0)
                        error("ERROR reading from socket");
                printf("Reply from server: %s\n", buf);
        }
        else if(client_request[CLIENT_REQUEST_POINTER]==FILE_DOWNLOAD){
                bzero(buf, BUFSIZE);
                n = read(sockfd, buf, BUFSIZE);
                if (n < 0)
                        error("ERROR reading from socket");
                i=2;
                k=0;
                while(i!=0){
                        file_ack[k]=buf[n-i];
                        i--;
                        k++;
                }
                file_ack[k]='\0';
                if(strcmp(file_ack,NEG_ACK)==0){
                        printf("Reply from server: %s\nFile not found to download\n",buf);
                }
                else{
                        i=0;
                        k=0;
                        while(buf[i]!='\0'){
                                if(buf[i]==JUST_BEFORE_FILE_EXTENTION||dot_found==true){
                                        if(dot_found==false){
                                                dot_found=true;
                                                dot_reminder=i;
                                        }
                                        extention_copy[k]=buf[i];
                                        k++;
                                }
                                i++;
                        }
                        extention_copy[k]='\0';
                        buf[dot_reminder]='\0';
                        strcat(buf,"2");
                        strcat(buf,extention_copy);
                        strcpy(filename,buf);
                        printf("file: %s ready to download\n",buf);
                        output_fd=fopen(filename,"w");
                        if(output_fd==NULL){
                                printf("file can not be downloaded\n");
                                return 0;
                        }
                        bzero(buf,BUFSIZE);
                        strcpy(buf,"OK");
                        n=write(sockfd,buf,strlen(buf));
                        if (n < 0)
                                error("ERROR writing to socket");
                        do{
                                bzero(buf,BUFSIZE);
                                n=read(sockfd,buf,BUFSIZE);
                                if(n < 0)
                                        error("ERROR reading from socket");
                                if(strcmp(buf,DOWNLOADED)!=0){
                                        fputs(buf,output_fd);
                                        bzero(buf,BUFSIZE);
                                        strcpy(buf,"OK");
                                        n=write(sockfd,buf,strlen(buf));
                                        if(n<0)
                                                error("ERROR writing to socket");
                                }
                        }while(strcmp(buf,DOWNLOADED)!=0);
                        printf("File downloaded\n");
                        fclose(output_fd);
                }
        }
        else{
                bzero(buf,BUFSIZE);
                n=read(sockfd,buf,BUFSIZE);
                if(n<0)
                        error("Error reading from socket");
                printf("%s\n",buf);
        }
        close(sockfd);
    return 0;
}

void display_menu(){
    printf("\n\n");
    printf("====Please enter the menu====\n");
    printf("1.Request a file name (Usage: *1:filename)\n");
    printf("2.Download a file (Usage: *2:filename)\n");
    printf("0.Terminate (Usage: 0)\n");
    printf("Note: File name cannot be more than 50 bytes here\n");
}
