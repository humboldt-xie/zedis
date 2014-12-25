/*
 * =====================================================================================
 *
 *       Filename:  zedis.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/07/2014 09:35:33 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include "leveldb/db.h"
#include <zmq.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <typeinfo>
using namespace std;
//  Receive 0MQ string from socket and convert into string
static std::string
s_recv (zmq::socket_t & socket) {

    zmq::message_t message;
    socket.recv(&message);

    return std::string(static_cast<char*>(message.data()), message.size());
}

//  Convert string to 0MQ string and send to socket
static bool
s_send (zmq::socket_t & socket, const std::string & string,int flag=0) {

    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());

    bool rc = socket.send (message,flag);
    return (rc);
}

//  Sends string as 0MQ string, as multipart non-terminal
static bool
s_sendmore (zmq::socket_t & socket, const std::string & string) {

    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());

    bool rc = socket.send (message, ZMQ_SNDMORE);
    return (rc);
}
struct s_server
{
	leveldb::DB* db;
	leveldb::Options options;
	std::string ldb_filepath;
	int32_t     port;
	int32_t     pub_port;
}server;

void command_get(zmq::socket_t & sock,zmq::message_t * message,int size){
	//  Send reply back to client
		string result="";
		leveldb::Status s = server.db->Get(leveldb::ReadOptions(), 
				string((char*)message[1].data(),message[1].size()),
				&result
				);
		if(s.ok()){
			s_send (sock, "ok",ZMQ_SNDMORE);
			s_send (sock, result);
		}else if(s.IsNotFound()){
			s_send (sock, "notfound");
		}else{
			s_send (sock, "error",ZMQ_SNDMORE);
			s_send (sock, s.ToString());
		}
}
void command_set(zmq::socket_t & sock,zmq::message_t * message,int size){
	//  Send reply back to client
		leveldb::Status s = server.db->Put(leveldb::WriteOptions(), 
				string((char*)message[1].data(),message[1].size()),
				string((char*)message[2].data(),message[2].size())
				);
		string result="error";
		if(s.ok()){
			result="ok";
		}
		s_send (sock, result);
}
void command_del(zmq::socket_t & sock,zmq::message_t * message,int size){
	//  Send reply back to client
		leveldb::Status s = server.db->Delete(leveldb::WriteOptions(), 
				string((char*)message[1].data(),message[1].size())
				);
		string result="error";
		if(s.ok()){
			result="ok";
		}
		s_send (sock, result);
}
void command_sync(zmq::socket_t & sock,zmq::message_t * message,int size)
{
	string key=string((char*)message[1].data(),message[1].size());
	string num=string((char*)message[2].data(),message[2].size());
	int32_t nnum=atoll(num.c_str());
	leveldb::Iterator* it = server.db->NewIterator(leveldb::ReadOptions());
	it->Seek(key);
	for (int offset=0; it->Valid() && offset<nnum; it->Next(),offset++) {
		s_send(sock,it->key().ToString(),ZMQ_SNDMORE);
		s_send(sock,it->value().ToString(),ZMQ_SNDMORE);
	}
	if(it->Valid()){
		s_send(sock,"continue");
	}else{
		s_send(sock,"ok");
	}
	delete it;
}

class ibinder
{
public:
	virtual void set(string v)=0;
	virtual string get()=0;
};
template<typename T>
class tbinder:public ibinder
{
public:
	T * m_v;
	tbinder(T * v){
		m_v=v;
	}
	void set(string v)
	{
		stringstream s(v);
		s>>*m_v;
	}
	string get()
	{
		stringstream s;
		s<<*m_v;
		return s.str();
	}
};
template<typename T>
void bind_arg(map<string,ibinder*> &binder,string v,string lv,T *p){
	ibinder* bp=new tbinder<T>(p);
	ibinder* lbp=new tbinder<T>(p);
	binder[v]=bp;
	binder[lv]=lbp;
}

template<typename T>
string toStr(T & t)
{
	stringstream s;
	s<<t;
	return s.str();
}

int main(int argc,char *argv[])
{

	server.ldb_filepath="./text";
	server.port=4279;//="4279";
	server.pub_port=4289;//="4289";
	map<string,ibinder*> m_b;
	bind_arg(m_b,"-p","--port",&server.port);
	bind_arg(m_b,"-u","--pub_port",&server.pub_port);
	bind_arg(m_b,"-f","--file_path",&server.ldb_filepath);
	for(int i=0; i<argc-1; i++){
		char *k = argv[i];
		if(m_b.find(string(k))!=m_b.end()){
			char * v=argv[i+1];
			++i;
			m_b[k]->set(v);
		}
	}
	cout<<"port:"<<server.port<<endl;

	server.options.create_if_missing = true;

	leveldb::Status status = leveldb::DB::Open(server.options,server.ldb_filepath , &server.db);

    zmq::context_t context(1);

	zmq::socket_t responder(context, ZMQ_REP);
	//responder.connect("tcp://localhost:5560");
	std::string address=std::string("tcp://*:")+toStr(server.port);	
	responder.bind(address.c_str());
 
	while(1)
	{
		zmq::message_t message[1024];
		int index=0;
		int max_msg=1024;
		while(true){
			responder.recv(&message[index]);
			std::cout<<message[index].size()<<"[.]"<<(char*)message[index].data()<<std::endl;
			if(!message[index].more()){
				break;
			}
			index++;
		}

		std::string cmd(static_cast<char*>(message[0].data()), message[0].size());
		if(cmd=="get"){
			command_get(responder,message,index);
		}
		if(cmd=="set"){
			command_set(responder,message,index);
		}
		if(cmd=="del"){
			command_del(responder,message,index);
		}
		if(cmd=="sync"){
			command_sync(responder,message,index);
		}
		//std::cout << "Received request: " << cmd << std::endl;
	}

	return 0;
};
