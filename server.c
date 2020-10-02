#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

void serviceClient(int client){
	int bufferSize=256, readSize, fd, ack;
	char ch, command[bufferSize];
	while(1){
		printf("Waiting to read command from client...\n");
		memset(command,0,strlen(command));					// reset command string to empty
		if(readSize=read(client, command, bufferSize) < 0){	// read command from client
			fprintf(stderr, "read() error\n");
			exit(1);
		}
		
		printf("Command received: %s\n", command);
		
		if(strncmp(command,"get",3)==0){
			char *fileName=command;							// pointer to filename in command
			fileName+=4;
			fd = open(fileName, O_RDONLY);
			write(client, &fd, sizeof(fd));					// send acknowledge of file descriptor
			if (fd == -1){
				printf("file could not be opened, please check it exists and has proper permissions\n");
			}else{
				printf("Sending %s to the client...\n", fileName);
		    	while(read(fd, &ch, 1)>0){					// sending file
		    		write(client, &ch, 1);
		    	}
		    	ch=4;										// ASCII code 4 to signal end-of-file to the client
				write(client, &ch, 1);
		    	close(fd);
		    	printf("%s successfully sent to the client.\n", fileName);
		    }
		}
		else if(strncmp(command,"put",3)==0){
			char *fileName=command;
			fileName+=4;

			read(client, &ack, sizeof(ack));
			if(ack==-1){
				write(1,"File does not exists on client machine\n",strlen("File does not exists on client machine\n"));
			}else{
				fd=open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 0700);
				if (fd == -1){
					write(1,"file could not be created, please check directory\n",strlen("file could not be created, please check directory\n"));
				}else{
					printf("Saving %s ...\n", fileName);
					while(read(client, &ch, 1)>0){			// receive file
			    		if(ch==4)							// ASCII code 4 to end reading file from client
			    			break;
			    		write(fd, &ch, 1);
			    	}
			    	close(fd);
			    	printf("%s saved successfully.\n", fileName);
			    }
		    }
		}
		else if(strcmp(command,"quit")==0){
			printf("Client quit");
			close(client);
			exit(0);
		}
		else{
			printf("Wrong command. Please send get, put or quit command.\n");
		}
		printf("------------------------------------------\n");
	}
    exit(0); 
}

int main(int argc, char *argv[]){  
	int sd, portNumber, client;
	struct sockaddr_in servAdd;     	//server socket address
	
	if(argc != 2){
		printf("Call model: %s <Port Number>\n", argv[0]);
		exit(0);
	}

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){ 		//create socket 
		fprintf(stderr, "Could not create a socket\n");
		exit(1);
	}
	servAdd.sin_family = AF_INET;							//intialize server address structure
	servAdd.sin_addr.s_addr = htonl(INADDR_ANY);			// converts the unsigned integer hostlong from host byte order to network byte order
	sscanf(argv[1], "%d", &portNumber);
	servAdd.sin_port = htons((uint16_t)portNumber);			// converts the unsigned short integer hostshort from host byte order to network byte order
	
	bind(sd, (struct sockaddr *) &servAdd, sizeof(servAdd));	//bind socket address to the socket
	listen(sd, 6);											//Make the socket listening
	
	while(1){
		printf("Waiting for client to connect...\n");	
		client = accept(sd,(struct sockaddr*)NULL,NULL);
		printf("Client connected.\n");
		
		if(!fork()){
		  serviceClient(client);
		}
	}
	return 0;
}