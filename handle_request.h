/*************************************************************************
	> File Name: handle_request.h
	> Author: 
	> Mail: 
	> Created Time: 2017年05月29日 星期一 14时41分05秒
 ************************************************************************/

#ifndef _HANDLE_REQUEST_H
#define _HANDLE_REQUEST_H

class HandleRequest{
public:
    //负责读取相应的事件类型，进行分发
    void DistributeEvent(int sockfd);

    //下面就是对于各种不同事件的处理
    void ReplyPut(int sockfd);
    void ReplySendPieces(int sockfd);
    void ReplyDownLoad(int sockfd);
    void ReplyDownLoadPieces(int sockfd);
    void ReplyBack(int sockfd);

private:
    //定义了事件类型
    enum event_type{PUT,SENDPIECES,DOWNLOAD,BACK,DOWNLOADPIECES};
};

void HandleRequest::DistributeEvent(int sockfd){
    
}

void HandleRequest::ReplyPut(int sockfd){

}

void HandleRequest::ReplySendPieces(int sockfd){
    
}

void HandleRequest::ReplyDownLoad(int sockfd){

}


void HandleRequest::ReplyDownLoadPieces(int sockfd){
    
}

void HandleRequest::ReplyBack(int sockfd){

}


#endif
