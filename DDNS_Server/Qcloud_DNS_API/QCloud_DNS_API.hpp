/*
       _____                        ____ 
      / ___/____  _________ ___  __/ __ \
      \__ \/ __ \/ ___/ __ `/ / / / / / /
     ___/ / /_/ / /  / /_/ / /_/ / /_/ / 
    /____/ .___/_/   \__,_/\__, /\____/  
        /_/               /____/         

    2020.3.3 ver1.0
    腾讯云解析
*/
#ifndef QCLOUD_DNS_API_HPP_
#define QCLOUD_DNS_API_HPP_

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <x86_64-linux-gnu/curl/curl.h>
#include <openssl/hmac.h>   
#include <openssl/bio.h>
#include <openssl/evp.h> 
#include <math.h>
#include <unistd.h>

#define POSTURL "https://cns.api.qcloud.com/v2/index.php"
#define FILENAME   "Qcloud_DNS_API/curlposttest.log"

extern std::string str_SecretId;
extern std::string str_SecretKey;
extern std::string domain;
extern std::string recordId;
extern std::string subDomain;
extern std::string recordType;
extern std::string recordLine;


namespace Q_DNS{  
    void Read_InitTxT(void);
    int RecordModify(std::string IP_Addr);
}
size_t write_data(void* buffer,size_t size,size_t nmemb,void *stream);
int Base64Encode(const char* message, char** buffer);

#endif