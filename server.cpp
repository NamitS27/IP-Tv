#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <bits/stdc++.h>

#define TCP_PORT 8080

#define BIT_RATE1 4096
#define BIT_RATE2 2048
#define BIT_RATE3 1024
#define BIT_RATE4 512
#define BIT_RATE5 256

using namespace std;



class station{
    public:
        int station_number;
        string station_name;
        int multicast_address;
        int data_port;
        int info_port;
        int bit_rate;

    public:
        station(int station_number,string station_name, int multicast_address, int data_port,int info_port,int bit_rate) { 
        this->station_number=station_number;
        this->station_name=station_name;
        this->multicast_address=multicast_address;
        this->data_port=data_port;    
        this->info_port=info_port;
        this->bit_rate=bit_rate;
    }

    string to_str(){
        return to_string(this->station_number) + "|" + this->station_name + "|" + to_string(this->multicast_address) + "|" + to_string(this->data_port) + "|" + to_string(this->info_port) + "|" + to_string(this->bit_rate);
    }
    
};



class channel_info{
    public:
        string RADIO;
        int MULTI_PORT;
        int INFO_PORT;
        int BUF_SIZE;
        string video_filename;
        string duration_filename;
};

map<int,bool> v;
vector<station> station_list;

void initialize_map(){
    for(int i=1;i<=255;i++){
        v[i]=true;
    }
}


int get_free_number(){
    for(auto it:v){
        if(it.second) return it.first;
    }
    return -1;
}


bool add_station(){
    
    int station_number = get_free_number();
    if(station_number!=-1){
        string addr = "239.192.1." + to_string(station_number); //Why do we need new ip address for each station, can't we just assign it different port ?
        const char *address = addr.c_str();
        int data_port = 5000+station_number;
        int info_port = 6000+station_number;
        station new_station(station_number,"Station"+to_string(station_number),inet_addr(address),data_port,info_port,BIT_RATE1);
        station_list.push_back(new_station);
        v[station_number] = false;
        return true;
    }
    return false;
}

void remove_station(int station_number){
    v[station_number] = true;
    int ind=0;
    
    for(auto ind_station : station_list) {
        if(ind_station.station_number==station_number) {
            station_list.erase(station_list.begin()+ind);
            break;
        }
        ind++;
    }
}

string serialize_station_list(){
    string data="";
    for(auto it:station_list) data += it.to_str() + "&";
    return data.substr(0,data.size()-1);
}

void* fetch_stations(void* input){
    
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TCP_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        string data = serialize_station_list();
        send(new_socket, &data[0], data.size(), 0);
    }
    
}

int main(int argc, char *argv[]){
    
    initialize_map();

    add_station() ? cout << "Station Added Successfull\n" : cout << "Cannot add station!\n";
    add_station() ? cout << "Station Added Successfull\n" : cout << "Cannot add station!\n";

    pthread_t ptid;

    pthread_create(&ptid,NULL,fetch_stations,NULL);
    sleep(0.5);
    
    while(true)
    {
        cout << "\n1. Add Station \n2. Remove Station\n3. Quit\n\nEnter your choice: ";
        int input;
        cin >> input;   
        if(input==1)                    
            add_station() ? cout << "Station Added Successfully\n" : cout << "Cannot add the station!\n";
        else if(input==2){                           
            cout << "Enter station number for removing: ";
            int remove_station_number;
            cin >> remove_station_number;
            remove_station(remove_station_number);
            cout << "Station removed\n";
        } 
        else break;
    }

    pthread_exit(NULL);    
}