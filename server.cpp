//Server side program

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

#define TCP_PORT 8080       // Initial communication happens at this port
#define TCP_PORT2 8081      // Information about removed channels is communicated through this port 
#define BIT_RATE1 4096      // Transmission of all channel data happens at the rate of 4kbps
#define TCP_IP "127.0.0.1"


#define MAX_STATION_SIZE 30

// Prototype for a function as the rest are declared in relevent order

void station_remove_req(int station_removed);

typedef struct station_list         // Secondary DS for storing information about each channel/station
{
    int station_number;
    char station_name[255];
    int multicast_address;
    int data_port;
    int info_port;
    int bit_rate;
    char video_filename[255];
    float video_duration;
} stats;

typedef struct information{         // structure carrying info about all channels for initial stage
    int size;
    stats data[MAX_STATION_SIZE];    
} inf;

inf parse_input();

using namespace std;

class station{                  // main class having info about each station/channel
    public:
        int station_number;
        string station_name;
        int multicast_address;
        int data_port;
        int info_port;
        int bit_rate;
        string video_filename;
        int video_duration;

    public:
        station(int station_number,string station_name, int multicast_address, int data_port,int info_port,int bit_rate) { 
        this->station_number=station_number;
        this->station_name=station_name;
        this->multicast_address=multicast_address;
        this->data_port=data_port;    
        this->info_port=info_port;
        this->bit_rate=bit_rate;
        this->video_duration = get_file_duration(station_number,station_name,bit_rate);
        this->video_filename = "videos/"+station_name+".mp4";

    }

    string to_str(){
        return to_string(this->station_number) + "|" + this->station_name + "|" + to_string(this->multicast_address) + "|" + to_string(this->data_port) + "|" + to_string(this->info_port) + "|" + to_string(this->bit_rate) ;//+ "|" + to_string(this->video_duration) + "|" + this->video_filename;
    }
    
    // calculates duration of each video file
    int get_file_duration(int station_number,string station_name,int bit_rate){

        string station_name_conv = station_name;

        string sys_call = "ffmpeg -i videos/" + station_name_conv + ".mp4 2>&1 | grep Duration | cut -d ' ' -f 4 | sed s/,// > duration" + to_string(station_number) + ".txt";
        system(sys_call.c_str());

        FILE *media_file;
        FILE *duration_file;

        string file_name = "videos/" + station_name + ".mp4";
        string duration_file_name = "duration" + to_string(station_number) + ".txt";
        
        media_file = fopen(file_name.c_str(),"re");
        duration_file = fopen(duration_file_name.c_str(),"re");
        
        if(media_file==NULL)
        {
            printf("Could not open mediafile!!");
            exit(1);
        }
        if(duration_file==NULL)
        {
            printf("Could not open durationfile!!");
            exit(1);
        }

        fseek(media_file,0,SEEK_END);
        int file_size = ftell(media_file); // end of file contains size of file
        
        // converting time into required format
        char duration[100] = {0};
        fread(duration,1,100,duration_file);
        int hour=0,min=0,sec=0;
        sscanf(duration,"%d:%d:%d",&hour,&min,&sec);
        // cout << hour << ":" << min << ":" << sec << "\n\n";
        sys_call = "rm " + duration_file_name;
        
        fclose(media_file);
        fclose(duration_file);
        int total_duration = ((file_size)/(3600*hour + 60*min + sec))/bit_rate;
        system(sys_call.c_str()); 
        return total_duration;
    }

};

map<int,bool> v;
vector<station> station_vec;

/* Initialising the map in order to check for the free stations */
void initialize_map(){
    for(int i=1;i<=255;i++){
        v[i]=true;
    }
}

// gets the first unused station number at a given instant
int get_free_number(){
    for(auto it:v){
        if(it.second) return it.first;
    }
    return -1;
}

// adds channel to the channel pool
int add_station(){
    
    int station_number = get_free_number();
    if(station_number!=-1){
        string addr = "127.0.0.1";
        const char *address = addr.c_str();
        int data_port = 7000+station_number;
        int info_port = 8010+station_number;
        station new_station(station_number,"station"+to_string(station_number),inet_addr(address),data_port,info_port,BIT_RATE1);
        /* Adding new station to the channel pool */
        station_vec.push_back(new_station);
        v[station_number] = false;
        return station_number;
    }
    return -1;
}

// removes channel from the channel pool
void remove_station(int station_number){
    v[station_number] = true;
    int ind=0;
    
    for(auto ind_station : station_vec) {
        if(ind_station.station_number==station_number) {
            station_vec.erase(station_vec.begin()+ind);
            break;
        }
        ind++;
    }
    station_remove_req(station_number);
}

// communicates removal of channel to the clients for updating info
void station_remove_req(int station_removed)
{
    int TCP_sockfd = 0, TCP_read;
    struct sockaddr_in TCP_servaddr;
    // Socket is created here
    if ((TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        exit(1);
    }
    printf("Socket Created!\n");

    if (inet_pton(AF_INET, "127.0.0.1", &TCP_servaddr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        exit(1);
    }

    /* Initialising with the required veriables & their values */
    TCP_servaddr.sin_family = AF_INET;
    TCP_servaddr.sin_addr.s_addr = INADDR_ANY;
    TCP_servaddr.sin_port = htons(TCP_PORT2);  // using port 2 to send special info

    // request to connect with clients
    if (connect(TCP_sockfd, (struct sockaddr *)&TCP_servaddr, sizeof(TCP_servaddr)) < 0)
    {
        printf("\nConnection Failed \n");
        exit(1);
    }
    /* Sending the station number which is needed to be deleted */
    send(TCP_sockfd, &station_removed, sizeof(int),0);
    close(TCP_sockfd);
}

// This function is used to send details about the available channels over TCP
void* fetch_stations(void* input){
    // All the necessary declarations
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // The socket is created here
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
    // port 1 for sending initial total channel info
    address.sin_port = htons(TCP_PORT);

    // Socket is binded here
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // Now we wait for a connection. MAX CONNECTIONS = 3
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        /* forming the list of the stations */
        inf input_data = parse_input();
        /* sending the station list along with their respective data */
        send(new_socket, &input_data, sizeof(inf), 0);
    }
    
}

// This function is a threaded function used to send video over UDP
void* send_data(void* input){
    int multiport,infoport,duration,station_number,multicast_address,bit_rate;
    string videofilename,station_name;
    
    multiport = ((station*)input)->data_port;
    infoport = ((station*)input)->info_port;
    videofilename = ((station*)input)->video_filename;
    duration = ((station*)input)->video_duration;
    station_name = ((station*)input)->station_name;
    station_number = ((station*)input)->station_number;
    multicast_address = ((station*)input)->multicast_address;
    bit_rate = ((station*)input)->bit_rate;
    

    clock_t start, mid, end;
    double execTime;


    /*----------------------    SOCKET MULTI-CAST   --------------------*/
    int multi_sockfd;
    struct sockaddr_in servaddr;

   // Initializing the UDP socket for the transmission
    if ((multi_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("server : socket");
        exit(1);
    }

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = multicast_address;
    servaddr.sin_port = htons(multiport);
    
    char buffer[bit_rate];
    int send_status;
    
    FILE *mediaFile;
    int filesize, packet_index, read_size, total_sent;
    packet_index = 1;
    
    mediaFile = fopen(videofilename.c_str(), "re");
    execTime = 0;
    start = clock();
    while (!feof(mediaFile)) {

        read_size = fread(buffer, 1, bit_rate, mediaFile);
        total_sent += read_size;
        // printf("Packet Size: = %d\n", read_size);

        if ((send_status = sendto(multi_sockfd, buffer, sizeof(buffer), 0,(struct sockaddr *)&servaddr,sizeof(servaddr))) == -1) {
            perror("sender: sendto");
            exit(1);
        }
        // printf("%d : Packet Number: %i Multicast_address : %d\n", multiport, packet_index,multicast_address);
        
        packet_index++;
        if (packet_index % duration == 0) {
            mid = clock();
            execTime = ((double)(mid - start)) / CLOCKS_PER_SEC;
            execTime = (0.9 - execTime);
            usleep((int)(execTime * 1000000));
        
            start = clock();
        }
    }

    // Sending a end frame such that client gets an idea of the end of the file.
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer,"end");
    sendto(multi_sockfd, buffer, sizeof(buffer), 0,(struct sockaddr *)&servaddr,sizeof(servaddr));
    memset(buffer, 0, sizeof(buffer));
    close(multi_sockfd); // close the socket
    return NULL;
}

// parsing object data into structure suitable for transmission in socket
inf parse_input(){
    int data_size = station_vec.size();
    inf input_data;
    input_data.size = data_size;
    stats temp_struc;
    
    /* Forming the structure (the format required to send to the client) */
    for (int i = 0; i < data_size; i++){

        temp_struc.bit_rate = station_vec[i].bit_rate;
        temp_struc.data_port = station_vec[i].data_port;
        strcpy(temp_struc.video_filename,station_vec[i].video_filename.c_str());
        temp_struc.video_duration = station_vec[i].video_duration;
        temp_struc.info_port = station_vec[i].info_port;
        temp_struc.multicast_address = station_vec[i].multicast_address;
        strcpy(temp_struc.station_name,station_vec[i].station_name.c_str());
        temp_struc.station_number = station_vec[i].station_number;
        input_data.data[i] = temp_struc;
    }
    
    return input_data;
}

int main(int argc, char *argv[]){
    
    initialize_map(); // initialising the map which helps in finding the station which are not yet used

    pthread_t ptid;

    pthread_create(&ptid,NULL,fetch_stations,NULL); // thread for sending the station details
    sleep(0.5);
    
    while(true)
    {
        cout << "\n1. Add Station \n2. Remove Station\n3. Exit\n\nEnter your choice: ";
        int input;
        cin >> input;   
        if(input==1){    // Add a new station                
            int stat_number = add_station();
            stat_number > 0 ? cout << "Station Added Successfully\n" : cout << "Cannot add the station!\n"; // check if the station is added successfully or not.
            int ind = -1;
            for(int i=0;i<station_vec.size();i++){
                if(stat_number==station_vec[i].station_number){
                    ind = i;
                    break;
                } 
            }
            // cout << ind << endl;
            
            // start the video transmission of the respective station
            pthread_t udpid;
            pthread_create(&udpid,NULL,send_data,(void *)&station_vec[ind]);
            sleep(1);
        }
        else if(input==2) {                           
            cout << "Enter station number for removing: ";
            int remove_station_number;
            cin >> remove_station_number;
            remove_station(remove_station_number); // remove the station as per the station number provided by the user
            cout << "Station removed\n";
        } 
        else exit(1);
    }

    pthread_exit(NULL);    
}