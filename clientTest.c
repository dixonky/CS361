/*******************************************************************************
* ** Author: 
* ** Date: 
* ** Descriptions: Client used to test the server (white cane project)
* ** 	Arguments: 1 <SERVER_HOST>, 2 <SERVER_PORT>, 3 <COMMAND>, 4 <DATA_PORT>
* **    Example: flip3 30727 a 30728
* **	General Sources: Beej's guide and CS 341
* **	http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
* ****************************************************************************/
#include <stdio.h>    
#include <stdlib.h>   
#include <string.h>
#include <unistd.h> 
#include <errno.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 

#define BUFFER 500

//Prep Address Function
	//"Prep the socket address structures for subsequent use"-Beej
	//pass in the address and port number of the desired socket
struct addrinfo* prepAddress(char* node, char* port){             
	struct addrinfo hints;	
    struct addrinfo *address;
    int stat;
	memset(&hints, 0, sizeof hints);	//set up holders
	hints.ai_family = AF_INET;		//IPv4         
	hints.ai_socktype = SOCK_STREAM;  	//TCP
	if((stat = getaddrinfo(node, port, &hints, &address)) != 0){	//create and validate the socket address structure
		fprintf(stderr,"Error: client port number %s\n", gai_strerror(stat));	//https://linux.die.net/man/3/gai_strerror
		exit(1);
	}
	return address;
}


//Create Socket Function
	//pass in socket address struct
	//creates a socket and validates
int createSocket(struct addrinfo* address){           
	int sockfd;       //holds the socket file descriptor                                     
	if ((sockfd = socket(address->ai_family, address->ai_socktype, address->ai_protocol)) == -1){	//create the socket, save the returned desciptor, and validate the socket via address info of struct
		fprintf(stderr, "Error: client socket creation\n");
		exit(1);
	}
	return sockfd;
}


//Connect Socket Function
	//pass in socket file descriptor and socket address struct
	//connects the socket and validates
void connectSocket(int sockfd, struct addrinfo * address){
	int stat;
	if ((stat = connect(sockfd, address->ai_addr, address->ai_addrlen)) == -1){	//connect socket and validate via address info of struct
		fprintf(stderr, "Error: client socket connection\n");
		exit(1);
	}
}


// Returns hostname for the local computer 
    //https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
void checkHostName(int hostname) 
{ 
    if (hostname == -1) 
    { 
        perror("gethostname"); 
        exit(1); 
    } 
} 
  
// Returns host information corresponding to host name 
    //https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
void checkHostEntry(struct hostent * hostentry) 
{ 
    if (hostentry == NULL) 
    { 
        perror("gethostbyname"); 
        exit(1); 
    } 
} 
  
// Converts space-delimited IPv4 addresses to dotted-decimal format 
    //https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
void checkIPbuffer(char *IPbuffer) 
{ 
    if (NULL == IPbuffer) 
    { 
        perror("inet_ntoa"); 
        exit(1); 
    } 
} 


//Send command function
    //sends command to server
void sendCommand(int sockfd, char* command, char* portData, char* server){
    char input[BUFFER];      //holder for data sent to server
    memset(input, 0 ,sizeof(input));
    memcpy(input, portData, strlen(portData));
    printf("Data Port: %s\n", input);
    int errorCheck = 0; 
    errorCheck = send(sockfd, input, strlen(input), 0);		//send the data port first, validate
    if(errorCheck == -1){
		fprintf(stderr, "Error: Client Data Port\n");
		exit(1);
	}
    memset(input, 0 ,sizeof(input));
    sprintf(input, "%s", command);
    printf("Command: %s\n", input);
    errorCheck = send(sockfd, input, strlen(input), 0);		//send the command, validate
    if(errorCheck == -1){
		fprintf(stderr, "Error: Client Command\n");
		exit(1);
	}
    memset(input, 0 ,sizeof(input));
    //Source for code used to get local ip address: https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
    char hostbuffer[256]; 
    char *IPbuffer; 
    struct hostent *host_entry; 
    int hostname; 
    hostname = gethostname(hostbuffer, sizeof(hostbuffer)); 
    checkHostName(hostname); 
    host_entry = gethostbyname(hostbuffer); 
    checkHostEntry(host_entry); 
    IPbuffer = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); 
    memcpy(input, IPbuffer, strlen(IPbuffer));
    errorCheck = send(sockfd, input, strlen(input), 0);		//send the ip address, validate
    if(errorCheck == -1){
		fprintf(stderr, "Error: Client IP Address\n");
		exit(1);
	}
    memset(input, 0 ,sizeof(input));
    if(strcmp(command, "a") == 0){		//if the command was to initiate the alarm, the alarm code is needed
    	char alarmCode[10];
    	printf("Enter specific alarm code: ");	//get the code from the user
		scanf("%s", alarmCode);	
		memcpy(input, alarmCode, strlen(alarmCode));
		errorCheck = send(sockfd, input, strlen(input), 0);		//send the code to the server
		memset(input, 0 ,sizeof(input));
    }
    if(strcmp(command, "l") == 0){		//if the command was to find the cane, prepare to receive location data from the server
		char location[500];
		memset(location, 0, sizeof(location));
		int status;
		status = recv(sockfd, location, 500, 0);      //Not currently receiving the correct location data
		if (status == -1){
			fprintf(stderr, "Error: Location data not received properly\n");
			exit(1);
		}
		printf("Location Data Received: %s\n", location);
    }
    if(strcmp(command, "o") == 0){		//object command
    	char locationX[10];
    	printf("Coordinates are based on the user being at 0,0\n"); 	//user simulates the camera 'capturing' an object
    	printf("Compass directions follow axis plane (north is +y, east is +x)\n");
    	printf("1 unit = 1 meter\n");
    	printf("Enter x coordinate: ");	//get the coordinate from the user
		scanf("%s", locationX);	
		memcpy(input, locationX, strlen(locationX));
		errorCheck = send(sockfd, input, strlen(input), 0);		//send the code to the server
		memset(input, 0 ,sizeof(input));
		char locationY[10];
    	printf("Enter y coordinate: ");	//get the coordinate from the user, coordinates are based on current user location, with user being 0,0
		scanf("%s", locationY);	
		memcpy(input, locationY, strlen(locationY));
		errorCheck = send(sockfd, input, strlen(input), 0);		//send the code to the server
		memset(input, 0 ,sizeof(input));
		
		char velocity[10];
    	printf("Enter velocity (meters covered per second): ");	//get the code from the user
		scanf("%s", velocity);	
		memcpy(input, velocity, strlen(velocity));
		errorCheck = send(sockfd, input, strlen(input), 0);		//send the code to the server
		memset(input, 0 ,sizeof(input));
		
		char direction[10];
		printf("north = 0 degrees, east = 90, south = 180, west = 270\n");
    	printf("Enter movement direction: ");	//get the code from the user
		scanf("%s", direction);	
		memcpy(input, direction, strlen(direction));
		errorCheck = send(sockfd, input, strlen(input), 0);		//send the code to the server
		memset(input, 0 ,sizeof(input));
		
    	printf("The server has made a call to another sensor to receive user data\n");
    	printf("You are now that sensor...\n");
		char velocityU[10];
    	printf("Enter user velocity (meters covered per second): ");	//get the code from the user
		scanf("%s", velocityU);	
		memcpy(input, velocityU, strlen(velocityU));
		errorCheck = send(sockfd, input, strlen(input), 0);		//send the code to the server
		memset(input, 0 ,sizeof(input));
		
		char directionU[10];
		printf("north = 0 degrees, east = 90, south = 180, west = 270\n");
    	printf("Enter user movement direction: ");	//get the code from the user
		scanf("%s", directionU);	
		memcpy(input, directionU, strlen(directionU));
		errorCheck = send(sockfd, input, strlen(input), 0);		//send the code to the server
		memset(input, 0 ,sizeof(input));
    }
}


//Main Function
int main(int argc, char *argv[]){                             
	struct addrinfo* address = prepAddress(argv[1], argv[2]);  //cast pointer to address struct created from passed in arguments
	int sockfd = createSocket(address);   //create socket and save the socket descriptor
	connectSocket(sockfd, address);		//connect the created socket to the address struct
    sendCommand(sockfd, argv[3], argv[4], argv[1]);
	freeaddrinfo(address); 	//free linked list of address struct
}
