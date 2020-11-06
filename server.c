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

#define TCP_IP "0.0.0.0"
#define BUFF_SIZE 10000

typedef struct station_list{
    uint8_t station_number;
    char station_name[10];
    uint32_t multicast_address;
    uint16_t data_port;
    uint16_t info_port;
    uint32_t bit_rate;
} stats;

int arr[255];

void initailize(){
    memset(arr,1,sizeof(arr));
}

int get_free_number(){
    for(int i=0;i<255;i++){
        if(arr[i]) return i;
    }
    return -1;
}

stats stations[255];

void initialize_stats(){
    memset(stations,NULL,sizeof(stations));
}






int main(int argc, char const *argv[])
{
    initialize_stats();
    printf("%s",stations[0].station_name);
    return 0;
}
