#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vlc/vlc.h>
#include <bits/stdc++.h>

#define TCP_PORT 8080

#define TCP_IP "0.0.0.0"
#define BUFF_SIZE 10000

using namespace std;

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
        station(char *station_number,char *station_name, char *multicast_address, char *data_port,char *info_port,char *bit_rate, char *is_active) { // Constructor with parameters
            this->station_number=atoi(station_number);
            this->station_name=station_name;
            this->multicast_address=atoi(multicast_address);
            this->data_port=atoi(data_port);    
            this->info_port=atoi(info_port);
            this->bit_rate=atoi(bit_rate);
            if(is_active=="1"){
                this->is_active = true;
            }else{
                this->is_active = false;
            }
        }

        station(vector<string> v1){
            this->station_number=stoi(v1[0]);
            this->station_name=v1[1];
            this->multicast_address=stoi(v1[2]);
            this->data_port=stoi(v1[3]);    
            this->info_port=stoi(v1[4]);
            this->bit_rate=stoi(v1[5]);
            this->is_active = stoi(v1[6]);
        }
    
};

class channel_info{
    private:
        string RADIO;
        int MULTI_PORT;
        int INFO_PORT;
        int BUF_SIZE;
        string video_filename;
        string duration_filename;
};

int main(){

    int TCP_sockfd = 0, TCP_readsize;
    struct sockaddr_in TCP_servaddr;

    if ((TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(1);
    }

    if (inet_pton(AF_INET, TCP_IP, &TCP_servaddr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(1);
    }

    TCP_servaddr.sin_family = AF_INET;
    TCP_servaddr.sin_port = htons(TCP_PORT);

    if (connect(TCP_sockfd, (struct sockaddr *)&TCP_servaddr, sizeof(TCP_servaddr)) < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }
    std::string s;

    int n;
    char buf[BUFF_SIZE]="";
    string receivedString;

    if((TCP_readsize = read(TCP_sockfd, &buf, sizeof(buf))) < 0){
        perror("Failed to read");
        exit(-1);
    }
    // else{

    receivedString=buf;

    // }

    stringstream ss(buf);
    string stationList,stations;

    vector <station> slist;

    while (getline(ss, stationList, '&')){

        stringstream stationListStream(stationList);

        vector <string> v1;

        while(getline(stationListStream, stations, '|')){
            cout << stations << endl;

            v1.push_back(stations);

        }
        station new_station(v1);
        slist.push_back(new_station);

    }

    // cout << "\n\n\n";

    // cout << slist[0].station_number << endl;
    // cout << slist[0].is_active << endl;
    // cout << slist[0].station_name << endl;
    // cout << slist[0].multicast_address << endl;


}