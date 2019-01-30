#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h> 
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cmath>
#include <sstream>
#include <string>
#include<fcntl.h>
#include<unordered_map>
using namespace std;

#define SYNPORT 8000
#include <fstream>
#include<thread>

#define SEND_DATA 1
#define RECEIVE_DATA 2
#define CHUNKSIZE 2048
#define CHUNKSIZEFLOAT 2048.0

unordered_map<string,string> SEEDER_LIST;
int PORT;
void update_as_seeder_to_tracker(string operation_at_tracker,int trk1_prt){

	int clientSocket ,ret;
	struct sockaddr_in serverAddr;
	
	int ack=1;
	clientSocket=socket(PF_INET,SOCK_STREAM,0); //sock define tcp communication
	if(clientSocket<0){
		cout<<"Error in creating Socket"<<endl;
		exit(1);
	}
	memset(&serverAddr,'\0',sizeof(serverAddr));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(trk1_prt);
	serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");

	ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
	if(ret < 0){
		cout<<"Error in connection"<<endl;
		
	}else{
	
	
	int operation_at_tracker_length,RequestType;
	RequestType = RECEIVE_DATA;
	send(clientSocket,&RequestType,sizeof(int),0);

	operation_at_tracker_length=operation_at_tracker.length();
	send(clientSocket,&operation_at_tracker_length,sizeof(int),0);

	send(clientSocket,operation_at_tracker.c_str(),operation_at_tracker_length,0);

	close(clientSocket);
}

}
void write_to_file(){

ofstream soutfile("mystream.txt",std::ofstream::binary);
soutfile.close();
//ifstream infile("myseeder.txt",std::ofstream::binary);

fstream outfile("mystream.txt", ios::in | ios::out);
//int i=0;
for (auto itr = SEEDER_LIST.begin(); itr != SEEDER_LIST.end();itr++)
{
	/* code */
	string sha = (*itr).first+"\n";
	string clients = (*itr).second+"\n";
	//outfile.seekp(2*i*5);
	outfile.write(sha.c_str(),sha.length());
	outfile.write(clients.c_str(),clients.length());
	cout<<"SHA "<<sha<<endl;
	cout<<"CLIENTS "<<clients<<endl;
	
	
}
outfile.close();

}

void write_to_map(){
	string line;
	fstream reader("mystream.txt", ios::in | ios::out);
	while (getline(reader, line)){
			string value;
			getline(reader, value);
			SEEDER_LIST[line]=value;
	}
}

void split(string command , string & operation , string & value){
	int i=1;
	while(i<command.length()){
		if(command[i]==' '){
			operation = command.substr(0,i);
			value = command.substr(i+1);
			
			break;
		}
		i++;
	}
}


void serving_Requests(int newsocket,struct sockaddr_in newAddr){

	float progress;
	
	int rack=1;
	
	cout<<"Connected  \n";
	
	printf("IP address is: %s\n", inet_ntoa(newAddr.sin_addr));
	printf("port is: %d\n", (int) ntohs(newAddr.sin_port));
	cout<<endl;
	int shalength;
	recv(newsocket,&shalength,sizeof(shalength),0);
	cout<<shalength;
	char SHA_BUFFER_READER[shalength+20];
//cout<<"sscc";

	recv(newsocket,SHA_BUFFER_READER,shalength,0); 
	//int infile = open("infile.txt",O_RDONLY,O_SYNC);
	
	string command;
	string search = (string) SHA_BUFFER_READER;
	command = search.substr(0,shalength);

	update_as_seeder_to_tracker(command,9000);

	cout<<"Requested For SHA: "<<command;
	string line;
	

	
	string operation,value;
	split(command,operation,value);
	cout<<command;

	if(operation=="search"){
		line  = SEEDER_LIST[value];
		cout<<line;
//string ss = to_string(last_part_size);
		int linesize= line.length();
		send(newsocket,&linesize,sizeof(linesize),0);
		//cout<<"length of shalength sent";
		recv(newsocket,&rack,sizeof(rack),0);
		//cout<<"recieved ackknowledge";
		//usleep(25000);
		
		send(newsocket,line.c_str(),linesize,0);
		recv(newsocket,&rack,sizeof(rack),0);
		
	}else if(operation=="add"){
		string sha;
		cout<<"Adding to map";
		split(value,sha,value);
		if(SEEDER_LIST[sha].length()<2){
			SEEDER_LIST[sha]=value;	
		}
		else{
			SEEDER_LIST[sha]=SEEDER_LIST[sha]+" "+value;
		}
		write_to_file();
	}else if(operation=="remove"){
					int found_at;
					string Whole_String="";
					string sha;
					split(value,sha,value);
					string realString = SEEDER_LIST[sha];
					found_at = realString.find(value);

				
					if(found_at>3){
						Whole_String = realString.substr(0,found_at-1);
					}

					if(found_at+value.length()==realString.length()){
							Whole_String = Whole_String;

					}else
					{
						Whole_String = Whole_String + " "+realString.substr(found_at+value.length()+1);
						cout<<Whole_String<<"--";

					}
					cout<<" Removed Entry : "<<Whole_String<<Whole_String;

					if(Whole_String.length()==0){
						SEEDER_LIST.erase(sha);
					}else{
						SEEDER_LIST[sha]=Whole_String;
					}

					write_to_file();

	}else if(operation=="show"){
		cout<<endl;
		for(auto itr = SEEDER_LIST.begin();itr!= SEEDER_LIST.end();itr++){
						cout<<(*itr).first<<"--->"<<(*itr).second<<endl;
					}
	}

		close(newsocket);
	
		printf("\nEnding connection with ");
		printf("IP address : %s with ", inet_ntoa(newAddr.sin_addr));
		printf("port  %d\n", (int) ntohs(newAddr.sin_port));
	
}

void server_for_other_tracker(){
	int socketfd,ret;
	struct sockaddr_in serveraddr;
	
	socketfd=socket(PF_INET,SOCK_STREAM,0);
	if(socketfd<0){
		cout<<"\nError in connection"<<endl;
		exit(1);
	}
	memset(&serveraddr,'\0',sizeof(serveraddr));

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(SYNPORT);
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	ret = bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
	
	
	if(ret < 0){
		cout<<"\nError in Binding"<<endl;
		exit(1);
	}
	if(listen(socketfd,5)==0){
		cout<<"\nListening...";
	}else{
		cout<<"\nError in Listening"<<endl;
		exit(1);
	}

	struct sockaddr_in newAddr;
	int newsocket;
	socklen_t addr_size;
	addr_size=sizeof(newAddr);
	while(1){

	newsocket= accept(socketfd,(struct sockaddr*)&newAddr,&addr_size);
	


	if(newsocket>=0){
		


		int RequestType;
		recv(newsocket,&RequestType,sizeof(int),0);

		if(RequestType==SEND_DATA){
				ifstream infile("mystream.txt",ifstream::binary);
				infile.seekg (0,infile.end);
  				int size = infile.tellg();
  				infile.seekg(0,infile.beg);
  				//infile.read (buffer,size);
				int no_of_parts = ceil((float)size/CHUNKSIZEFLOAT);
				
				int part_no=1;
				int last_part_size = size - (no_of_parts-1)*CHUNKSIZE;
  				send(newsocket,&size,sizeof(int),0);

  					while(part_no<=no_of_parts){
  						if(part_no==no_of_parts){
  								char BUFFER[last_part_size+1];
  								infile.read (BUFFER,last_part_size);
  								send(newsocket,BUFFER,last_part_size,0);
  						}else{
  								char BUFFER[CHUNKSIZE+1];
  								infile.read (BUFFER,CHUNKSIZE);
  								send(newsocket,BUFFER,CHUNKSIZE,0);
  								

  						}
  						part_no++;
  					}

  					infile.close();
  					close(newsocket);
		}

		else if(RequestType==RECEIVE_DATA){
				

				int shalength;
	recv(newsocket,&shalength,sizeof(shalength),0);
	cout<<shalength;
	char SHA_BUFFER_READER[shalength+20];
//cout<<"sscc";

	recv(newsocket,SHA_BUFFER_READER,shalength,0); 
	//int infile = open("infile.txt",O_RDONLY,O_SYNC);
	
	string command;
	string search = (string) SHA_BUFFER_READER;
	command = search.substr(0,shalength);

	

	cout<<"Requested For SHA: "<<command;
	string line;
	

	
	string operation,value;
	split(command,operation,value);
	cout<<command;
		if(operation=="add"){
			string sha;
			cout<<"Adding to map";
			split(value,sha,value);
			if(SEEDER_LIST[sha].length()<2){
				SEEDER_LIST[sha]=value;	
			}
			else{
				SEEDER_LIST[sha]=SEEDER_LIST[sha]+" "+value;
			}
			write_to_file();
		}else if(operation=="remove"){
					int found_at;
					string Whole_String="";
					string sha;
					split(value,sha,value);
					string realString = SEEDER_LIST[sha];
					found_at = realString.find(value);

				
					if(found_at>3){
						Whole_String = realString.substr(0,found_at-1);
					}

					if(found_at+value.length()==realString.length()){
							Whole_String = Whole_String;

					}else
					{
						Whole_String = Whole_String + " "+realString.substr(found_at+value.length()+1);
						cout<<Whole_String<<"--";

					}
					cout<<" Removed Entry : "<<Whole_String<<Whole_String;

					if(Whole_String.length()==0){
						SEEDER_LIST.erase(sha);
					}else{
						SEEDER_LIST[sha]=Whole_String;
					}

					write_to_file();

				close(newsocket);

				
		}else{
			cout<<"Some Error Occured"<<endl;
		}

	}


}

}

}void thread_for_clients(){
	fstream outfile("mystream.txt", ios::in);
	if(outfile){
		outfile.close();
		write_to_map();

	}
	else{
		outfile.close();
	}


	int socketfd,ret;
	struct sockaddr_in serveraddr;
	
	socketfd=socket(PF_INET,SOCK_STREAM,0);
	if(socketfd<0){
		cout<<"\nError in connection"<<endl;
		exit(1);
	}
	memset(&serveraddr,'\0',sizeof(serveraddr));

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	ret = bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
	
	
	if(ret < 0){
		cout<<"\nError in Binding"<<endl;
		exit(1);
	}
	if(listen(socketfd,5)==0){
		cout<<"\nListening...";
	}else{
		cout<<"\nError in Listening"<<endl;
		exit(1);
	}

	struct sockaddr_in newAddr;
	int newsocket;
	socklen_t addr_size;
	addr_size=sizeof(newAddr);
	while(1){

	newsocket= accept(socketfd,(struct sockaddr*)&newAddr,&addr_size);
	


	if(newsocket>=0){
		thread RequestThread(serving_Requests,newsocket,newAddr);
		RequestThread.detach();
	}


}

}


int main(){

	cout<<"Enter Port No :";
	cin>>PORT;
	int clientSocket ,ret;
	struct sockaddr_in serverAddr;
	
	int ack=1;
	clientSocket=socket(PF_INET,SOCK_STREAM,0); //sock define tcp communication
	if(clientSocket<0){
		cout<<"Error in creating Socket"<<endl;
		exit(1);
	}
	memset(&serverAddr,'\0',sizeof(serverAddr));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(9000);
	serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");

	ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
	




	if(ret < 0){
		//cout<<"Error in connection"<<endl;
		//tracker 1 is not present

	fstream outfile("mystream.txt", ios::in);
	if(outfile){
		outfile.close();
		write_to_map();

	}
	else{
		outfile.close();
	}
	}else{

		int Request = SEND_DATA;
		send(clientSocket,&Request,sizeof(Request),0);
		int total_file_size;
		recv(clientSocket,&total_file_size,sizeof(int),0);
		cout<<"size to recieve"<<total_file_size<<endl;
		int no_of_parts = ceil((float)total_file_size/CHUNKSIZEFLOAT);
		int last_part_size = total_file_size - (no_of_parts-1)*CHUNKSIZE;

		int part_no=1;
		ofstream outfile("mystream.txt",ofstream::binary);

		while(part_no<=no_of_parts){
			
			if(part_no==no_of_parts){
				char BUFFER[last_part_size+1];
				recv(clientSocket,BUFFER,last_part_size,0);
				outfile.write (BUFFER,last_part_size);	
		
			}
			else{
			
				char BUFFER[last_part_size+1];
				recv(clientSocket,BUFFER,last_part_size,0);
				outfile.write (BUFFER,CHUNKSIZE);	
			}
			part_no++;
		}
		outfile.close();
		write_to_map();
		close(clientSocket);
	}
	thread server_sync(server_for_other_tracker);
	

	thread thread_for_C(thread_for_clients);
	thread_for_C.join();
	server_sync.join();



	return 0;


}