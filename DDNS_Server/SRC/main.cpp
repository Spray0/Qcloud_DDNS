
#include <iostream>
#include <string>
#include "../Qcloud_DNS_API/QCloud_DNS_API.hpp"

#define LOG_ON 
#ifdef LOG_ON
	#define OUT Log_File
#else 
	#define OUT std::cout
#endif 

std::ofstream Log_File; 
#include "IP_Detector.cpp"
int main(int argc, char **argv)
{
	Read_InitTxT();
	Q_DNS::Read_InitTxT();

	Log_File.open("DDNS.log",std::ios::app);

	time_t Login_time;
	time (&Login_time);

	OUT<<std::endl<<std::endl;
	OUT<<">	"<<"Server start @ "<<ctime(&Login_time)<<std::endl;

	Detector_Init();

	std::string IP;
	while(true){
		IP=Detector_loop();
		IP_now=(IP=="")?IP_now:IP;
		if(IP_now!=IP_last){
			OUT<<"-	"<<"IP changed @ "<<ctime(&Login_time);
			IP_last=IP_now;
			// 更改解析记录
			OUT<<"-	修改"<<subDomain<<"."<<domain<<"解析记录为： "<<IP_now<<std::endl;
    		if(Q_DNS::RecordModify(IP_now))OUT<<"✔	修改成功!"<<std::endl;
			else	OUT<<"ERROR!"<<std::endl;
		}
		sleep(1);
	}
	
	
	Log_File.close();
	
	return 0; 
}