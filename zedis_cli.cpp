/*
 * =====================================================================================
 *
 *       Filename:  zedis_cli.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/26/2014 11:55:50 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <zmq.hpp>
#include <iostream>
#include <string>

int main(int argc,char * argv[])
{
    zmq::context_t context(1);
	zmq::socket_t req(context, ZMQ_REQ);
	req.connect(argv[1]);
	for(int i=2; i<argc; i++){
		zmq::message_t message(strlen(argv[i]));
		memcpy(message.data(),argv[i],strlen(argv[i])+1);
		if(i!=argc-1){
			req.send(message,ZMQ_SNDMORE);
		}else{
			req.send(message);
		}
	}
	while(true){
		zmq::message_t message;
		req.recv(&message);
		std::cout<<message.size()<<"."<<std::string((char*)message.data(),message.size())<<std::endl;
		if(!message.more()){
			break;
		}
	}
}
