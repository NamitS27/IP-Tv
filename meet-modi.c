
#include <arpa/inet.h>
#include <errno.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
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

#define MAX_CLIENTS 20
#define CLADDR_LEN 100

#define BUF_SIZE1 256
#define BUF_SIZE2 512
#define BUF_SIZE3 1024
#define BUF_SIZE4 2048
#define BUF_SIZE5 4096
#define BUF_SIZE6 10240

#define IF_NAME1 "ens33"
#define IF_NAME2 "wlan0"
#define IF_NAME3 "enp2s0"

#define BORDER_WIDTH 6

/*IMPORTANT DO NOT REMOVE file:// in the path, it needs to be a file url*/

#define input_video "file:///home/namit27/Documents/CNP/InternetTv-IPmulticast-Stream-radio/output.mp4"

int flag;
int pause_flag;
int curr_radio_channel;
/*------------------ STRUCTURES DEFINATIONS ----------------------------*/
typedef struct station_info_request {

    uint8_t type;
} station_req;

typedef struct station_list {

    uint8_t station_number;
    uint8_t station_name_size;
    char station_name[100];
    uint32_t multicast_address;
    uint16_t data_port;
    uint16_t info_port;
    uint32_t bit_rate;

} station_info;

typedef struct site_info {

    uint8_t type;
    uint8_t site_name_size;
    char site_name[100];
    uint8_t site_desc_size;
    char site_desc[100];
    uint8_t station_count;
    station_info station_list[5];

} Site_Info;

typedef struct song_info {

    uint8_t type;
    uint8_t song_name_size;
    char song_name[100];
    uint16_t remaining_time_in_sec;
    uint8_t next_song_name_size;
    char next_song_name[100];

} Song_info;
/*--------------------------------------------------------------------*/

struct args {

    uint32_t multicast_address;
    uint16_t data_port;
    uint32_t info_port;
    uint32_t BUF_SIZE;
};

struct vlc {};

/*------------------ GTK FUNCTION DECLARATIONS------------------------*/
void destroy(GtkWidget *widget, gpointer data);
void player_widget_on_realize(GtkWidget *widget, gpointer data);
// void on_open(void);
void open_media(const char *uri);
void play(void);
void pause_player(void);
void on_playpause(GtkWidget *widget, gpointer data);
void on_stop(GtkWidget *widget, gpointer data);
/*--------------------------------------------------------------------*/

/*------------------- GLOBAL INSTANCES -------------------------------*/
libvlc_media_player_t *media_player;
libvlc_instance_t *vlc_inst;
GtkWidget *playpause_button;

station_req station_info_req;
Site_Info slist;
/*--------------------------------------------------------------------*/

void delay(int milliseconds) {
    long pause;
    clock_t now, then;

    pause = milliseconds * (CLOCKS_PER_SEC / 1000);
    now = then = clock();
    while ((now - then) < pause)
        now = clock();
}

void destroy(GtkWidget *widget, gpointer data) { gtk_main_quit(); }

void player_widget_on_realize(GtkWidget *widget, gpointer data) {
    libvlc_media_player_set_xwindow(
        media_player, GDK_WINDOW_XID(gtk_widget_get_window(widget)));
}

void *on_open() {
    usleep(500000);
    // sleep(1);                                  /* DELAY TESTED : OK*/
    open_media(input_video);
}

void open_media(const char *uri) {

    libvlc_media_t *media;
    media = libvlc_media_new_location(vlc_inst, uri);
    libvlc_media_player_set_media(media_player, media);
    play();
    libvlc_media_release(media);
}

void on_stop(GtkWidget *widget, gpointer data) {
    pause_player();
    libvlc_media_player_stop(media_player);
}

void play(void) {
    libvlc_media_player_play(media_player);
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-pause");
}

void pause_player(void) {
    libvlc_media_player_pause(media_player);
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-play");
}

void *radio_channel(void *input) {
    flag = 0;
    usleep(500000);
    // sleep(1);                                  /* DELAY TESTED : OK*/
    int opt = 1;
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-pause");
    uint32_t multicast_address = ((struct args *)input)->multicast_address;
    uint16_t data_port = ((struct args *)input)->data_port;
    uint32_t info_port = ((struct args *)input)->info_port;
    uint32_t BUF_SIZE = ((struct args *)input)->BUF_SIZE;

    printf("multi-cast adrress : %d\n", multicast_address);
    printf("PORT : %d\n", data_port);
    printf("INFO PORT : %d\n", info_port);
    printf("BUFFER SIZE : %d\n", BUF_SIZE);

    /*----------------------    SOCKET MULTI-CAST   --------------------*/
    int multi_sockfd;
    struct sockaddr_in servaddr;
    char interface_name[100];
    struct ifreq ifr;
    char *mcast_addr;
    struct ip_mreq mcastjoin_req;      /* multicast join struct */
    struct sockaddr_in mcast_servaddr; /* multicast sender*/
    socklen_t mcast_servaddr_len;

    if ((multi_sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("receiver: socket");
        exit(1);
    }
    printf("HERE\n");

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(data_port);

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, IF_NAME1, sizeof(IF_NAME1) - 1);

    if (setsockopt(multi_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if ((setsockopt(multi_sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr,
                    sizeof(ifr))) < 0) {
        perror("receiver: setsockopt() error");
        close(multi_sockfd);
        exit(1);
    }

    if ((bind(multi_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) <
        0) {
        perror("receiver: bind()");
        exit(1);
    }

    mcastjoin_req.imr_multiaddr.s_addr = multicast_address;
    mcastjoin_req.imr_interface.s_addr = htonl(INADDR_ANY);

    if ((setsockopt(multi_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                    (void *)&mcastjoin_req, sizeof(mcastjoin_req))) < 0) {
        perror("mcast join receive: setsockopt()");
        exit(1);
    }
    /*-----------------------------------------------------------------*/

    /*----------------------    BUFFER DECLAARATIONS ------------------*/
    char buffer[BUF_SIZE];
    int recieve_size;
    /*-----------------------------------------------------------------*/

    /*----------------------    FILE DECLARATIONS   -------------------*/
    FILE *mediaFile;
    char outputarray[BUF_SIZE5], verify = '1';
    mediaFile = fopen("output.mp4", "w");

    if (mediaFile == NULL) {
        printf("Error has occurred. Image file could not be opened\n");
        exit(1);
    }

    //  on_open();

    pthread_t vlc;
    pthread_create(&vlc, NULL, on_open, NULL);

    /*-----------------------------------------------------------------*/
    printf("\nReady to listen!\n\n");
    flag = 1;
    pause_flag = 0;

    while (flag) {
        if (pause_flag == 0) {
            memset(&mcast_servaddr, 0, sizeof(mcast_servaddr));
            mcast_servaddr_len = sizeof(mcast_servaddr);
            memset(buffer, '\0', BUF_SIZE);
            delay(1);
            memset(buffer, 0, sizeof(buffer));
            if ((recieve_size = recvfrom(multi_sockfd, buffer, BUF_SIZE, 0,
                                         (struct sockaddr *)&mcast_servaddr,
                                         &mcast_servaddr_len)) < 0) {
                perror("receiver: recvfrom()");
                exit(TRUE);
            }
            fwrite(buffer, 1, recieve_size, mediaFile);
            // fputs(buffer,stdout);

            if (recieve_size < BUF_SIZE) {
                flag = 0;
            }
        }
    }
    fclose(mediaFile);
    close(multi_sockfd);
    printf("Successfully reieved!!");
}

void on_playpause(GtkWidget *widget, gpointer data) {

    if (libvlc_media_player_is_playing(media_player) == 1) {
        pause_flag = 1;

        /*FILE *fp = fopen("/home/meet/Project/output.mp4", "w");
        fclose(fp);
    */
        // flag = 0;
        pause_player();
    } else {
        pause_flag = 0;
        flag = 0;
        // play();
        pthread_t my_radio_channel;
        struct args *station = (struct args *)malloc(sizeof(struct args));

        station->multicast_address =
            slist.station_list[curr_radio_channel].multicast_address;
        station->data_port = slist.station_list[curr_radio_channel].data_port;
        station->info_port = slist.station_list[curr_radio_channel].info_port;
        station->BUF_SIZE = slist.station_list[curr_radio_channel].bit_rate;

        pthread_create(&my_radio_channel, NULL, radio_channel, (void *)station);
    }
}

void button_fetch_clicked(gpointer data) {

    /*----------------------- TCP SOCKET------------------------------*/
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

    if (connect(TCP_sockfd, (struct sockaddr *)&TCP_servaddr,
                sizeof(TCP_servaddr)) < 0) {
        printf("\nConnection Failed \n");
        exit(1);
    }

    /*--------------------------------------------------------------*/

    station_info_req.type = 1;
    send(TCP_sockfd, &station_info_req, sizeof(station_req), 0);
    TCP_readsize = read(TCP_sockfd, &slist, sizeof(Site_Info));

    char *radiolist[slist.station_count][3];
    char temp[33], temp1[33];
    printf("%d\n", slist.type);
    printf("%s\n", slist.station_list[0].station_name);

    printf("---------------%s-----------------\n", slist.site_name);
    printf("%s\n", slist.site_desc);

    gtk_clist_clear((GtkCList *)data);

    for (int i = 0; i < slist.station_count; i++) {
        snprintf(temp1, sizeof(temp1), "%d", i);
        printf("temp : %s\n", temp1);
        radiolist[i][0] = temp1;

        radiolist[i][1] = slist.station_list[i].station_name;
        snprintf(temp, sizeof(temp), "%d",
                 slist.station_list[i].multicast_address);
        printf("temp : %s\n", temp);
        radiolist[i][2] = temp;

        gtk_clist_append((GtkCList *)data, radiolist[i]);
    }

    close(TCP_sockfd);
    return;
}

void selection_made(GtkWidget *clist, gint row, gint column,
                    GdkEventButton *event, gpointer data) {
    // gtk_clist_get_text(GTK_CLIST(clist), row, column, &text);
    gchar *text;
    gtk_clist_get_text(GTK_CLIST(clist), row, 2, &text);
    g_print("IP address is %d\n\n", row);

    pthread_t my_radio_channel;
    struct args *station = (struct args *)malloc(sizeof(struct args));

    station->multicast_address = slist.station_list[row].multicast_address;
    station->data_port = slist.station_list[row].data_port;
    station->info_port = slist.station_list[row].info_port;
    station->BUF_SIZE = slist.station_list[row].bit_rate;

    curr_radio_channel = row;
    pthread_create(&my_radio_channel, NULL, radio_channel, (void *)station);
}

int main(int argc, gchar *argv[]) {

    GtkWidget *window;
    GtkWidget *vbox, *hbox;
    GtkWidget *scrolled_window, *clist;
    GtkWidget *button_fetch;
    GtkWidget *hbuttonbox;
    GtkWidget *player_widget;
    GtkWidget *stop_button;

    gchar *titles[3] = {"Channel No.", "Channel Name", "Channel Address"};

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(GTK_WIDGET(window), 1000, 600);

    gtk_window_set_title(GTK_WINDOW(window), "Internet Radio");
    gtk_signal_connect(GTK_OBJECT(window), "destroy",
                       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show(scrolled_window);

    clist = gtk_clist_new_with_titles(3, titles);

    gtk_clist_set_column_width(GTK_CLIST(clist), 0, 380);
    gtk_clist_set_column_width(GTK_CLIST(clist), 1, 380);
    gtk_clist_set_column_width(GTK_CLIST(clist), 2, 380);

    gtk_signal_connect(GTK_OBJECT(clist), "select_row",
                       GTK_SIGNAL_FUNC(selection_made), NULL);

    gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_OUT);

    gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
    gtk_widget_show(clist);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show(hbox);

    button_fetch = gtk_button_new_with_label("Fetch All Channels");

    gtk_box_pack_start(GTK_BOX(hbox), button_fetch, TRUE, TRUE, 0);

    gtk_signal_connect_object(GTK_OBJECT(button_fetch), "clicked",
                              GTK_SIGNAL_FUNC(button_fetch_clicked),
                              (gpointer)clist);

    gtk_widget_show(button_fetch);

    ///////////////////////////////////
    player_widget = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    playpause_button = gtk_button_new_with_label("gtk-media-play");
    gtk_button_set_use_stock(GTK_BUTTON(playpause_button), TRUE);
    stop_button = gtk_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
    g_signal_connect(playpause_button, "clicked", G_CALLBACK(on_playpause),
                     NULL);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop), NULL);
    hbuttonbox = gtk_hbutton_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), BORDER_WIDTH);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), playpause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, FALSE, 0);

    vlc_inst = libvlc_new(0, NULL);
    media_player = libvlc_media_player_new(vlc_inst);
    g_signal_connect(G_OBJECT(player_widget), "realize",
                     G_CALLBACK(player_widget_on_realize), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
