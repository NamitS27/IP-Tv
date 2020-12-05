#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
// #include <gdk/gdkx.h>
// #include <gtk/gtk.h>
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
#include <netdb.h>

#define TRUE 1

#define TCP_PORT 8080
#define TCP_IP "127.0.0.1"
#define BUFF_SIZE 10000
#define IF_NAME "enp1s0"
#define MAX_STATION_SIZE 300
#define BORDER_WIDTH 6

// void destroy(GtkWidget *widget, gpointer data);
// void player_widget_on_realize(GtkWidget *widget, gpointer data);
// void open_media(const char *uri);
// void play(void);
// void pause_player(void);
// void on_playpause(GtkWidget *widget, gpointer data);
// void on_stop(GtkWidget *widget, gpointer data);

// libvlc_media_player_t *media_player;
// libvlc_instance_t *vlc_inst;
// GtkWidget *playpause_button;

typedef struct station_list
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

typedef struct information
{
    int size;
    stats data[MAX_STATION_SIZE];
} station_information;

station_information infos;
int current_radio_channel;
int flag;
int pause_flag;

void delay(int milliseconds)
{
    long pause;
    clock_t now, then;

    pause = milliseconds * (CLOCKS_PER_SEC / 1000);
    now = then = clock();
    while ((now - then) < pause)
        now = clock();
}

int main(int argc)
{

    // usleep(500000);
    int opt = 1;
    // gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-pause");

    int multicast_address = inet_addr("219.192.1.1");
    int data_port = 7001;
    int info_port = 8011;
    int bit_rate = 4096;

    printf("Multi-cast adrress : %d\n", multicast_address);
    printf("Data Port : %d\n", data_port);
    printf("Info Port : %d\n", info_port);
    printf("Bit Rate : %d\n", bit_rate);

    /*----------------------    SOCKET MULTI-CAST   --------------------*/
    int multi_sockfd;
    struct sockaddr_in servaddr;
    char interface_name[100];
    struct ifreq ifr;
    char *mcast_addr;
    struct ip_mreq mcastjoin_req;      /* multicast join struct */
    struct sockaddr_in mcast_servaddr; /* multicast sender*/
    socklen_t mcast_servaddr_len;

    if ((multi_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("receiver: socket");
        exit(1);
    }
    printf("Socket Created\n");

    struct hostent *server = gethostbyname("127.0.0.1");

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=INADDR_ANY;
    // bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr, server->h_length);
    servaddr.sin_port = htons(data_port);

    printf("PORT no : %d\n", data_port);

    memset(&ifr, 0, sizeof(ifr));
    printf("interface name : %ld\n", sizeof(IF_NAME));
    strncpy(ifr.ifr_name, IF_NAME, sizeof(IF_NAME) - 1);

    if (setsockopt(multi_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    printf("Here 1\n");

    int ret;
    // if ((ret = setsockopt(multi_sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr,
    //                 sizeof(ifr))) < 0) {
    //     perror("Receiver: setsockopt() error");
    //     printf("RET : %d\n",ret);
    //     close(multi_sockfd);
    //     exit(1);
    // }
    // printf("Here 2\n");

    if ((bind(multi_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
    {
        perror("Receiver: bind()");
        exit(1);
    }
    printf("Here 3\n");


    // bcopy((char *)server->h_addr, (char *)&mcastjoin_req.imr_multiaddr.s_addr, server->h_length);

    mcastjoin_req.imr_multiaddr.s_addr = multicast_address;
    mcastjoin_req.imr_interface.s_addr = inet_addr("127.0.0.1");
    printf("Before mcast join receive\n");
    // if ((ret = setsockopt(multi_sockfd, IPPROTO_IP, IP_MULTICAST_IF, (void *)&mcastjoin_req, sizeof(mcastjoin_req))) < 0)
    // {
    //     perror("mcast join receive: setsockopt()");
    //     exit(1);
    // }

    if ((ret = setsockopt(multi_sockfd, IPPROTO_IP, IP_MULTICAST_IF, (void *)&mcastjoin_req, sizeof(mcastjoin_req))) < 0)
    {
        perror("mcast join receive: setsockopt()");
        exit(1);
    }

    /*-----------------------------------------------------------------*/

    /*----------------------    BUFFER DECLAARATIONS ------------------*/
    char buffer[bit_rate];
    int recieve_size;
    /*-----------------------------------------------------------------*/

    /*----------------------    FILE DECLARATIONS   -------------------*/
    FILE *mediaFile;
    char outputarray[bit_rate];
    mediaFile = fopen("output.mp4", "w");

    if (mediaFile == NULL)
    {
        printf("Error has occurred. Image file could not be opened\n");
        exit(1);
    }

    // pthread_t vlc;
    // pthread_create(&vlc, NULL, on_open, NULL);

    /*-----------------------------------------------------------------*/
    printf("\nReady to listen!\n\n");
    flag = 1;
    pause_flag = 0;

    while (flag)
    {
        printf("Hello\n");
        if (pause_flag == 0)
        {
            printf("Here1\n");
            // memset(&mcast_servaddr, 0, sizeof(mcast_servaddr));
            // mcast_servaddr_len = sizeof(mcast_servaddr);
            // memset(buffer, '\0', bit_rate);
            // delay(1);
            memset(buffer, 0, sizeof(buffer));
            printf("Here2\n");
            recvfrom(multi_sockfd, buffer, bit_rate, 0, (struct sockaddr *)&mcastjoin_req, sizeof(mcastjoin_req));
            // {
            //     perror("receiver: recvfrom()");
            //     exit(TRUE);
            // }
            // printf("DATA: %d\n", recieve_size);
            fwrite(buffer, 1, 4096, mediaFile);
            // printf("DATA: %s\n",buffer);
            if (4096 < bit_rate)
            {
                flag = 0;
            }
        }
        flag = 0;
    }
    fclose(mediaFile);
    close(multi_sockfd);
    printf("Successfully Received Channel Data!!");

    return 0;
}