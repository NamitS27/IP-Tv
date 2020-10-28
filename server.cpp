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

map<int,bool> v;

void initialize_map(){
    for(int i=1;i<=255;i++){
        v[i]=true;
    }
}


class station{
    public:
        int station_number;
        string station_name;
        int multicast_address;
        int data_port;
        int info_port;
        int bit_rate;
        bool is_active;

    public:
        station(int station_number,string station_name, int multicast_address, int data_port,int info_port,int bit_rate,bool is_active) { 
        this->station_number=station_number;
        this->station_name=station_name;
        this->multicast_address=multicast_address;
        this->data_port=data_port;    
        this->info_port=info_port;
        this->bit_rate=bit_rate;
        this->is_active=is_active;
    }

    string to_str(){
        return to_string(this->station_number) + "|" + this->station_name + "|" + to_string(this->multicast_address) + "|" + to_string(this->data_port) + "|" + to_string(this->info_port) + "|" + to_string(this->bit_rate) + "|" + to_string(this->is_active);
    }
    
};

vector<station> station_list;

class channel_info{
    private:
        string RADIO;
        int MULTI_PORT;
        int INFO_PORT;
        int BUF_SIZE;
        string video_filename;
        string duration_filename;
};


int get_free_number(){
    for(auto it:v){
        if(it.second) return it.first;
    }
    return -1;
}


bool add_station(){
    
    int station_number = get_free_number();
    if(station_number!=-1){
        bool check = false;
        for(auto it:station_list) 
            if(it.station_number==station_number) {
                check=true;
                it.is_active = true; 
                break;
            }
            
        if(!check){
            string addr = "239.192.1." + to_string(station_number); //Why do we need new ip address for each station, can't we just assign it different port ?
            const char *address = addr.c_str();
            int data_port = 5000+station_number;
            int info_port = 6000+station_number;
            station new_station(station_number,"Station"+to_string(station_number),inet_addr(address),data_port,info_port,BIT_RATE1,true);
            station_list.push_back(new_station);
            v[station_number] = false;
        }
        
        return true;
    }
    return false;
}

void remove_station(station &station_obj){
    v[station_obj.station_number] = true;
    station_obj.is_active = false;
    int ind=0;
    for(auto i : station_list)
    {
        if(i.station_number==station_obj.station_number)
        {
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

void* fetch_stations(void* argasdsad){
    // for(auto it:station_list) cout << it.to_str() << "\n";

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

void add_remove_stations(){

}


int main(){
    initialize_map();

    add_station();
    add_station();

    pthread_t ptid;

    pthread_create(&ptid,NULL,fetch_stations,NULL);
    sleep(1);
    
    while(true)
    {
        
        cout << "Enter integer : ";
        int input;
        cin >> input;
        if(input==1)                    //Add Station
        {
            add_station();
            cout << "Station created\n";
        }
        else{                           //Remove Station
            cout << "enter station number : ";
            int opt;
            cin >> opt;
            remove_station(station_list[opt-1]);
            cout << "Station removed\n";
        }
    }

    pthread_join(ptid,NULL);    
    // pthread_exit(NULL);
}