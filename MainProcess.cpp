#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fstream>
#include<iostream>
#include <unistd.h>
#include<vector>
#include<cmath>
#include<fcntl.h>
#include<sys/wait.h> 
#include <semaphore.h>
#include<thread>
#include<map>
#include <openssl/sha.h>
using namespace std;


#define REAL_CHUNK_SIZE 524288
#define CHUNKSIZE 2048
#define CHUNKSIZEFLOAT 2048.0
#define BLOCK_FOT 524288
#define BLOCKSIZECHUNK 524288.0
int PORT;
int REQUEST_FOR_VECTOR=1;
int REQUEST_FOR_DATA=2;
int FILESIZE = 1574493;
int Tracker_port_1,Tracker_port_2;

string IP_ADDRESS;
map<string,vector<int>> MEMORY;
map<string,string> FILES_WITH_NAMES;
map<string,int> FILES_WITH_SIZES;
map<string,string> DOWNLOADS;

sem_t mutex;
void write_seed_log();

void write_to_file();
void split(string command , string & operation , string & value){
	int i=1;
	cout<<"comnd "<<command;
	while(i<command.length()){
		if(command[i]==' '){

			operation = command.substr(0,i);
			value = command.substr(i+1);
			
			break;          
		}
		i++;
	}
}
void get_SHA_Of_SHA(unsigned char buffer[],int len,char *res){
	unsigned char SHA_Buffer[SHA_DIGEST_LENGTH];
	char buf[SHA_DIGEST_LENGTH*2];
int i;
	memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
	memset(SHA_Buffer, 0x0, SHA_DIGEST_LENGTH);

	SHA1((unsigned char *)buffer, len, SHA_Buffer);
	for (i=0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf((char*)&(buf[i*2]), "%02x", SHA_Buffer[i]);
	}
for(int i=0;i<SHA_DIGEST_LENGTH*2;i++){
			res[i]=buf[i];
		}
	
}
void getsha(unsigned char buffer[],int len,char *res){
	unsigned char SHA_Buffer[SHA_DIGEST_LENGTH];
	char buf[SHA_DIGEST_LENGTH*2];
int i;
	memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
	memset(SHA_Buffer, 0x0, SHA_DIGEST_LENGTH);

	SHA1((unsigned char *)buffer, len, SHA_Buffer);
	for (i=0; i < SHA_DIGEST_LENGTH; i++) {
		sprintf((char*)&(buf[i*2]), "%02x", SHA_Buffer[i]);
	}
for(int i=0;i<SHA_DIGEST_LENGTH*2;i++){
			res[i]=buf[i];
		}
	
}
void make_torrent(string file_path,string torrname){
string shatotal="";
	string filename,filenameDup;
	filename=file_path;
	filenameDup=filename;
	ifstream infile (filename.c_str(),std::ifstream::binary);
	
filename = torrname;
	//filename=filename+".mtorrent";
	ofstream outfile (filename.c_str(),std::ofstream::binary);
	infile.seekg (0,infile.end);
  	int size = infile.tellg();
  	infile.seekg(0,infile.beg);
  	//infile.read (buffer,size);
	char buffer[BLOCK_FOT+1];
	int parts = ceil((float)size/BLOCKSIZECHUNK);
	int part_no=1;
	int last_part_size = size - (parts-1)*BLOCK_FOT;

	while(part_no<=parts){
		
		memset(buffer,'\0',BLOCK_FOT+1);

		if(part_no==parts){
		memset(buffer,'\0',BLOCK_FOT+1);
		
		infile.read (buffer,last_part_size);	
		
		char *res = new char[SHA_DIGEST_LENGTH*2];
		getsha((unsigned char *)buffer,last_part_size,res);
		
		for(int i=0;i<20;i++){
			//cout<<res[i];
			shatotal=shatotal+res[i];
		}
		break;
		}
		else{
		memset(buffer,'\0',BLOCK_FOT+1);
			infile.read (buffer,BLOCK_FOT);	
			
			char *res = new char[SHA_DIGEST_LENGTH*2];
		getsha((unsigned char *)buffer,BLOCK_FOT,res);
		
		for(int i=0;i<20;i++){
			
			shatotal=shatotal+res[i];
		}

		}
		part_no++;
	}
	string torrentdata = "";
	string p1 = to_string(Tracker_port_1);
	string p2 = to_string(Tracker_port_2);

	string tracker1 = "127.0.0.1 "+p1;
	string tracker2 = "127.0.0.1 "+p2;
	
	torrentdata = torrentdata + tracker1 +"\n";
	torrentdata = torrentdata + tracker2 +"\n";
	torrentdata = torrentdata + filenameDup+"\n";
	torrentdata = torrentdata + to_string(size)+"\n";
	outfile.write (torrentdata.c_str(),torrentdata.length());
	
	outfile.write (shatotal.c_str(),(parts*20));
	infile.close();
	outfile.close();



cout<<"\n Torrent Created Successfully";


}
string get_sha_from_torrent(string torrent_location,string & fz){

			//string torrent_location,destination_location;
			//split(value,torrent_location,destination_location);

			ifstream ofstr(torrent_location.c_str(),ifstream::binary);
			string line;
			
			getline(ofstr,line);
			string tracker_1 = line;

			getline(ofstr,line);
			string tracker_2 = line;

			getline(ofstr,line);
			getline(ofstr,line);
			string filesize;
			filesize=line;
			fz = filesize;
			getline(ofstr,line);
			string sha = line;


			

			unsigned char bufferS[sha.length()+1];
			for(int i=0;i<sha.length();i++){
				bufferS[i]=sha[i];
			}
			
			char *res = new char[SHA_DIGEST_LENGTH*2];

			get_SHA_Of_SHA(bufferS,sha.length(),res);
			
		

			string shatotal="";
			for(int i=0;i<20;i++){
					

				shatotal=shatotal+res[i];
			
			}
			

			ofstr.close();

			return shatotal;
}



void get_data_vector(int port,vector<int> & v,string ip,string sha){
	
	int clientSocket;
	struct sockaddr_in serverAddr;
	int ack=1;
	cout<<"Connecting to server with PORT "<<port<<" and IP "<<ip;
	cout<<" sha size "<<FILES_WITH_SIZES[sha]<<endl;
	int no_of_parts = ceil((float)FILES_WITH_SIZES[sha]/CHUNKSIZEFLOAT);

	cout<<"file with siz "<<no_of_parts;
	char buffer[no_of_parts+1];
	clientSocket=socket(PF_INET,SOCK_STREAM,0); //sock define tcp communication
	
	if(clientSocket<0){
		cout<<"Error in creating Socket"<<endl;
		exit(1);
	}
	memset(&serverAddr,'\0',sizeof(serverAddr));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(port);
	serverAddr.sin_addr.s_addr=inet_addr(ip.c_str());

	int ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
	if(ret < 0){
		cout<<"Error in connection"<<endl;
		exit(1);
	}
	memset(buffer,'\0',no_of_parts+1);
	
	int RequestType = REQUEST_FOR_VECTOR;
	
	send(clientSocket,&RequestType,sizeof(int),0);

	recv(clientSocket,&ack,sizeof(int),0);



	int size_of_sha = sha.length();
	send(clientSocket,&size_of_sha,sizeof(int),0);
	recv(clientSocket,&ack,sizeof(ack),0);
	
	send(clientSocket,sha.c_str(),size_of_sha,0);
	



	char bufferVector[no_of_parts+1];

	recv(clientSocket,bufferVector,no_of_parts,0);

	//cout<<"no of parts "<<no_of_parts;
	for(int i=0;i<no_of_parts;i++){
		int val = bufferVector[i]-'0';
		//cout<<val;
		if(val==1){
		v.push_back(1);
		//cout<<"--"<<i+1<<"--";
		}else{
			v.push_back(0);
		}
		
	
	}
	

	

	close(clientSocket);

}


void downloadData(ofstream & outfile,int port,string ip,vector<int> pieceVector,string sha){

	cout<<"Trying to  DOWNLOAD Pieces FROM SERVER WITH PORT"<<port;

	int clientSocket;
	struct sockaddr_in serverAddr;
	int ack=1;
	cout<<"Connecting to server with PORT "<<port<<" and IP "<<ip;
	int no_of_parts = ceil((float)FILES_WITH_SIZES[sha]/CHUNKSIZEFLOAT);
	char buffer[CHUNKSIZE+1];
	clientSocket=socket(PF_INET,SOCK_STREAM,0); //sock define tcp communication
	
	if(clientSocket<0){
		cout<<"Error in creating Socket"<<endl;
		exit(1);
	}
	memset(&serverAddr,'\0',sizeof(serverAddr));
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons(port);
	serverAddr.sin_addr.s_addr=inet_addr(ip.c_str());

	int ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
	if(ret < 0){
		cout<<"Error in connection"<<endl;
		exit(1);
	}
	cout<<"Connected with port"<<port<<endl;
	memset(buffer,'\0',no_of_parts+1);
	
	int RequestType = REQUEST_FOR_DATA;
	
	send(clientSocket,&RequestType,sizeof(int),0);
	recv(clientSocket,&ack,sizeof(int),0);
	

	int size_of_sha = sha.length();
	
	send(clientSocket,&size_of_sha,sizeof(int),0);
	recv(clientSocket,&ack,sizeof(ack),0);
	
	send(clientSocket,sha.c_str(),size_of_sha,0);
	recv(clientSocket,&ack,sizeof(ack),0);

	char dataBuffer[CHUNKSIZE+1];
	int total_pieces=pieceVector.size();
	cout<<"Total pieces to take from this client "<<total_pieces;



	//sending information that i loop till these no. of pieces
	send(clientSocket,&total_pieces,sizeof(total_pieces),0);
	recv(clientSocket,&ack,sizeof(ack),0);

	int last_part_size = FILES_WITH_SIZES[sha] - (no_of_parts-1)*CHUNKSIZE;
int pieceNo=1;
		int size_of_file=0;
	while(pieceNo<=total_pieces){
		int pice = pieceVector[pieceNo-1];//start
		//cout<<"Request For Piece No" <<pice<<endl;
		if(pice==no_of_parts){
		//		cout<<"Last Part Request";
				size_of_file=last_part_size;
		}
		else{
				size_of_file=CHUNKSIZE;
		}
		
		//send(clientSocket,&size_of_file,sizeof(size_of_file),0);
		//recv(clientSocket,&ack,sizeof(ack),0);

		send(clientSocket,&pice,sizeof(pice),0);
		//cout<<"REquested to "<<port<<" for pieceNo "<<pice<<endl;
		//recv(clientSocket,&ack,sizeof(ack),0);
		
		memset(dataBuffer,'\0',CHUNKSIZE+1);
		
		recv(clientSocket,dataBuffer,CHUNKSIZE,0);

		sem_wait(&mutex); 

		outfile.seekp(0);
		int pos = (pice-1)*CHUNKSIZE;		
		outfile.seekp(pos);
		outfile.write (dataBuffer,size_of_file);
		MEMORY[sha][pice-1]=1;
		sem_post(&mutex); 
   
		
	//	send(clientSocket,&ack,sizeof(ack),0);


		pice++;
		pieceNo++;
	
	}


	close(clientSocket);

}

void PieceSelectionAlgo(vector<vector<int>> bits,vector<vector<int>> & resultbits,vector<int> & peers){

	vector<int> peerssum;
	vector<int> load;
	for(int i=0;i<bits.size();i++){
		vector<int> ex,exres;
		int sum=0;
		for(int j=0;j<bits[0].size();j++){
			int res = bits[i][j];
			sum=sum+res;
			
			exres.push_back(0);
		}
		peerssum.push_back(sum);
		
		
		load.push_back(0);
		resultbits.push_back(exres);
	}
	

	for(int i=0;i<peerssum.size();i++){
		int min=i;

		for(int j=i+1;j<peerssum.size();j++){
			if(peerssum[min]>peerssum[j]){
				min=j;

			}
		}

		int temp=peerssum[min];
		peerssum[min]=peerssum[i];
		peerssum[i]=temp;
		vector<int> tmpvec = bits[i];
		bits[i]=bits[min];
		bits[min]=tmpvec;
		int tempper = peers[i];
		peers[i]=peers[min];
		peers[min]=tempper;
	}
for(int i=0;i<bits[0].size();i++){
		vector<int> loadindex;
		for(int j=0;j<bits.size();j++){
			if(bits[j][i]==1){
				loadindex.push_back(j);
				//kis kis peer ke paas ye bit on hai jo ascending order main hai

			}
		}
		
		int minloader = loadindex[0]; 
		for(int k=1;k<loadindex.size();k++){
					 if(load[minloader]>load[loadindex[k]]){
					 	minloader = loadindex[k];
					 }
		}
		resultbits[minloader][i]=1;
		load[minloader]++;
	
	}



}
void get_Details_from_tracker(string  sha,int trk1_prt,int trk2_prt,vector<string> & ip_s,vector<int> & port_s,vector<string> & addresses){

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
		cout<<"Tracker One Not Working"<<endl;
		memset(&serverAddr,'\0',sizeof(serverAddr));
		serverAddr.sin_family=AF_INET;
		serverAddr.sin_port=htons(trk2_prt);
		serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
		ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
		if(ret<0){
			cout<<"Tracker 2 Also Down, Try Again Later";
			exit(0);
		}
		//exit(1);
	}else{
		cout<<"Connectd to tracker one";
	}
	
	
	int shalength;
	sha = "search "+sha;
	shalength=sha.length();
	send(clientSocket,&shalength,sizeof(shalength),0);

	send(clientSocket,sha.c_str(),shalength,0);
	int clients_length;

recv(clientSocket,&clients_length,sizeof(clients_length),0);
send(clientSocket,&ack,sizeof(ack),0);
char buffer_for_clients[clients_length+10];
recv(clientSocket,buffer_for_clients,clients_length,0);
send(clientSocket,&ack,sizeof(ack),0);
string Recieved_clients = (string) buffer_for_clients;
Recieved_clients = Recieved_clients.substr(0,clients_length);

int i=0,k=0;


int data_pointer=0;
cout<<"RECIEVED META DATA FROM TRACKER\n";
while(Recieved_clients[i]!='\0'){

	if(Recieved_clients[i]==' '){

		string data = Recieved_clients.substr(k,(i-k));
		if(data_pointer==0){
			ip_s.push_back(data);
			cout<<"\nIp Address : "<<data<<endl;
			data_pointer++;
			k=i+1;
		}else if(data_pointer==1){
			port_s.push_back(atoi(data.c_str()));
			cout<<"PORT : "<<data<<endl;
			data_pointer++;
			k=i+1;
		}else if(data_pointer==2){
			addresses.push_back(data);
			cout<<"Physical Address : "<<data<<endl;
			data_pointer=0;
			k=i+1;
		}
	}
	i++;


}


string data = Recieved_clients.substr(k,(i-k));
addresses.push_back(data);
cout<<"Physical Address : "<<data<<endl;

close(clientSocket);

}

void update_as_seeder_to_tracker(string operation_at_tracker,int trk1_prt,int trk2_prt){

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
	cout<<"tracker port"<<trk1_prt;
	serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");

	ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
	if(ret < 0){

		cout<<"Tracker 1 Not Working"<<endl;
		cout<<"Trying to connect to Tracker 2";
		memset(&serverAddr,'\0',sizeof(serverAddr));
		serverAddr.sin_family=AF_INET;
		serverAddr.sin_port=htons(trk2_prt);
		cout<<"tracker port"<<trk2_prt;
		serverAddr.sin_addr.s_addr=inet_addr("127.0.0.1");
		ret = connect(clientSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
		if(ret < 0){
			cout<<"Tracker 2 also down";
			exit(0);
		}
		//exit(1);
	}
	
	
	int operation_at_tracker_length;
	
	operation_at_tracker_length=operation_at_tracker.length();
	send(clientSocket,&operation_at_tracker_length,sizeof(int),0);

	send(clientSocket,operation_at_tracker.c_str(),operation_at_tracker_length,0);

	close(clientSocket);
}

void real_client_thread(string sha,string torrentname,string filenametostore){
	string fz;

	sha = get_sha_from_torrent(torrentname,fz);

	int filesize = atoi(fz.c_str()); 




	vector<string> ip_s;
	//vector<int> port_s;
	vector<string> addresses;



	vector<int> peers; 
FILES_WITH_SIZES[sha]=filesize;//peers->ports

	get_Details_from_tracker(sha,Tracker_port_1,Tracker_port_2,ip_s,peers,addresses);



for(int i=0;i<addresses.size();i++){
	cout<<ip_s[i]<<endl;
	cout<<peers[i]<<endl;
	cout<<addresses[i]<<endl;
}
	vector<vector<int>> bits,resultbits;
	

		
	

	//peers.push_back(3000);
	//peers.push_back(4000);

	string ip="127.0.0.1";

for(int ii=0;ii<peers.size();ii++){
	vector<int> vect;

	bits.push_back(vect);
}
	
	
	int no_of_parts = ceil((float)FILES_WITH_SIZES[sha]/CHUNKSIZEFLOAT);
	


	thread * threadpointervectorrecieve = new thread[peers.size()];
 	for (int i = 0; i < peers.size(); i++)
    {
      threadpointervectorrecieve[i] = thread(get_data_vector,peers[i],ref(bits[i]),ip_s[i],sha);
     
    }
  	for (int i = 0; i < peers.size(); i++)
      // Join will block our main thread, and so the program won't exit until
      // everyone comes home.
    {
      threadpointervectorrecieve[i].join();
    }
  	delete [] threadpointervectorrecieve;


/*
	thread thread1(get_data_vector,peers[0],ref(vect),ip,sha);
	thread thread12(get_data_vector,peers[0],ref(vect2),ip,sha);
	thread1.join();
	
	thread12.join();

	bits.push_back(vect);
	bits.push_back(vect2);
*/
	/////////////peers.push_back(3000);
	////////////////////peers.push_back(4000);
	cout<<"Result"<<endl;
	for(int i=0;i<resultbits.size();i++){
		//cout<<" "<<peers[i]<<" : ";
		for(int j=0;j<resultbits[0].size();j++){
		//	cout<<bits[i][j]<<" ";		
		}
		
		cout<<"\n";
	}

	PieceSelectionAlgo(bits,ref(resultbits),ref(peers));


	// cout<<"Result"<<endl;
	// for(int i=0;i<resultbits.size();i++){
	// 	cout<<" "<<peers[i]<<" : ";
	// 	for(int j=0;j<resultbits[0].size();j++){
	// 		cout<<resultbits[i][j]<<" ";		
	// 	}
		
	// 	cout<<"\n";
	// }
vector<vector<int>> VectorResult;
	for(int i=0;i<resultbits.size();i++){
		vector<int> temper;
		for(int j = 0;j<resultbits[0].size();j++){
			if(resultbits[i][j]==1){
				temper.push_back(j+1);
				
			}
		}
		VectorResult.push_back(temper);
	}

	

	cout<<"Result"<<endl;

	for(int i=0;i<VectorResult.size();i++){
		//cout<<" "<<peers[i]<<" : ";
		for(int j=0;j<VectorResult[i].size();j++){
		//	cout<<VectorResult[i][j]<<"-";		
		}
		
		//cout<<"\n";
	}


	string port_in_String = to_string(PORT);
	string operation_at_tracker = "add "+sha+" 127.0.0.1 "+port_in_String+" "+filenametostore;

	update_as_seeder_to_tracker(operation_at_tracker,Tracker_port_1,Tracker_port_2);
		vector<int> Mbits;
		for(int i=0;i<no_of_parts;i++){
			Mbits.push_back(0);
		}
		MEMORY[sha]=Mbits;
		FILES_WITH_NAMES[sha]=filenametostore;

		vector<int> xxxxx = MEMORY[sha];

		//cout<<"\nBEFORE\n";
	for(auto itr = xxxxx.begin();itr!=xxxxx.end();itr++){
	//	cout<<(*itr);
	}

	ofstream outfile(filenametostore.c_str(),std::ofstream::binary);




thread * threadpointerDATArecieve = new thread[peers.size()];
 	for (int i = 0; i < peers.size(); i++)
    {
      threadpointerDATArecieve[i] = thread(downloadData,ref(outfile),peers[i],ip,VectorResult[i],sha);
     
    }
  	for (int i = 0; i < peers.size(); i++)
      // Join will block our main thread, and so the program won't exit until
      // everyone comes home.
    {
      threadpointerDATArecieve[i].join();
    }
  	delete [] threadpointerDATArecieve;



/*

	thread thread2(downloadData,ref(outfile),peers[0],ip,VectorResult[0],sha);
	
	thread thread22(downloadData,ref(outfile),peers[1],ip,VectorResult[1],sha);
	thread2.join();
	thread22.join();
*/
	xxxxx = MEMORY[sha];
	for(auto itr = xxxxx.begin();itr!=xxxxx.end();itr++){
//		cout<<(*itr);
	}
	//cout<<"\nAFTER\n";
	
//	cout<<"end";

	outfile.close();
	string D_line = filenametostore + "   "+to_string(filesize )+" Bytes\n";
	DOWNLOADS[sha]=D_line;

	write_to_file();

}
void write_to_file(){

ofstream soutfile("downloads_log.txt",std::ofstream::binary);
soutfile.close();
//ifstream infile("myseeder.txt",std::ofstream::binary);

fstream outfile("downloads_log.txt", ios::in | ios::out);
//int i=0;
for (auto itr = DOWNLOADS.begin(); itr != DOWNLOADS.end();itr++)
{
	/* code */
	string sha = (*itr).first;
	string clients = (*itr).second+"\n";
	//outfile.seekp(2*i*5);
	outfile.write(sha.c_str(),sha.length());
	outfile.write(clients.c_str(),clients.length());
	
	
}
outfile.close();

}
void clientThread(){


string op;
while(1){
cout<<"\nEnter command : ";
cin>>op;

	if(op=="share"){
		string str;
	//cout<<"Enter File you want to share";
	cin>>str;
	string torr;
	cin>>torr;
	//cout<<str;
	make_torrent(str,torr);
	string cc = str;
	str = torr;
	string fzz;
	string sha_of_sha = get_sha_from_torrent(str,fzz);

	//cout<<"REAL SHA IS : "<<sha_of_sha;
	string port_in_String = to_string(PORT);
	string filenametostore = str;
	string operation_at_tracker = "add "+sha_of_sha+" 127.0.0.1 "+port_in_String+" "+cc;
	update_as_seeder_to_tracker(operation_at_tracker,Tracker_port_1,Tracker_port_2);




	vector<int> bits;
	//POINTERS_TO_FILE["abcde"] = new ifstream("testingtext.txt",std::ifstream::binary);
	//int no_of_parts = ceil((float)FILESIZE/CHUNKSIZEFLOAT);
	

	ifstream infile (cc.c_str(),std::ifstream::binary);
	infile.seekg (0,infile.end);
  	int size = infile.tellg();
  	infile.close();
  	int no_of_parts = ceil((float)size/CHUNKSIZEFLOAT);
	
  	FILES_WITH_SIZES[sha_of_sha]=size;
	//cout<<no_of_parts<<"---";
	for(int i=0;i<no_of_parts;i++){
		bits.push_back(1);
	}


	MEMORY[sha_of_sha]=bits;
	FILES_WITH_NAMES[sha_of_sha]=cc;


	}else if(op=="quit"){
		write_seed_log();
		exit(0);
	}

	else if(op=="show"){
		string ss;
		cin>>ss;
		ifstream infile ("downloads_log.txt",std::ifstream::binary);
		if(infile){
			for(auto itr = DOWNLOADS.begin();itr!=DOWNLOADS.end();itr++){
				cout<<"\n"<<(*itr).second;
			}
		}else{
			cout<<"No Downloads Till yet"<<endl;
		}
	}
	else if(op=="remove"){
		string torrentname;
		cin>>torrentname;
		string fz;
		string sha1 = get_sha_from_torrent(torrentname,fz);
		string port_in_String = to_string(PORT);
		string operation_at_tracker = "remove "+sha1+" 127.0.0.1 "+port_in_String+" "+FILES_WITH_NAMES[sha1];
		update_as_seeder_to_tracker(operation_at_tracker,Tracker_port_1,Tracker_port_2);

	}

	else if(op=="get"){



	string sha="abcde";
	//cout<<"Enter Torrent Name";
	string torrentname;

	cin>>torrentname;
	
	string filenametostore;
	//cout<<"\nEnter File Name with which you want to store : ";
	cin>>filenametostore;
	thread clientThreadR(real_client_thread,sha,torrentname,filenametostore);
	clientThreadR.detach();
	}

}//while loop end


}





void serveRequest(int newsocket,struct sockaddr_in newAddr){
	
	printf("Connected to IP address is: %s\n", inet_ntoa(newAddr.sin_addr));
	printf("port is: %d\n", (int) ntohs(newAddr.sin_port));
	int ack=1;
	int RequestType;
	recv(newsocket,&RequestType,sizeof(int),0);
	send(newsocket,&ack,sizeof(int),0);

	int size_of_sha;

	//cout<<RequestType<<endl;
	if(REQUEST_FOR_VECTOR==RequestType){
		recv(newsocket,&size_of_sha,sizeof(int),0);
		send(newsocket,&ack,sizeof(int),0);

		char BufferForSha[size_of_sha+1];
		recv(newsocket,&BufferForSha,size_of_sha,0);

		string sha = (string) BufferForSha;
		sha = sha.substr(0,size_of_sha);
		cout<<"Request for Bit Vector of "<<sha<<endl;
		cout<<MEMORY[sha].size();
		char buffervector[MEMORY[sha].size()+1];
		for(int k=0;k<MEMORY[sha].size();k++){
			cout<<MEMORY[sha][k];
			buffervector[k]=(char)(MEMORY[sha][k] + 48);
		}
		send(newsocket,buffervector,MEMORY[sha].size(),0);


	}else if(REQUEST_FOR_DATA==RequestType){
		cout<<"Request for data";
		ack=99;

		recv(newsocket,&size_of_sha,sizeof(int),0);
		send(newsocket,&ack,sizeof(int),0);
			//cout<<"size of sha"<<size_of_sha;
		char BufferForSha[size_of_sha+1];
		recv(newsocket,&BufferForSha,size_of_sha,0);
		send(newsocket,&ack,sizeof(int),0);
		string sha = (string) BufferForSha;
		sha = sha.substr(0,size_of_sha);

		//cout<<"Request Data for sha "<<sha<<endl;
			int no_of_parts = ceil((float)FILES_WITH_SIZES[sha]/CHUNKSIZEFLOAT);
		int gettingPiece,totalPieces;
		recv(newsocket,&totalPieces,sizeof(gettingPiece),0);
		send(newsocket,&ack,sizeof(int),0);
		cout<<"Total Pieces to send : "<<totalPieces<<endl;
		string filename = FILES_WITH_NAMES[sha];
		cout<<"Requested for file "<<filename;
		ifstream infile (filename.c_str(),std::ifstream::binary);
		if(infile){
			cout<<"opened success";
		}
		int last_part_size = FILES_WITH_SIZES[sha] - (no_of_parts-1)*CHUNKSIZE;
		int piece_no = 1;
		infile.seekg(0,infile.beg);
		int Pice;
		int fetchChunkSize=0;
		char dataBuffer[CHUNKSIZE+1];
		while(piece_no<=totalPieces){
	
			//recv(newsocket,&fetchChunkSize,sizeof(fetchChunkSize),0);
			

			//send(newsocket,&ack,sizeof(int),0);
	
			recv(newsocket,&Pice,sizeof(int),0);
			if(Pice==no_of_parts){
				fetchChunkSize=last_part_size;
			}else{
				fetchChunkSize=CHUNKSIZE;
			}
			//send(newsocket,&ack,sizeof(int),0);
	
			//cout<<"Request For Piece : "<<Pice<<endl;
	
			memset(dataBuffer,'\0',CHUNKSIZE+1);
			infile.seekg(0);
			int pos = (Pice-1)*CHUNKSIZE;		
			infile.seekg(pos);
		
			infile.read (dataBuffer,fetchChunkSize);	
				
			send(newsocket,dataBuffer,fetchChunkSize,0);
			//recv(newsocket,&ack,sizeof(int),0);
		
			piece_no++;
		
		
	}

	}else{
		cout<<"NO SYNC BETWEEN PEERS PLEASE RESTART";

	}
	close(newsocket);
	
}


void serverThread(){

	
	
	int socketfd,ret;
	struct sockaddr_in serveraddr;
	

	socketfd=socket(PF_INET,SOCK_STREAM,0);
	if(socketfd<0){
		cout<<"Error in connection"<<endl;
		exit(1);
	}
	memset(&serveraddr,'\0',sizeof(serveraddr));

	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(PORT);
	serveraddr.sin_addr.s_addr=inet_addr("127.0.0.1");
	
	ret = bind(socketfd,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
	
	
	if(ret < 0){
		cout<<"Error in Binding"<<endl;
		exit(1);
	}
	if(listen(socketfd,5)==0){
		cout<<"Listening...";
	}else{
		cout<<"Error in Listening"<<endl;
		exit(1);
	}

	struct sockaddr_in newAddr;
	int newsocket;
	socklen_t addr_size;
	addr_size=sizeof(newAddr);

	while(1){
	newsocket= accept(socketfd,(struct sockaddr*)&newAddr,&addr_size);
	
	if(newsocket>=0){
		thread RequestThread(serveRequest,newsocket,newAddr);
		RequestThread.detach();
	}
	//cout<<"iiii";
	}

	close(socketfd);

}



void write_to_map(){
	string line;
	fstream reader("downloads_log.txt", ios::in | ios::out);
	while (getline(reader, line)){
			string value;
			getline(reader, value);
			DOWNLOADS[line]=value;
	}
	reader.close();
}
void write_seed_log(){
	int k;

ofstream soutfile("seed_log.txt",std::ofstream::binary);
soutfile.close();
//ifstream infile("myseeder.txt",std::ofstream::binary);

fstream outfile("seed_log.txt", ios::in | ios::out);




	for(auto itr = MEMORY.begin();itr!=MEMORY.end();itr++){
			string sha;
			sha = (*itr).first;
			string filesize;
			int fz;
			string filename;
			fz= FILES_WITH_SIZES[sha];
			filesize = to_string(fz);
			filename = FILES_WITH_NAMES[sha];
			
			string port_in_String = to_string(PORT);
			string operation_at_tracker = "remove "+sha+" 127.0.0.1 "+port_in_String+" "+filename;
			update_as_seeder_to_tracker(operation_at_tracker,Tracker_port_1,Tracker_port_2);


			sha = sha+"\n";
			filesize = filesize+"\n";
			filename = filename+"\n";

			outfile.write(sha.c_str(),sha.length());
			
			outfile.write(filesize.c_str(),filesize.length());
			outfile.write(filename.c_str(),filename.length());
	

	}
	outfile.close();

}
void read_seed_log(){
	string line;
	fstream reader("seed_log.txt", ios::in | ios::out);
	

	while (getline(reader, line)){
			string sha;
			sha=line;
			string value;
			getline(reader, value);
			
			
			string filesize;
			filesize=value;
			int fz = atoi(filesize.c_str());
			getline(reader, value);
			string filename;
			filename = value;

			string port_in_String = to_string(PORT);
			
			string operation_at_tracker = "add "+sha+" 127.0.0.1 "+port_in_String+" "+filename;

			update_as_seeder_to_tracker(operation_at_tracker,Tracker_port_1,Tracker_port_2);

			int no_of_parts = ceil((float)fz/CHUNKSIZEFLOAT);
			vector<int> Mbits;
			for(int i=0;i<no_of_parts;i++){
				Mbits.push_back(1);
			}
			MEMORY[sha]=Mbits;
			FILES_WITH_NAMES[sha]=filename;
			FILES_WITH_SIZES[sha]=fz;
			//DOWNLOADS[line]=value;
	}
	reader.close();
}

int main(){
	

	sem_init(&mutex, 0, 1); 
	cout<<"ENTER YOUR PORT NUMBER : ";
	cin>>PORT;
	
	cout<<"ENTER TRACKER 1 PORT NUMBER : ";
	cin>>Tracker_port_1;
	
	cout<<"ENTER TRACKER 2 PORT NUMBER : ";
	cin>>Tracker_port_2;
	


	
	write_to_map();

	read_seed_log();
	thread serverThreadObj(serverThread);
	thread clientThreadCall(clientThread);
	
	clientThreadCall.join();
	serverThreadObj.join();

}