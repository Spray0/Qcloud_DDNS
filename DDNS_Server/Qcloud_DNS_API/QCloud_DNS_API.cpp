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


#include "QCloud_DNS_API.hpp"

// HTTP
std::string str_SecretId;
std::string str_SecretKey;
std::string domain;
std::string recordId;
std::string subDomain;
std::string recordType;
std::string recordLine;

std::string str_Return;   

// Time
struct timeval time_stamp;
std::string str_utime,str_time; 

// 从文件读取参数
void Q_DNS::Read_InitTxT(){
    std::ifstream stream("Init.txt");
	std::string line;
	if(stream) {
		while (getline (stream, line))
		{ 
			int index=line.find('=');
			if		(line.substr(0,index)=="str_SecretId")		str_SecretId=line.substr(index+1,line.length());
			else if	(line.substr(0,index)=="str_SecretKey")		str_SecretKey=line.substr(index+1,line.length());
			else if	(line.substr(0,index)=="domain")			domain=line.substr(index+1,line.length());
			else if	(line.substr(0,index)=="subDomain")			subDomain=line.substr(index+1,line.length());
			else if	(line.substr(0,index)=="recordType")		recordType=line.substr(index+1,line.length());
			else if	(line.substr(0,index)=="recordLine")		recordLine=line.substr(index+1,line.length());
		}
	}
}

// 获取解析记录ID
std::string GetRecordId(){
    std::string recordId ="";

    // 获取系统时间 
    gettimeofday(&time_stamp, NULL);
    str_time=std::to_string(time_stamp.tv_sec);
    str_utime=std::to_string(time_stamp.tv_usec);

    //构建参数内容
    std::string str_Content;
    str_Content+="Action=RecordList";
    str_Content+="&Nonce="+str_utime;
    str_Content+="&SecretId="+str_SecretId;
    str_Content+="&Timestamp="+str_time;

    str_Content+="&domain="+domain;
    str_Content+="&subDomain="+subDomain;

     // HmacSHA1
    unsigned char digest[EVP_MAX_MD_SIZE] = {'\0'};  
    {
        //构建内容
        std::string HmacSHA1_Content="POSTcns.api.qcloud.com/v2/index.php?"+str_Content;
        //使用密钥进行加密
        unsigned int digest_len = 0;  
        HMAC(EVP_sha1(), str_SecretKey.c_str(), str_SecretKey.length(), (unsigned char*)HmacSHA1_Content.c_str(), HmacSHA1_Content.length(), digest, &digest_len);
    }
    // Base64
    std::string Base64_Result;
    {   
        // Base64编码
        char* Base64_temp;
        Base64Encode((char*)digest,&Base64_temp);
        Base64_Result=Base64_temp;
    }
    // URL
    std::string Signature_Result;
    {
        for(int c=0;c<Base64_Result.length();++c){
            switch (Base64_Result.at(c))
            {
            case '=':
                Signature_Result+="%3D";
                break;
            case '+':
                Signature_Result+="%2B";
                break;
            case '/':
                Signature_Result+="%2F";
                break;
            default:
                Signature_Result+=Base64_Result.at(c);
                break;
            }
        }
    }

    //添加请求签名
    str_Content+="&Signature="+Signature_Result;

     //  libcurl
    CURL *curl;
    CURLcode res;
    FILE* fptr;
    struct curl_slist *http_header = NULL;

    if ((fptr = fopen(FILENAME,"w")) == NULL)
	{
		fprintf(stderr,"fopen file error:%s\n",FILENAME);
		return recordId;
	}

    curl = curl_easy_init();
    if (!curl)
    {
        fprintf(stderr,"curl init failed\n");
        return recordId;
    }

    curl_easy_setopt(curl,CURLOPT_URL,POSTURL); //url地址
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,str_Content.c_str()); //post参数
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data); //对返回的数据进行操作的函数地址
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fptr); //这是write_data的第四个参数值
    curl_easy_setopt(curl,CURLOPT_POST,1); //设置问非0表示本次操作为post

    res = curl_easy_perform(curl);
  
    if (res != CURLE_OK)return recordId;

    curl_easy_cleanup(curl);

    int index_s=str_Return.find("\"records\":[{\"id\":")+17;
    int index_e=str_Return.find(',',index_s);
    return str_Return.substr(index_s,index_e-index_s);
}

// 修改DNS解析记录
int Q_DNS::RecordModify(std::string IP_Addr){
    
    // 从文件读取参数
    if(str_SecretId.length()==0)Read_InitTxT();
    // 读取记录ID
    recordId=GetRecordId();

    // 获取系统时间 
    gettimeofday(&time_stamp, NULL);
    str_time=std::to_string(time_stamp.tv_sec);
    str_utime=std::to_string(time_stamp.tv_usec);

    // 构建参数内容
    std::string str_Content;
    str_Content+="Action=RecordModify";
    str_Content+="&Nonce="+str_utime;
    str_Content+="&SecretId="+str_SecretId;
    str_Content+="&Timestamp="+str_time;

    str_Content+="&domain="+domain;
    str_Content+="&recordId="+recordId;
    str_Content+="&recordLine="+recordLine;
    str_Content+="&recordType="+recordType;
    str_Content+="&subDomain="+subDomain;
    str_Content+="&value="+IP_Addr;

    // HmacSHA1
    unsigned char digest[EVP_MAX_MD_SIZE] = {'\0'};  
    {
        //构建内容
        std::string HmacSHA1_Content="POSTcns.api.qcloud.com/v2/index.php?"+str_Content;
        //使用密钥进行加密
        unsigned int digest_len = 0;  
        HMAC(EVP_sha1(), str_SecretKey.c_str(), str_SecretKey.length(), (unsigned char*)HmacSHA1_Content.c_str(), HmacSHA1_Content.length(), digest, &digest_len);
    }
    // Base64
    std::string Base64_Result;
    {   
        // Base64编码
        char* Base64_temp;
        Base64Encode((char*)digest,&Base64_temp);
        Base64_Result=Base64_temp;
    }
    // URL
    std::string Signature_Result;
    {
        for(int c=0;c<Base64_Result.length();++c){
            switch (Base64_Result.at(c))
            {
            case '=':
                Signature_Result+="%3D";
                break;
            case '+':
                Signature_Result+="%2B";
                break;
            case '/':
                Signature_Result+="%2F";
                break;
            default:
                Signature_Result+=Base64_Result.at(c);
                break;
            }
        }
    }

    //添加请求签名
    str_Content+="&Signature="+Signature_Result;

    //std::cout<<str_Content<<std::endl;

    //  libcurl
    CURL *curl;
    CURLcode res;
    FILE* fptr;
    struct curl_slist *http_header = NULL;

    if ((fptr = fopen(FILENAME,"w")) == NULL)
	{
		fprintf(stderr,"fopen file error:%s\n",FILENAME);
		return -1;
	}

    curl = curl_easy_init();
    if (!curl)
    {
        fprintf(stderr,"curl init failed\n");
        return -1;
    }

    curl_easy_setopt(curl,CURLOPT_URL,POSTURL); //url地址
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,str_Content.c_str()); //post参数
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data); //对返回的数据进行操作的函数地址
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fptr); //这是write_data的第四个参数值
    curl_easy_setopt(curl,CURLOPT_POST,1); //设置问非0表示本次操作为post

    res = curl_easy_perform(curl);
  
    if (res != CURLE_OK)return -1;

    curl_easy_cleanup(curl);

    return (str_Return.substr(0,45)=="{\"code\":0,\"message\":\"\",\"codeDesc\":\"Success\",\"")?1:0;
}

// 接受处理函数
size_t write_data(void* buffer,size_t size,size_t nmemb,void *stream){
	FILE *fptr = (FILE*)stream;
	fwrite(buffer,size,nmemb,fptr);
    str_Return=(char *)buffer;
	return size*nmemb;
}

// Base64编码
int Base64Encode(const char* message, char** buffer) { 
    
    BIO *bio, *b64;
    FILE* stream;
    int encodedSize = 4*ceil((double)20/3);
    *buffer = (char *)malloc(encodedSize+1);
  
    stream = fmemopen(*buffer, encodedSize+1, "w");
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_fp(stream, BIO_NOCLOSE);
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); 
    BIO_write(bio, message, 20);
    BIO_flush(bio);
    BIO_free_all(bio);
    fclose(stream);
  
    return 0;
}