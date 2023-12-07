#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <map>
#include <fstream>
#include <sstream>

#define TCPServerPort "45100"

#define LH "127.0.0.1"

using namespace std;
int main(){
// Establishing TCP connection with the main server
// Copied from Beej's tutorial 	
	int sockfd, numbytes;	
	struct addrinfo hints, *servinfo, *p;
	int rv;
	struct sockaddr_in my_port;
	socklen_t add_len = sizeof (my_port);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  //IPv4 address
	hints.ai_socktype = SOCK_STREAM;
	string name;
	string password;
	
	if (( rv =getaddrinfo(LH,TCPServerPort,&hints,&servinfo)!=0)){
		fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	for (p = servinfo; p != NULL;p=p->ai_next){
	
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol))==-1){
			perror("client: socket");
			continue;
		}
	
		if (connect(sockfd,p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("client: connect");
			continue;

		}
		break;
	}
	
	if (p == NULL){
		fprintf(stderr,"client:failed to connect\n");
		return 2;
	}
	
	freeaddrinfo(servinfo);
	struct sockaddr_in my_addr;
	socklen_t addr_len;
	addr_len = sizeof(my_addr);
	/*Retrieve the locally-bound name of the specified socket and store it in the
	sockaddr structure*/
	//copied from the project assignment pdf
	int getsock_check=getsockname(sockfd, (struct sockaddr*)&my_addr, &addr_len);
	//Error checking
	if (getsock_check== -1) {
		perror("getsockname");
	exit(1);
	}
	
	unsigned int client_port = my_addr.sin_port;
	cout<<"Client is up and running."<<endl;	
	
/* End of copy from Beej's guide */

/* authentication process with the main server */
	while (1){
		char buf[100],buf2[100];
		memset(buf,0,sizeof buf);
		memset(buf2,0,sizeof buf2);
		string encrypted_name;
		string encrypted_password;
		cout<<"Please enter the username:"<<endl;
		cin>>name;
		cout<<"Please enter the password:"<<endl;
		cin>>password;

	
	//username encryption
		for (string::iterator i = name.begin();i!= name.end();++i){
			if ((int)*i>=97 && (int)*i<=122){ //lowercase letter
			encrypted_name.push_back((char)((*i +5-97)%(122-96)+97));
			}
			else{
			encrypted_name.push_back(*i);
			}
		}
	//password encryption
		for (string::iterator i = password.begin();i!= password.end();++i){
			if ((int)*i>=97 && (int)*i<=122){ //lowercase letters
				encrypted_password.push_back((char)((*i +5-97)%(122-96)+97));
			}
			else if (((int)*i>=65 && (int)*i<=90)){ //uppercase letters
				encrypted_password.push_back((char)((*i +5-65)%(90-64)+65));
			}
			else if(((int)*i>=48 && (int)*i<=57)){ //digits
				encrypted_password.push_back((char)((*i +5-48)%(57-47)+48));
			}
			else{
				encrypted_password.push_back(*i);
			}
		
		}	

		char* n=new char[encrypted_name.length()];
		char* p=new char[encrypted_password.length()];
		strcpy(n,encrypted_name.c_str());
		strcpy(p,encrypted_password.c_str());

		if (send(sockfd,n,strlen(n),0)==-1){
				perror("send");
				continue;
			}		
		if ((numbytes = recv(sockfd,buf2,99,0))==-1){ 
				perror("recv");
				exit(1); 
			}
		
		if (send(sockfd,p,strlen(p),0)==-1){
				perror("send");
				continue;
			}
		if ((numbytes = recv(sockfd,buf2,99,0))==-1){ 
				perror("recv");
				exit(1); 
			}
		cout<<encrypted_name<<" sent an authentication request to the Main Server."<<endl;
		if ((numbytes = recv(sockfd,buf,99,0))==-1){
			perror("recv");
			exit(1);
		}
		
		if (buf[0] == '-'){ // username not found 
			cout<<encrypted_name<<" received the result of authentication from Main Server using TCP over port "<< client_port<<". Authentication failed: Username not found."<<endl;
			continue;
		}	
		if (buf[0] == '0'){ // wrong password
			cout<<encrypted_name<<" received the result of authentication from Main Server using TCP over port  "<< client_port<<". Authentication failed: Password does not match."<<endl;
			continue;
		}			
		if (buf[0] == '1'){ // authentication successful
			cout<<encrypted_name<<" received the result of authentication from Main Server using TCP over port "<< client_port<<". Authentication is successful."<<endl;
			name = encrypted_name;
			break;
		}
		// memory managment
		delete[] n;
		delete[] p;
		
	}
	// end of authentication
	// beginning of book query 
	while (1){
		char buf2[100];
		memset(buf2,0,sizeof buf2);
		cout<<"--- Start a new query ---"<<endl;
		cout<<"Please enter book code to query: "<<endl;
		string bc;
		cin>>bc;
		char* n=new char[bc.length()];
		strcpy(n,bc.c_str());
		if (send(sockfd,n,strlen(n),0)==-1){
				perror("send");
				continue;
			}
		cout<<name<<" sent the request to Main server."<<endl;	
		if ((numbytes = recv(sockfd,buf2,99,0))==-1){ 
				perror("recv");
				exit(1); 
			}
		cout<<"Response received from the main server on TCP port "<<client_port<<endl;
		if (buf2[0]=='1'){
			cout<<"The requested book "<<bc<<" is avaliable in the library."<<endl;
		}
		else if (buf2[0]=='0') {
			cout<<"The requested book "<<bc<<" is Not avaliable in the library."<<endl;
		}
		else{
			cout<<"Not able to find the book-code "<<bc<<" in the system."<<endl;
		}
		buf2[0]='\n';
		delete[] n;

	}
	//tear down connection
	close(sockfd);
	return 0;
}

