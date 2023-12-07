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
#include <bits/stdc++.h>

#define UDPORT "44100"
#define SP 41100
#define LP 42100
#define HP 43100
#define TCPORT "45100"
#define LH "127.0.0.1"
#define TCP_BACK_LOG 5

using namespace std;
// copied from Beej's tutorial  
void sigchld_handler (int s){
	int saved_errno = errno;
	while(waitpid(-1,NULL,WNOHANG)>0);
	errno = saved_errno;
}
//end of the copy

int main(){
	
//Parse the member.txt and put the (username, password) pairs into a map
//The usage of getline() is referred from a stockoverflow post
	map<string, string> MemberList;
	ifstream in("member.txt");
	string line;
	while (getline(in,line)){
		string name,password;
		stringstream current(line);
		while (getline(current,name,',')){
			getline(current,password);
		}

		password.erase(0,1); //erase leading white space after the commma
		password.erase(password.length()-1,1); //erase trailing white space
		MemberList.insert({name,password});
	}


// end of the file parsing

/* create TCP and UDP sockets */
// Copied from Beej's tutorial

	int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes = 1;
	int rv;
	int numbytes;
	bool auth = false;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;  //IPv4 address
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	//TCP
	if ((rv = getaddrinfo(NULL, TCPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rv));
		exit(1);
		}		

	for (p = servinfo; p != NULL;p=p->ai_next){
	
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol))==-1){
			perror("server: socket");
			continue;
		}
	
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))==-1){
			perror("setsocketopt");
			exit(1);
		}
	
		if(bind(sockfd,p->ai_addr, p->ai_addrlen) == -1){
			close(sockfd);
			perror("server:bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo);

	if( p == NULL){
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	
	if (listen(sockfd, TCP_BACK_LOG) == -1) {
		perror("listen");
		exit(1);
	}
	
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags =SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL)==-1){
		perror("sigaction");
		exit(1);
	}
	// end of TCP
	
	// begin of UDP
	int usockfd;
	struct addrinfo h;
	memset(&h,0,sizeof h);
	
	h.ai_family = AF_INET;

	h.ai_socktype=SOCK_DGRAM;

	
	if((rv=getaddrinfo(LH,UDPORT,&h,&servinfo))!=0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}		
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((usockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol))==-1){
			perror("listener: socket");
			continue;
		}
		if (bind(usockfd,p->ai_addr,p->ai_addrlen)==-1){
			close(usockfd);
			perror("listener: bind");
			continue;
		}
		break;
	}
	if (p==NULL){
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}
	freeaddrinfo(servinfo);
	//end of copying from Beej's tutorial
	
	
	// S backend address
	struct sockaddr_in BS;
	BS.sin_family = AF_INET;
	BS.sin_port = htons(SP);
	BS.sin_addr.s_addr = inet_addr(LH);
	struct sockaddr_storage test;
	socklen_t len=sizeof(BS);
	// L backend address
	struct sockaddr_in BL;
	BL.sin_family = AF_INET;
	BL.sin_port = htons(LP);
	BL.sin_addr.s_addr = inet_addr(LH);
	socklen_t len1=sizeof(BL);
	// H backend address
	struct sockaddr_in BH;
	BH.sin_family = AF_INET;
	BH.sin_port = htons(HP);
	BH.sin_addr.s_addr = inet_addr(LH);
	socklen_t len2=sizeof(BH);
	
	
	// end of UDP
	// Initial boot up message
	cout<<"Main Server is up and running."<<endl;
	cout<<"Main Server loaded the member list."<<endl;
	
	// Main connection loop 
	// The while (1){ if (!fork()){}} structure is copied from the 
	// Beej's guide, but modified to suit for this project.
	while(1) {
		char buf[100],buf1[100],buf2[100],buf3[100];
		
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size);
		if (new_fd == -1){
			perror("accept");
			continue;
		}
		//Child socket for TCP connection with the client
		if (!fork()){
			close(sockfd);
			while(1){
			memset(buf,0,sizeof buf);
			memset(buf1,0,sizeof buf1);
				if ((numbytes = recv(new_fd,buf,99,0))==-1){ //receive username
					perror("recv");

					exit(1); 
				}
				if (send(new_fd,"usnrcvd",7,0)==-1){
					perror("send");
					continue;
				}		
	
				if ((numbytes = recv(new_fd,buf1,99,0))==-1){ //receive password
					perror("recv");
					exit(1); 
				}
				if (send(new_fd,"passrcvd",7,0)==-1){
					perror("send");
					continue;
				}	
				cout<<"Main Server received the username and password from the client using TCP over port "<<TCPORT<<endl;
				map<string,string>::iterator it = MemberList.find((string)buf);
				if (it != MemberList.end()){
					string k = MemberList[(string)buf];
					string t = buf1;
					int res = t.compare(k);
					if (res==0){ 
						auth = true;
						cout<<"Password "<<buf1<<" matches the username. Send a reply to the client."<<endl;
						if (send(new_fd,"1",1,0)==-1){ //1 means success 
						perror("send");
						continue; 						
						}
						
					}
					else{
						cout<<"Password "<<buf1 <<" does not match the username. Send a reply to the client."<<endl;
						buf[0]='\0';
						buf1[0]='\0';
						if (send(new_fd,"0",1,0)==-1){ // 0 means wrong password
						perror("send");
						continue; 						
						}	
						
					}
				}
				else {
					cout<<"invalid username"<<endl;
					cout<<buf<<" is not registered. Send a reply to the client."<<endl;
					if (send(new_fd,"-1",2,0)==-1){ //-1 means wrong username
						perror("send");
					}
					
					continue;
				}
				if (auth == true){
					break;
				}
			}
			
			// end of authentication
			// begin of book query
			
			while(1){
				memset(buf2,0,sizeof buf2);
				memset(buf3,0,sizeof buf3);
				if ((numbytes = recv(new_fd,buf2,99,0))==-1){
					perror("recv");
					exit(1); 
				}
				cout<<"Main server received the book request from the client using TCP port "<<TCPORT<<endl;
				if (buf2[0]=='S'){ // Contact S server
					cout<<"Found "<<buf2<<" located at Server S. Send to Server S."<<endl;
					if((numbytes = sendto(usockfd,buf2,4,0,(struct sockaddr*)&BS,sizeof(BS)))==-1){
						perror("talker: sendto");
						exit(1);
					}
				
					if((numbytes = recvfrom(usockfd,buf3,99,0,(struct sockaddr*)&BS,&len)) == -1){
						perror("recvfrom");
						exit(1);
						}
						string number = buf3;
					cout<<number<<" received "<<endl;
					if (buf3[0]=='-'){
						if (send(new_fd,"-1",2,0)==-1){
						perror("send");
						}
						cout<<"Did not find "<<buf2<<" in the book code list."<<endl;
					}
					else if (buf3[0]=='0'){
						cout<<"Main Server received from server S the book status result using UDP over port "<<UDPORT<<" Number of books "<<buf2<<" available is: 0."<<endl;
						if (send(new_fd,"0",2,0)==-1){
						perror("send");
						}
					}
					else{
						cout<<"Main Server received from server S the book status result using UDP over port "<<UDPORT<<" Number of books "<<buf2<<" available is: "<<buf3<<"."<<endl;
						if (send(new_fd,"1",1,0)==-1){
						perror("send");
						}
					}
						cout<<"Main server sent the book status to the client."<<endl;
				}
				else if (buf2[0]=='L'){ // Contact L server
					cout<<"Found "<<buf2<<" located at Server L. Send to Server L."<<endl;
					if((numbytes = sendto(usockfd,buf2,4,0,(struct sockaddr*)&BL,sizeof(BL)))==-1){
						perror("talker: sendto");
						exit(1);
					}
				
					if((numbytes = recvfrom(usockfd,buf3,99,0,(struct sockaddr*)&BL,&len1)) == -1){
						perror("recvfrom");
						exit(1);
						}
						
					
					if (buf3[0]=='-'){
						if (send(new_fd,"-1",2,0)==-1){
						perror("send");
						}
						cout<<"Did not find "<<buf2<<" in the book code list."<<endl;
					}
					else if (buf3[0]=='0'){
						cout<<"Main Server received from server L the book status result using UDP over port "<<UDPORT<<" Number of books "<<buf2<<" available is: 0."<<endl;
						if (send(new_fd,"0",2,0)==-1){
						perror("send");
						}
					}
					else{
						cout<<"Main Server received from server L the book status result using UDP over port "<<UDPORT<<" Number of books "<<buf2<<" available is: "<<buf3<<"."<<endl;
						if (send(new_fd,"1",1,0)==-1){
						perror("send");
						}
					}
					cout<<"Main server sent the book status to the client."<<endl;
				}
				else if (buf2[0]=='H'){ // contact H server
					cout<<"Found "<<buf2<<" located at Server H. Send to Server H."<<endl;
					if((numbytes = sendto(usockfd,buf2,4,0,(struct sockaddr*)&BH,sizeof(BH)))==-1){
						perror("talker: sendto");
						exit(1);
					}
				
					if((numbytes = recvfrom(usockfd,buf3,99,0,(struct sockaddr*)&BH,&len2)) == -1){
						perror("recvfrom");
						exit(1);
						}
						
					
					if (buf3[0]=='-'){
						if (send(new_fd,"-1",2,0)==-1){
						perror("send");
						}
						cout<<"Did not find "<<buf2<<" in the book code list."<<endl;
					}
					else if (buf3[0]=='0'){
						cout<<"Main Server received from server H the book status result using UDP over port "<<UDPORT<<" Number of books "<<buf2<<" available is: 0."<<endl;
						if (send(new_fd,"0",2,0)==-1){
						perror("send");
						}
					}
					else{
						cout<<"Main Server received from server H the book status result using UDP over port "<<UDPORT<<" Number of books "<<buf2<<" available is: "<<buf3<<"."<<endl;
						if (send(new_fd,"1",1,0)==-1){
						perror("send");
						}
					}
					cout<<"Main server sent the book status to the client."<<endl;
				}
				else {
					if (send(new_fd,"-1",2,0)==-1){
						perror("send");
						}
					cout<<"Did not find "<<buf2<<" in the book code list."<<endl;
					continue;	
					
				}
			}
		}
}
//Tear down connections 
close(new_fd); 
close(usockfd);

	return 0;
}