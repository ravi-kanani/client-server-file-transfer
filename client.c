#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	int server, portNo, fd, bufferSize=256, readSize, ack;
	struct sockaddr_in servAdd;     											// server socket address
	char ch, message[bufferSize];
	
	if(argc != 3){
		printf("Call model: %s <IP Address> <Port Number>\n", argv[0]);
		exit(0);
	}
	
	if((server = socket(AF_INET, SOCK_STREAM, 0)) < 0){							//create socket 
		fprintf(stderr, "Cannot create a socket\n");
		exit(1);
	}
	
	servAdd.sin_family = AF_INET;
	sscanf(argv[2], "%d", &portNo);
	servAdd.sin_port = htons((uint16_t)portNo);

	if(inet_pton(AF_INET, argv[1], &servAdd.sin_addr) < 0){						//converts an address from presentation to network format
		fprintf(stderr, " inet_pton() has failed\n");
		exit(2);
	}
	if(connect(server, (struct sockaddr *) &servAdd, sizeof(servAdd))<0){		//connect with server
		fprintf(stderr, "connect() has failed\n");
		exit(3);
	}
	printf("\nIt is a Text file transfer application.\nUsage: \n $ get <fileName> - to download a file\n $ put <filename> - to upload a file\n $ quit - to exit\n\n");
	while(1){
		write(1,"Enter command: ",strlen("Enter command: "));

		memset(message,0,strlen(message));		// clear the message string
		readSize=read(0, message, bufferSize);		// read command from user

		message[readSize-1] = '\0';				// remove new line character

		write(server, message, readSize);		// send command to server

		if(strncmp(message,"get",3)==0){		// if get command, execute this code
			char *fileName=message;
			fileName+=4;						// pointer to filename in command

			read(server, &ack, sizeof(ack));	// receive acknowledge if file successfully opened in the server
			if(ack==-1){
				write(1,"File does not exists on server\n",strlen("File does not exists on server\n"));
			}else{
				fd=open(fileName, O_WRONLY|O_CREAT|O_TRUNC, 0700);
				if (fd == -1){
					write(1,"file could not be created, please check directory\n",strlen("file could not be created, please check directory\n"));
				}else{
					write(1,"Downloading ",strlen("Downloading "));
					write(1,fileName,strlen(fileName));
					write(1," ...\n",strlen(" ...\n"));
			    	while(read(server, &ch, 1)>0){
			    		if(ch==4)				// ASCII code 4 to end reading file from server
			    			break;
			    		write(fd, &ch, 1);
			    	}
			    	close(fd);
			    	write(1,"file downloaded\n",strlen("file downloaded\n"));
			    }
		    }
		}
		else if(strncmp(message,"put",3)==0){	// if put command, execute this code
			char *fileName=message;
			fileName+=4;						// pointer to filename in command

			fd = open(fileName, O_RDONLY);
			write(server, &fd, sizeof(fd));		// send acknowledge of file descriptor
			if (fd == -1){
				write(1,"file could not be opened, please check it exists and has proper permissions\n",strlen("file could not be opened, please check it exists and has proper permissions\n"));
			}else{
				write(1,"Uploading ",strlen("Uploading "));
					write(1,fileName,strlen(fileName));
					write(1," ...\n",strlen(" ...\n"));
		    	while(read(fd, &ch, 1)>0){		// sending file
		    		write(server, &ch, 1);
		    	}
		    	ch=4;							// ASCII code 4 to signal end-of-file to the server
				write(server, &ch, 1);
		    	close(fd);
		    	write(1,"file uploaded\n",strlen("file uploaded\n"));
		    }
		}
		else if(strcmp(message, "quit")==0){
			write(1,"Bye Bye\n",strlen("Bye Bye\n"));
			close(server);
			exit(0);
		}
		else{
			write(1,"Only get, put or quit command allowed\n",strlen("Only get, put or quit command allowed\n"));
		}
		write(1,"--------------------------------------------\n",strlen("--------------------------------------------\n"));
	}	
}