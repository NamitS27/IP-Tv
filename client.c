#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vlc/vlc.h>

#define TCP_PORT 8080
#define TCP_IP "127.0.0.1"
#define BUFF_SIZE 10000

#define MAX_STATION_SIZE 300

typedef struct station_list{
    int station_number;
    char station_name[255];
    int multicast_address;
    int data_port;
    int info_port;
    int bit_rate;
    char video_filename[255];
    float video_duration;
} stats;

typedef struct information{
    int size;
    stats data[MAX_STATION_SIZE];    
} station_information;

station_information infos;

int main(){
    printf("What are you doing step brother 0W0 ?");
    int TCP_sockfd = 0, TCP_read;
    struct sockaddr_in TCP_servaddr;
    if ((TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        exit(1);
    }
    printf("Socket Created!\n");

    if (inet_pton(AF_INET, TCP_IP, &TCP_servaddr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        exit(1);
    }

    printf("IP Checked!!\n");

    TCP_servaddr.sin_family = AF_INET;
    TCP_servaddr.sin_port = htons(TCP_PORT);

    if (connect(TCP_sockfd, (struct sockaddr *)&TCP_servaddr, sizeof(TCP_servaddr)) < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }

    // char buf[BUFF_SIZE];

    
    if((TCP_read = read(TCP_sockfd, &infos, sizeof(station_information))) < 0){
        perror("Failed to read");
        exit(-1);
    }
    
    printf("%d\n",infos.size);
    for(int i=0;i<infos.size;i++){
        printf("%s\n", infos.data[i].station_name);
        printf("%d\n", infos.data[i].station_number);
    }

    // printf("Hey now brown cow\n");

    /*
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
    */
   return 0;

}