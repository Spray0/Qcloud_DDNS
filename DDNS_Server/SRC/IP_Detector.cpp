#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/types.h> 
#include <errno.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>
#include <fstream>
std::string IP_now;
std::string IP_last;

int PORT=0;
int time_out=0;
std::string str_UsrName;

void Read_InitTxT(){
    std::ifstream stream("Init.txt");
	std::string line;
	if(stream) {
		while (getline (stream, line))
		{ 
			int index=line.find('=');
			if		(line.substr(0,index)=="socket_port")		PORT=atoi(line.substr(index+1,line.length()).c_str());
			else if	(line.substr(0,index)=="str_UsrName")		str_UsrName=line.substr(index+1,line.length());
            else if	(line.substr(0,index)=="time_out")		    time_out=atoi(line.substr(index+1,line.length()).c_str());
		}
	}
}

struct Client_t{
	int fd;
	time_t Login_time;
	struct sockaddr_in Addr;
    int second;
}sp;
std::vector<struct Client_t> Client_list;

struct Client_t Client_struct_make(int connect_fd){
	struct Client_t client;
	socklen_t sin_size = 0;
	sin_size = sizeof(struct sockaddr_in);
	time (&client.Login_time);
	if (getpeername(connect_fd, (struct sockaddr*) &client.Addr, &sin_size) == -1) {
			printf("getpeername error\n");
			exit(0);
		}
	client.fd=connect_fd;
    client.second=0;
	return client;
}
void show_client_list(){
	int total_num=Client_list.size();
	if(total_num>0){
		printf("\n\033[7;37;40m");
		printf("  %-20s %-10s %-25s"," IP Address","Port","      Login Time");
		printf("\033[0m\n");
		for(int c=0;c<total_num;++c){
			printf("  %-20s %-10d %-20s",inet_ntoa(Client_list[c].Addr.sin_addr),ntohs(Client_list[c].Addr.sin_port),ctime(&Client_list[c].Login_time));
		}
		printf("\033[0m\n");
	}else
	{
		printf("\nClient list is empty!\n");
	}
}
pthread_t my_thread;
void* listen(void *arg) {
	int socket_fd=*(int *)arg;

	struct Client_t new_client;
	socklen_t sin_size = 0;
	sin_size = sizeof(struct sockaddr_in);
	while(1){
		
		int connect_fd = accept(socket_fd, (struct sockaddr*) &new_client.Addr, &sin_size);
		if(connect_fd==-1){
			printf("accept new client err!");
			return NULL;
		}
			new_client=Client_struct_make(connect_fd);
			Client_list.push_back(new_client);
            OUT<<"-	"<<"Login: "<<inet_ntoa(new_client.Addr.sin_addr)<<"("<<ntohs(new_client.Addr.sin_port)<<") @ "<<ctime(&new_client.Login_time);
			//将客户端配置成非阻塞
 			int flags = fcntl(connect_fd, F_GETFL, 0);  
    		fcntl(connect_fd, F_SETFL, flags | O_NONBLOCK); 

		//show_client_list();
	}
	return NULL;
}

char buff[4096];
#define MAXLINE 4096
int socket_fd, connect_fd;
void Detector_Init(){

	struct sockaddr_in servaddr;
	struct sockaddr_in clientaddr;
	struct sockaddr_in listendAddr;
	socklen_t sin_size = 0;
	sin_size = sizeof(struct sockaddr_in);
	int opt = SO_REUSEADDR;
	int n;

	//初始化Socket  fd
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("create socket_fd error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	} 
	
	//允许重复绑定使用
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	//初始化  
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); //IP地址设置成INADDR_ANY,让系统自动获取本机的IP地址。
	//servaddr.sin_addr.s_addr=inet_addr("192.168.0.107");
	servaddr.sin_port = htons(PORT); //设置的端口为DEFAULT_PORT  
	//将本地地址绑定到所创建的套接字上  
	if (bind(socket_fd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
		printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	} 
    //开始监听是否有客户端连接  
	if (listen(socket_fd, 10) == -1) {
		printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
		exit(0);
	}




	pthread_create(&my_thread, NULL, listen, &socket_fd);

}

int total_num=0;
int get_byte_num=0;
std::string Detector_loop(){
    std::string IP="";
        total_num=Client_list.size();
		for(int index=0;index<total_num;++index){
			get_byte_num= recv(Client_list[index].fd, buff, MAXLINE, 0);
			//printf("get %d %d [%d]\n",get_byte_num, errno,EINTR);
			switch(get_byte_num){
				case -1: //接收为空
                    Client_list[index].second++;
                    if(Client_list[index].second==time_out){
                        //关闭并且把这个客户端从队列中清除
                        time_t now_time;
						time (&now_time);
                        OUT<<"-	"<<"Logout:"<<inet_ntoa(Client_list[index].Addr.sin_addr)<<"("<<ntohs(Client_list[index].Addr.sin_port)<<") @ "<<ctime(&now_time)<<std::endl;
					    close(Client_list[index].fd);
					    Client_list.erase(Client_list.begin()+index);
					    //打印客户端列表
					    //show_client_list();
                    }
				break;
				case 0:	 //连接中断
					//关闭并且把这个客户端从队列中清除
                    time_t now_time;
					time (&now_time);
                    OUT<<"-	"<<"Logout:"<<inet_ntoa(Client_list[index].Addr.sin_addr)<<"("<<ntohs(Client_list[index].Addr.sin_port)<<") @ "<<ctime(&now_time)<<std::endl;
					close(Client_list[index].fd);
					Client_list.erase(Client_list.begin()+index);
					//打印客户端列表
					//show_client_list();
				break;
				default: //正常接收
					if(get_byte_num>0){
                        Client_list[index].second=0;
						buff[get_byte_num] = '\0';
                        std::string get_msg=&buff[0];
                        if(get_msg.find(str_UsrName)!=get_msg.npos)IP=inet_ntoa(Client_list[index].Addr.sin_addr);
						time_t now_time;
						time (&now_time);
						if (send(Client_list[index].fd, ctime(&now_time), 30, 0) == -1){
							perror("send error");
							close(Client_list[index].fd);
						}
					}
				break;
			}

		}
    return IP;
}

















