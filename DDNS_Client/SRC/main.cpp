#include <iostream>
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h> 
#include <string>
#include <errno.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <signal.h>

std::ofstream Log_File;
int DDNS_PORT;
std::string DDNS_IP;
std::string str_UsrName;

time_t mytime;
#define Update_time() time(&mytime)
#define Time_str ctime(&mytime)

void Read_InitTxT(void);

#define LOG_ON 
#ifdef LOG_ON
	#define OUT Log_File
#else 
	#define OUT std::cout
#endif 

int sockfd, n, rec_len;
char recvline[4096], sendline[4096];
char buf[4096];
struct sockaddr_in servaddr;
bool status=false;
int main(int argc, char **argv) {

	signal(SIGPIPE, SIG_IGN);

    Read_InitTxT();

    Log_File.open("DDNS.log",std::ios::app);

	

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(DDNS_PORT);

	if (inet_pton(AF_INET, DDNS_IP.c_str(), &servaddr.sin_addr) <= 0) {
		printf("inet_pton error for %s\n", argv[1]);
		exit(0);
	}
	

    Update_time();
    OUT<<std::endl<<std::endl;
	OUT<<">	"<<"Client start @ "<<Time_str<<std::endl;

    ////////////////////////////////////////////////////////////////////////////////////////////
	while(true){
		if(!status){ //需要重连	
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
				exit(0);
			}
			if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) sleep(1); //重连失败 延时1S等待重连
			else{ 
					OUT<<"-	"<<"(Re)Connected to "<<DDNS_IP<<"("<<DDNS_PORT<<") @ "<<Time_str<<std::endl;
					status=true;
				}	
		}else{
			if (send(sockfd,str_UsrName.c_str(),strlen(str_UsrName.c_str()), 0) < 0) status=false;
			if ((rec_len = recv(sockfd, buf, 4096, 0)) == -1)status=false;
			if (!status) OUT<<"-	"<<"Disonnected to "<<DDNS_IP<<"("<<DDNS_PORT<<") @ "<<Time_str<<std::endl;
		}
		//buf[rec_len] = '\0';
		//printf("Received : %s ", buf);
		sleep(1);
		Update_time();
	}
    return 0;

	close(sockfd);
	exit(0);
	return 0;
}

void Read_InitTxT(){
    std::ifstream stream("Init.txt");
	std::string line;
	if(stream) {
		while (getline (stream, line))
		{ 
			int index=line.find('=');
			if		(line.substr(0,index)=="DDNS_PORT")		DDNS_PORT=atoi(line.substr(index+1,line.length()).c_str());
			else if	(line.substr(0,index)=="DDNS_IP")		DDNS_IP=line.substr(index+1,line.length());
            else if	(line.substr(0,index)=="str_UsrName")   str_UsrName=line.substr(index+1,line.length());
		}
	}
}