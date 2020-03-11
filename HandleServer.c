/*******************************************************************************
* ** Author: 
* ** Date: 8/06/2019-
* ** Descriptions: HandleServer.c (Project CS 361)
* ** 	Server for Cane Network 
* **	General Sources: Beej's guide (heavily followed for server/socket setup) and CS 372, 341
* **	http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
* **	"The C Programming Language" Kernighan & Ritchie
* ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h>

#define BUFFER 500
#define MAXDATASIZE 100 //"max number of bytes we can get at once" -Beej 
#define BACKLOG 10     //"how many pending connections queue will hold" -Beej 


//Structure of a point on a coordinate plane
	//with x and y coordinate pair
struct Point 
{ 
   int x, y; 
};


//Prep Address Function (Beej)
	//"Prep the socket address structures for subsequent use"-Beej
	//pass in the port number and set up socket using address of the local host
struct addrinfo* prepAddress(char* port){             
	struct addrinfo hints;	
    struct addrinfo* address;
    int stat;
	memset(&hints, 0, sizeof hints);	//set up holders
	hints.ai_family = AF_INET;		//IPv4         
	hints.ai_socktype = SOCK_STREAM;  	//TCP
	hints.ai_flags = AI_PASSIVE;	//assign the address of the local host
	if((stat = getaddrinfo(NULL, port, &hints, &address)) != 0){	//create and validate the socket address structure
		fprintf(stderr,"Error: server port number %s\n", gai_strerror(stat));	//https://linux.die.net/man/3/gai_strerror
		exit(1);
	}
	return address;
}


//Prep IP Address Function (Beej)
	//"Prep the socket address structures for subsequent use"-Beej
	//pass in the address and port number of the desired socket
struct addrinfo* prepIPAddress(char* node, char* port){             
	struct addrinfo hints;	
    struct addrinfo* address;
    int stat;
	memset(&hints, 0, sizeof hints);	//set up holders
	hints.ai_family = AF_INET;		//IPv4         
	hints.ai_socktype = SOCK_STREAM;  	//TCP
	if((stat = getaddrinfo(node, port, &hints, &address)) != 0){	//create and validate the socket address structure
		fprintf(stderr,"Error: server port number %s\n", gai_strerror(stat));	//https://linux.die.net/man/3/gai_strerror
		exit(1);
	}
	return address;
}


//Create Socket Function (Beej)
	//pass in socket address struct
	//creates a socket and validates
int createSocket(struct addrinfo* address){           
	int sockfd;       //holds the socket file descriptor                                     
	if ((sockfd = socket(address->ai_family, address->ai_socktype, address->ai_protocol)) == -1){	//create the socket, save the returned desciptor, and validate the socket via address info of struct
		fprintf(stderr, "Error: server socket creation\n");
		exit(1);
	}
	return sockfd;
}


//Connect Socket Function (Beej)
	//pass in socket file descriptor and socket address struct
	//connects the socket and validates
void connectSocket(int sockfd, struct addrinfo* address){
	int stat;
	if ((stat = connect(sockfd, address->ai_addr, address->ai_addrlen)) == -1){	//connect socket and validate via address info of struct
		fprintf(stderr, "Error: server socket connection\n");
		exit(1);
	}
}


//Bind Socket Function (Beej)
	//pass in socket file descriptor and socket address struct
	//assigns the address to the socket
void bindSocket(int sockfd, struct addrinfo* address){                
	if (bind(sockfd, address->ai_addr, address->ai_addrlen) == -1) {	//assign address to the socket and validate
		close(sockfd);
		fprintf(stderr, "Error: server binding socket\n");
		exit(1);
	}
}


//Listen Socket Function (Beej)
	//pass in the socket file descriptor
	//sets up 10 incoming connections on the socket (10 chosen randomly from beej)
void listenSocket(int sockfd){         
	if(listen(sockfd, BACKLOG) == -1){	//listen to the socket and validate
		fprintf(stderr, "Error: server listen\n");
		close(sockfd);	//make sure to close the socket before exiting the program
		exit(1);
	}
}


//Accept Connection Function (Beej)
	//Accepts connection from client and listens for commands
void acceptConnection(int new_fd){	  
	char* pass = "pass"; 	//checks for client
	char* fail = "fail"; 
	char port[MAXDATASIZE];		//holders
    char command[MAXDATASIZE];
    char ipAddress[MAXDATASIZE];
	memset(port, 0, sizeof(port));  	//initialize holders
	memset(command, 0, sizeof(command)); 
	memset(ipAddress, 0, sizeof(ipAddress)); 
	
	recv(new_fd, port, sizeof(port)-1, 0);      //receive the data port address
	send(new_fd, pass, strlen(pass),0);		//send pass
	recv(new_fd, command, sizeof(command)-1, 0);	//receive command
	send(new_fd, pass, strlen(pass), 0);   //send pass
	recv(new_fd, ipAddress, sizeof(ipAddress)-1,0);		//receive client ip address
	
	printf("New connection: %s\n", ipAddress);
    printf("Data Port Number: %s\n",port);
    printf("Command Received: %s\n",command);

    if(strcmp(command, "a") == 0){    //client command to initiate alarm     
    	char alarmCode[BUFFER];		//holder for specific alarm code
    	memset(alarmCode, 0, sizeof(alarmCode));     
		send(new_fd, pass, strlen(pass),0);      //send pass
		recv(new_fd, alarmCode, sizeof(alarmCode)-1, 0); 	//get alarm code from client 
		printf("Alarm Code: %s\n", alarmCode);
		alarmSwitch(alarmCode[0]); //initiate alarm function
	}
	
	else if(strcmp(command, "l") == 0){    //client command to locate cane   
		//saveData();
		//select() 
		
		char dataLocation[22] = "Example Location Data";
		void* dataPtr = dataLocation;	//pointer used to send data
		send(new_fd, dataPtr, sizeof(dataLocation),0);
		  
    	char alarmCode[BUFFER];		//initiate the alarm for missing cane (code 5)
    	memset(alarmCode, 5, sizeof(alarmCode)); 	
		alarmSwitch(alarmCode[0]); 
	}
	
	else if(strcmp(command, "o") == 0){    //command for obstacle 
		char locationX[BUFFER];		//holder for location coordinates, coordinates are based on current user location, with user being 0,0
    	memset(locationX, 0, sizeof(locationX));     
		send(new_fd, pass, strlen(pass),0);      //send pass
		recv(new_fd, locationX, sizeof(locationX)-1, 0); 	//get location from client 
		
		char locationY[BUFFER];		//holder for location coordinates
    	memset(locationY, 0, sizeof(locationY));     
		send(new_fd, pass, strlen(pass),0);      //send pass
		recv(new_fd, locationY, sizeof(locationY)-1, 0); 	//get location from client 
		
		char velocity[BUFFER];		//holder for velocity coordinates
    	memset(velocity, 0, sizeof(velocity));     
		send(new_fd, pass, strlen(pass),0);      //send pass
		recv(new_fd, velocity, sizeof(velocity)-1, 0); 	//get velocity from client 
		
		char direction[BUFFER];		//holder for direction
    	memset(direction, 0, sizeof(direction));     
		send(new_fd, pass, strlen(pass),0);      //send pass
		recv(new_fd, direction, sizeof(direction)-1, 0); 	//get direction from client
		
		float locX, locY, vel, dir; //convert char to float
		locX = atof(locationX);
		locY = atof(locationY);
		vel = atof(velocity);
		dir = atof(direction);
		compile(locX, locY, vel, dir, new_fd); //compile the sensor data into an object
	}
	
	else{	//else client command was invalid
		send(new_fd, fail, strlen(fail), 0);
		printf("Error: invalid client message\n");
	}

	printf("\nHandle Server waiting... (Ctrl C to Quit)\n\n");
}


//Wait Function (Beej)
	//pass in socket file descriptor listening for connections
	//new socket connection created for all send and recv with connecting client
void wait(int sockfd){	               
	struct sockaddr_storage their_addr;		//client address
    socklen_t addr_size;	
	int new_fd;	//new socket file descriptor of connection for send and recv
	
	while(1){	//accept incoming connection (server loop until SIGINT)
		addr_size = sizeof(their_addr);		
		new_fd = accept(sockfd, (struct addrinfo *)&their_addr, &addr_size);
		if(new_fd == -1){			
			fprintf(stderr, "Error: server/client socket\n");	//validate but continue to let client know/ dont shut down entire server
			continue;
		}
		acceptConnection(new_fd);	//function to accept and relay message with client
		close(new_fd);	//be sure to close the newly created socket
	}
}


//Alarm Function
	//Inputs: int type: type of alarm to activate 
	//	1 - standard notification alert, 2 - menu selection verification, 3 - Obstacle in path
	//	4 - Clear to proceed, 5 - Missing cane beeps, Default- Nothing happens
void alarmSwitch(int type){
	printf("Alarm switch initiated...\n");
	if (type > 47){		//convert type if it is an ascii value into an int
			type = type - 48;
	}
	float htzLevel;		//attributes for beep
	float decibel;
	float length = 1;
	int repeat = 0;
	int lenRepeat = 1;
	float vibrateG;		//attributes for handle shake
	float shakePerSec;
	float vibrateLen;
	int vibrateRep;
	
	switch(type) {
		case 1:
			htzLevel = 5000;	//set code specific attributes
			decibel = 45;
			length = 0.1;
 			vibrateG = 0.6;
			vibrateLen = length;
			vibrateRep = 1;
			//System.goBeep(htzLevel, decibel, length, repeat, lenRepeat);	//theoretical functions
			//System.shakeHandle(vibrateG, vibrateLen, vibrateRep); 	//theoretical functions
			printf("Alarm 1 initiated: <feedback playing> \n");
			break;

		case 2: 
			htzLevel = 8000;
			decibel = 55;
			length = 0.2;
 			vibrateG = 0.75;
			vibrateLen = length;
			vibrateRep = 2;
			//System.goBeep(htzLevel, decibel, length, repeat, lenRepeat);	
			//System.shakeHandle(vibrateG, vibrateLen, vibrateRep);
			printf("Alarm 2 initiated: <menu noise>\n");
			break;
			
		case 3: 
			htzLevel = 12000;
			decibel = 100;
			length = 0.1;
			repeat = 1;      //True, repeat beeps
			lenRepeat = 10;
 			vibrateG = 0.9;
			vibrateLen = length / 2;
			vibrateRep = lenRepeat * 4;
			//forwardMobile(ObstacleAlert); 	//forward to mobile device if possible
			//System.goBeep(htzLevel, decibel, length, repeat, lenRepeat);	
			//System.shakeHandle(vibrateG, vibrateLen, vibrateRep);
			printf("Alarm 3 initiated: Obstacle in path! \n");
			break;
					
		case 4:
			htzLevel = 6000;
			decibel = 55;
			length = 0.1;
			repeat = 1;
			lenRepeat = 2;
			vibrateG = 0.6;
			vibrateLen = length;
			vibrateRep = 1;
			//System.goBeep(htzLevel, decibel, length, repeat, lenRepeat);	
			//System.shakeHandle(vibrateG, vibrateLen, vibrateRep);
			printf("Alarm 4 initiated: Clear...\n");
			break;
			
		case 5:
			htzLevel = 6000;
			decibel = 75;
			length = 0.1;
			repeat = 1;
			lenRepeat = 20;
			vibrateG = 0.7;
			vibrateLen = length;
			vibrateRep = 20;
			//System.goBeep(htzLevel, decibel, length, repeat, lenRepeat);	
			//System.shakeHandle(vibrateG, vibrateLen, vibrateRep);
			printf("Alarm 5 initiated: Beep Beep Beep... \n");
			break;

		default:
			//System.goBeep(htzLevel, decibel, length, repeat, lenRepeat);	
			//System.shakeHandle(vibrateG, vibrateLen, vibrateRep);
			printf("Alarm initiated but no code given \n");
			break;
		}
return;
}


//Compile Function
	//pass in sensor data corrresponding to an object
	//estimates straight line movement of object after 5 seconds
	//estimates straight line movement of user after 5 seconds (can be stationary)
	//calculates is object and user will collide <-to detect obstacle
void compile(float locationX1, float locationY1, float velocity, float direction, float new_fd){
	char* pass = "pass"; 	//checks for client
	char* fail = "fail"; 
	int locationX2 = 0; 	//calculate object estimated end location, ints used to increase collision chance 
	int locationY2 = 0;
	int inPath = 0;		//serves as a bool, 0 = not in path
	
	//estimate locationX2 & locationY2
	float distance5s = 5 * velocity;	//calculate the distance covered by the object in the time frame (5 sec?)
	int section, distanceX, distanceY;
	float percent;
	if (direction <= 90) section = 1;	//find section of direction of direction (based off coordinate plane)
	if ((direction > 90) && (direction <= 180))  section = 2;
	if ((direction > 180) && (direction < 270))  section = 3;
	if (direction >= 270) section = 4;
	switch(section) {					//action depends on section of direction
		case 1:
			percent = direction / 90;	//percent is used to divide the distance covered
			percent = percent * 100;
			distanceX = distance5s * percent;	//divide the distance (this is sort of like breaking apart the rise and run)
			distanceY = distance5s - (distance5s * percent);
			locationX2 = locationX1 + distanceX;	//how the distance values are added to the previous coordinate value depends on the section
			locationY2 = locationY1 + distanceY;
			break;
		case 2:
			percent = (direction - 90) / 90;
			percent = percent * 100;
			distanceX = distance5s * percent;
			distanceY = distance5s - (distance5s * percent);
			locationX2 = locationX1 + distanceX;
			locationY2 = locationY1 - distanceY;
			break;
		case 3:
			percent = (direction - 180) / 90;
			percent = percent * 100;
			distanceX = distance5s * percent;
			distanceY = distance5s - (distance5s * percent);
			locationX2 = locationX1 - distanceX;
			locationY2 = locationY1 - distanceY;
			break;
		case 4:
			percent = (direction - 270) / 90;
			percent = percent * 100;
			distanceX = distance5s * percent;
			distanceY = distance5s - (distance5s * percent);
			locationX2 = locationX1 - distanceX;
			locationY2 = locationY1 + distanceY;
			break;
		default:
			printf("Error: Object distance estimator");
			break;
		}
			
	int userLocationX1 = 0; //calculate user estimated end location, ints used to increase chance of collision
	int userLocationY1 = 0;
	int userLocationX2 = 0;
	int userLocationY2 = 0;
	char userVelocity[BUFFER];		//holder for velocity
    memset(userVelocity, 0, sizeof(userVelocity)); 
	char userDirection[BUFFER];		//holder for direction
    memset(userDirection, 0, sizeof(userDirection));    
	float userVel; //converted forms
	float userDir;
	
	send(new_fd, pass, strlen(pass),0);      //send pass
	recv(new_fd, userVelocity, sizeof(userVelocity)-1, 0); 	//get velocity from client (simulating sensor) 
	send(new_fd, pass, strlen(pass),0);      //send pass
	recv(new_fd, userDirection, sizeof(userDirection)-1, 0); 	//get direction from client
	userVel = atof(userVelocity); 	//convert char userVelocity & userDirection into float userVel & userDir
	userDir = atof(userDirection);
	
	//estimate userLocationX2 & userLocationY2
	float userDistance5s = 5 * userVel;	//calculate the distance covered by the object in the time frame (5 sec?) 
	int userSection, userDistanceX, userDistanceY;
	float userPercent;
	if (userDir <= 90) userSection = 1;	//find section of direction of direction (based off coordinate plane)
	if ((userDir > 90) && (userDir <= 180))  userSection = 2;
	if ((userDir > 180) && (userDir < 270))  userSection = 3;
	if (userDir >= 270) userSection = 4;
	switch(userSection) {					//action depends on section of direction
		case 1:
			userPercent = userDir / 90;		//follow the same pattern as before
			userPercent = userPercent * 100;
			userDistanceX = userDistance5s * userPercent;
			userDistanceY = userDistance5s - (userDistance5s * userPercent);
			userLocationX2 = userLocationX1 + userDistanceX;
			userLocationY2 = userLocationY1 + userDistanceY;
			break;
		case 2:
			userPercent = (userDir - 90) / 90;
			userPercent = userPercent * 100;
			userDistanceX = userDistance5s * userPercent;
			userDistanceY = userDistance5s - (userDistance5s * userPercent);
			userLocationX2 = userLocationX1 + userDistanceX;
			userLocationY2 = userLocationY1 - userDistanceY;
			break;
		case 3:
			userPercent = (userDir - 180) / 90;
			userPercent = userPercent * 100;
			userDistanceX = userDistance5s * userPercent;
			userDistanceY = userDistance5s - (userDistance5s * userPercent);
			userLocationX2 = userLocationX1 - userDistanceX;
			userLocationY2 = userLocationY1 - userDistanceY;
			break;
		case 4:
			userPercent = (userDir - 270) / 90;
			userPercent = userPercent * 100;
			userDistanceX = userDistance5s * userPercent;
			userDistanceY = userDistance5s - (userDistance5s * userPercent);
			userLocationX2 = userLocationX1 - userDistanceX;
			userLocationY2 = userLocationY1 + userDistanceY;
			break;
		default:
			printf("Error: User distance estimator");
			break;
		}
		
	if (userLocationX2 != 0 && userLocationY2 != 0){	//if the user is moving, call intersect function to see if lines cross
		struct Point o1 = {locationX1, locationY1}, o2 = {locationX2, locationY2}; 
    	struct Point u1 = {userLocationX1, userLocationY1}, u2 = {userLocationX2, userLocationY2}; 
    	inPath = intersect(o1,o2,u1,u2);	//function returns 1 if the lines created from the passed points crossed
	}
	else {	//the user is stationary, calculate if a line crosses the origin
		float slope = (locationY2 - locationY1) / (locationX2 - locationX1);
		float remainder = (slope * locationX1)- locationY1;
		if (remainder == 0 || slope == 0) {
			inPath = 1;
		}
	}
	
	identify(inPath);  //send important object attributes to identify function
	return;
}


//Function Min
	//returns the lowest of the two passed in ints
	//defaults to b if equal
int min (int a, int b)
{
	if (a < b) return a;
	else return b;
}


//Function Max
	//returns the highest of the two passed in ints
	//defaults to a if equal
int max(int a, int b)
{
	if (a < b) return b;
	else return a;
}


//onSegment Function
//SOURCE: https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
int onSegment(struct Point p, struct Point q, struct Point r) 
{ 
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && 
        q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y)) 
       return 0; 
  
    return 1; 
} 
  
  
//Orientation Function
//SOURCE: https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
int orientation(struct Point p, struct Point q, struct Point r) 
{ 
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y); 
    if (val == 0) {
    	return 0;	
	}  
    return (val > 0)? 1: 2; 
} 
  
//Intersect Function
//SOURCE: https://www.geeksforgeeks.org/check-if-two-given-line-segments-intersect/
int intersect(struct Point p1, struct Point q1, struct Point p2, struct Point q2) 
{ 
    int o1 = orientation(p1, q1, p2); 
    int o2 = orientation(p1, q1, q2); 
    int o3 = orientation(p2, q2, p1); 
    int o4 = orientation(p2, q2, q1); 

    if (o1 != o2 && o3 != o4) return 1; 
    if (o1 == 0 && onSegment(p1, p2, q1)) return 1; 
    if (o2 == 0 && onSegment(p1, q2, q1)) return 1; 
    if (o3 == 0 && onSegment(p2, p1, q2)) return 1; 
    if (o4 == 0 && onSegment(p2, q1, q2)) return 1; 
  
    return 0; 
}

//Identify Function
	//Hiearchy of object classification choices based on attributes
	//as choices are added, passed in attributes will be added
void identify(int inPath){
	if (inPath){
		alarmSwitch(3);
	}
return;
}


//Main Function
int main(int argc, char *argv[]){
	if(argc != 2){                                                    
		fprintf(stderr, "Error: server arg number\n");	//validate argument number
		exit(1);
	}
	struct addrinfo* address = prepAddress(argv[1]);
	int sockfd = createSocket(address);
	bindSocket(sockfd, address);
	listenSocket(sockfd);
	printf("Handle Server on port: %s\n", argv[1]);
	wait(sockfd);
	freeaddrinfo(address);
}

