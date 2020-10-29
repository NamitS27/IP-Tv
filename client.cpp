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

    public:
        station(char *station_number,char *station_name, char *multicast_address, char *data_port,char *info_port,char *bit_rate) { // Constructor with parameters
            this->station_number=atoi(station_number);
            this->station_name=station_name;
            this->multicast_address=atoi(multicast_address);
            this->data_port=atoi(data_port);    
            this->info_port=atoi(info_port);
            this->bit_rate=atoi(bit_rate);
        }

        station(vector<string> astation){
            this->station_number=stoi(astation[0]);
            this->station_name=astation[1];
            this->multicast_address=stoi(astation[2]);
            this->data_port=stoi(astation[3]);    
            this->info_port=stoi(astation[4]);
            this->bit_rate=stoi(astation[5]);
        }

    string to_str(){
        return to_string(this->station_number) + "|" + this->station_name + "|" + to_string(this->multicast_address) + "|" + to_string(this->data_port) + "|" + to_string(this->info_port) + "|" + to_string(this->bit_rate);
    }
    
};

// bool comparator(const station& lhs,const station& rhs){
//     return lhs.station_number < rhs.station_number;
// }

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

    int TCP_sockfd = 0, TCP_read;
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
    char buf[BUFF_SIZE];
    string receivedString;

    if((TCP_read = read(TCP_sockfd, &buf, sizeof(buf))) < 0){
        perror("Failed to read");
        exit(-1);
    }

    receivedString = buf;

    stringstream ss(buf);
    string stationList,stations;

    vector <station> slist;

    while (getline(ss, stationList, '&')){

        stringstream stationListStream(stationList);

        vector <string> station_attributes;

        while(getline(stationListStream, stations, '|')){
            station_attributes.push_back(stations);
        }
        station new_station(station_attributes);
        slist.push_back(new_station);

    }

    sort(slist.begin(),slist.end(),[](const station& lhs,const station& rhs){
        return lhs.station_number < rhs.station_number;
    });
    for(auto itr:slist) cout << itr.to_str() << "\n";

}