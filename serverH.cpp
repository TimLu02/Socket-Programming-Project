#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
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
#define SourcePort "43100"
#define DestPORT 44100
#define LH "127.0.0.1"
using namespace std;

int main(){
	map<string, string> booklist;
//Parse the history.txt and put the (book, num) pairs into a map
//The usage of getline() is referred from a stockoverflow post
	ifstream in("history.txt");
	string line;
	while (getline(in,line)){
		stringstream current(line);
		string book,num;
		while (getline(current,book,',')){
			
			getline(current,num);
		}
		num.erase(0,1);	
		num.erase(num.length()-1,1);
		booklist.insert({book,num});
	}
// end of the file parsing
// Prepare UDP socket
// Copied from the Beej's guide 
	int sockfd;
	struct addrinfo h,*servinfo,*p;
	char buf[100];
	int numbytes,rv;
	memset(buf,0,sizeof buf);
	memset(&h,0,sizeof h);
	
	h.ai_family = AF_INET;

	h.ai_socktype=SOCK_DGRAM;

	
	if((rv=getaddrinfo(LH,SourcePort,&h,&servinfo))!=0){
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
	}		
	for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol))==-1){
		perror("listener: socket");
		continue;
	}
	if (bind(sockfd,p->ai_addr,p->ai_addrlen)==-1){
		close(sockfd);
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
//end of copy
	
	// dest addr
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port = htons(DestPORT);
	dest.sin_addr.s_addr = inet_addr(LH);
	socklen_t len;
	char msg[99];
	char msg1[99];
	

	// finish setup phase
	cout<<"Server H is up and running using UDP on port "<<SourcePort<<endl;
	// Main connection loop
	while (1){
		memset(msg,0,sizeof msg);
		memset(msg1,0,sizeof msg1);
		if((numbytes = recvfrom(sockfd,msg,4,0,(struct sockaddr*)&dest,&len)) == -1){
			perror("recvfrom");
			exit(1);
			}
		cout<<"Server H received "<<msg<<" code from the Main Server "<<endl;
		map<string,string>::iterator it = booklist.find((string)msg);
		if (it!=booklist.end()){
			
			if (it->second !="0"){
				string bc = it->second;
				char* n=new char[bc.length()];
				strcpy(n,bc.c_str());
				if((numbytes = sendto(sockfd,n,sizeof(n),0,(struct sockaddr*)&dest,sizeof(dest)))==-1){
					perror("talker: sendto");
					exit(1);
				}
			cout<<"Server H finished sending the availability status of code "<<msg<<" to the Main Server using UDP on port "<<SourcePort<<endl;
			}
			else{
				if((numbytes = sendto(sockfd,"0",1,0,(struct sockaddr*)&dest,sizeof(dest)))==-1){
				perror("talker: sendto");
				exit(1);
			}
			cout<<"Server H finished sending the availability status of code "<<msg<<" to the Main Server using UDP on port "<<SourcePort<<endl;
			}
		}
		else {
			
			if((numbytes = sendto(sockfd,"-1",2,0,(struct sockaddr*)&dest,sizeof(dest)))==-1){
				perror("talker: sendto");
				exit(1);
			}
			cout<<"Server H finished sending the availability status of code "<<msg<<" to the Main Server using UDP on port "<<SourcePort<<endl;
		}
	}
	//Tear down connection
	close(sockfd);
return 0;
}
