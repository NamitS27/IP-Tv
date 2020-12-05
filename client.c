#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
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
#define IF_NAME "enp1s0"
#define MAX_STATION_SIZE 300
#define BORDER_WIDTH 6

void destroy(GtkWidget *widget, gpointer data);
void player_widget_on_realize(GtkWidget *widget, gpointer data);
void open_media(const char *uri);
void play(void);
void pause_player(void);
void on_playpause(GtkWidget *widget, gpointer data);
void on_stop(GtkWidget *widget, gpointer data);

libvlc_media_player_t *media_player;
libvlc_instance_t *vlc_inst;
GtkWidget *playpause_button;


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
int current_radio_channel;
int flag;
int pause_flag;

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
    usleep(5000000);
    open_media("file:///media/mrj35/Data/Academics/ICT-Sem5/CN_Lab/Project/Internet-Tv/output.mp4");
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

void *play_channel(void *input){
    usleep(500000);
    int opt = 1;
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-pause");
    
    int multicast_address = ((struct station_list *)input)->multicast_address;
    int data_port = ((struct station_list *)input)->data_port;
    int info_port = ((struct station_list *)input)->info_port;
    int bit_rate = ((struct station_list *)input)->bit_rate;

    printf("Multi-cast adrress : %d\n", multicast_address);
    printf("Data Port : %d\n", data_port);
    printf("Info Port : %d\n", info_port);
    printf("Bit Rate : %d\n", bit_rate);

    // /*----------------------    SOCKET MULTI-CAST   --------------------*/
    int multi_sockfd;
    struct sockaddr_in servaddr;
    char interface_name[100];
    struct ifreq ifr;
    char *mcast_addr;
    struct ip_mreq mcastjoin_req;      /* multicast join struct */
    struct sockaddr_in mcast_servaddr; /* multicast sender*/
    socklen_t mcast_servaddr_len;

    if ((multi_sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("receiver: socket");
        exit(1);
    }
    printf("Socket Created\n");

    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(data_port);
    // printf("PORT no : %d\n",data_port);
    if(bind(multi_sockfd,(struct sockaddr*) &servaddr,sizeof(servaddr))<0)
    {
        perror("Receiver: bind()");
        // exit(1);
    }

    mcastjoin_req.imr_multiaddr.s_addr = multicast_address;
    mcastjoin_req.imr_interface.s_addr = htonl(INADDR_ANY);

    if(setsockopt(multi_sockfd,IPPROTO_IP,IP_ADD_MEMBERSHIP,(void *)&mcastjoin_req,sizeof(mcastjoin_req))<0)
    {
        perror("Receiver: setsockopt()");
        // exit(1);
    }

    // memset(&ifr, 0, sizeof(ifr));
    // strncpy(ifr.ifr_name, IF_NAME, sizeof(IF_NAME) - 1);

    // if (setsockopt(multi_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
    //                sizeof(opt))) {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }

    // int ret;
    // if ((ret = setsockopt(multi_sockfd, SOL_SOCKET, SO_BINDTODEVICE, (void *)&ifr,
    //                 sizeof(ifr))) < 0) {
    //     perror("Receiver: setsockopt() error");
    //     printf("RET : %d\n",ret);
    //     close(multi_sockfd);
    //     exit(1);
    // }

    // if ((bind(multi_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) <0) {
    //     perror("Receiver: bind()");
    //     exit(1);
    // }

    // mcastjoin_req.imr_multiaddr.s_addr = multicast_address;
    // mcastjoin_req.imr_interface.s_addr = htonl(INADDR_ANY);
    
    // if ((ret = setsockopt(multi_sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
    //                 (void *)&mcastjoin_req, sizeof(mcastjoin_req))) < 0) {
    //     perror("mcast join receive: setsockopt()");
        
    //     exit(1);
    // }
    // /*-----------------------------------------------------------------*/

    // /*----------------------    BUFFER DECLAARATIONS ------------------*/
    char buffer[bit_rate];
    int recieve_size;
    // /*-----------------------------------------------------------------*/

    // /*----------------------    FILE DECLARATIONS   -------------------*/
    FILE *mediaFile;
    char outputarray[bit_rate];
    mediaFile = fopen("output.mp4", "w");

    if (mediaFile == NULL) {
        printf("Error has occurred. Image file could not be opened\n");
        exit(1);
    }

    pthread_t vlc;
    pthread_create(&vlc, NULL, on_open, NULL);

    /*-----------------------------------------------------------------*/
    printf("\nReady to listen!\n\n");
    flag = 1;
    pause_flag = 0;

    while (flag) {
    //     printf("Hello\n");
        if (pause_flag == 0) {
    //         printf("Here\n");
            memset(&mcast_servaddr, 0, sizeof(mcast_servaddr));
            mcast_servaddr_len = sizeof(mcast_servaddr);
            // memset(buffer, '\0', bit_rate);
            // delay(1);
            memset(buffer, 0, sizeof(buffer));
            if ((recieve_size = recvfrom(multi_sockfd, buffer, bit_rate, 0, NULL,0)) < 0) {
                perror("receiver: recvfrom()");
                // exit(TRUE);
            }
            printf("DATA: %d\n",recieve_size);
            fwrite(buffer, 1, recieve_size, mediaFile);
    //         // printf("DATA: %s\n",buffer);
            if (recieve_size < bit_rate) {
                flag = 0;
            }
        }
        // flag = 0;
    }
    fclose(mediaFile);
    close(multi_sockfd);
    printf("Successfully Received Channel Data!!");
}

void get_station_list(gpointer data){
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
    
    if((TCP_read = read(TCP_sockfd, &infos, sizeof(station_information))) < 0){
        perror("Failed to read");
        exit(-1);
    }
    
    gtk_clist_clear((GtkCList *)data); 

    char *send_data[infos.size][2];
    
    for (int i = 0; i < infos.size; i++){
        
        printf("--------------------------------------------------\n");
        printf("Station number : %d\n",infos.data[i].station_number);
        printf("Station name : %s\n", infos.data[i].station_name);
        printf("Multicast_address : %d\n",infos.data[i].multicast_address);
        printf("Video filename : %s\n",infos.data[i].video_filename);
        printf("--------------------------------------------------\n");
        
        char sta_num[10];
        snprintf(sta_num, sizeof(sta_num), "%d", infos.data[i].station_number);      
        char sta_add[10];
        snprintf(sta_add, sizeof(sta_add), "%d", infos.data[i].multicast_address);
        // char *sta_name = &infos.data[i].station_name[0];
        
        send_data[i][0] = sta_num;
        send_data[i][1] = infos.data[i].station_name;
        // send_data[i][2] = sta_add;
        
        gtk_clist_append((GtkCList *)data, send_data[i]);
    }
    
    close(TCP_sockfd);

    for (int i = 0; i < infos.size; i++)
    {
        printf("%d) %s\n",infos.data[i].station_number,infos.data[i].station_name);
    }
    printf("-----------------------------------------------------------------------\n");
    // printf("Select your channel : ");
    // int channel_number;
    // scanf("%d",&channel_number);
}

void choose_station(GtkWidget *clist, gint row, gint column,
                    GdkEventButton *event, gpointer data) {
    // gtk_clist_get_text(GTK_CLIST(clist), row, column, &text);
    // gchar *text;
    // gtk_clist_get_text(GTK_CLIST(clist), row, 2, &text);
    // g_print("IP address is %d\n\n", row);

    pthread_t my_radio_channel;
    // struct args *station = (struct args *)malloc(sizeof(struct args));
    struct station_list *ind_station = (struct station_list*)malloc(sizeof(struct station_list));


    ind_station->multicast_address = infos.data[row].multicast_address;
    ind_station->data_port = infos.data[row].data_port;
    ind_station->info_port = infos.data[row].info_port;
    strcpy(ind_station->station_name,infos.data[row].station_name);
    ind_station->station_number = infos.data[row].station_number;
    ind_station->bit_rate = infos.data[row].bit_rate;


    current_radio_channel = row;
    printf("---------------------------------------------------\n");
    printf("Station name : %s",ind_station->station_name);
    printf("Station name : %d",ind_station->station_number);
    printf("Data port : %d",ind_station->data_port);
    printf("Info port : %d",ind_station->info_port);
    printf("Bit rate : %d",ind_station->bit_rate);
    printf("Multicast address : %d",ind_station->multicast_address);
    printf("-----------------------------------------------------\n");
    pthread_create(&my_radio_channel, NULL, play_channel, (void *)ind_station);
}

void on_playpause(GtkWidget *widget, gpointer data) {
    if (libvlc_media_player_is_playing(media_player) == 1) {
        pause_flag = 1;
        pause_player();
    } else {
        pause_flag = 0;
        flag = 0;
        pthread_t my_radio_channel;
        struct station_list *station = (struct station_list *)malloc(sizeof(struct station_list));
        station->multicast_address = infos.data[current_radio_channel].multicast_address;
        station->data_port = infos.data[current_radio_channel].data_port;
        station->info_port = infos.data[current_radio_channel].info_port;
        station->bit_rate = infos.data[current_radio_channel].bit_rate;
        pthread_create(&my_radio_channel, NULL, play_channel, (void *)station);
    }
}



int main(int argc, gchar *argv[]){
    GtkWidget *window;
    GtkWidget *vbox, *hbox;
    GtkWidget *scrolled_window, *clist;
    GtkWidget *button_fetch;
    GtkWidget *hbuttonbox;
    GtkWidget *player_widget;
    GtkWidget *stop_button;

    gchar *titles[2] = {"Channel No.", "Channel Name"};

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(GTK_WIDGET(window), 1000, 600);

    gtk_window_set_title(GTK_WINDOW(window), "Internet TV");
    gtk_signal_connect(GTK_OBJECT(window), "destroy",
                       GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    // gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled_window),700);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show(scrolled_window);

    clist = gtk_clist_new_with_titles(2, titles);

    gtk_clist_set_column_width(GTK_CLIST(clist), 0, 450);
    gtk_clist_set_column_width(GTK_CLIST(clist), 1, 450);
    // gtk_clist_set_column_width(GTK_CLIST(clist), 2, 380);

    // gtk_clist_set_row_height(GTK_CLIST(clist),100);

    gtk_signal_connect(GTK_OBJECT(clist), "select_row",GTK_SIGNAL_FUNC(choose_station), NULL);

    gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_OUT);

    gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
    // get_station_list((gpointer)clist);
    gtk_widget_show(clist);
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show(hbox);

    button_fetch = gtk_button_new_with_label("Fetch All Channels");

    gtk_box_pack_start(GTK_BOX(hbox), button_fetch, TRUE, TRUE, 10);

    gtk_signal_connect_object(GTK_OBJECT(button_fetch), "clicked",
                              GTK_SIGNAL_FUNC(get_station_list),
                              (gpointer)clist);

    GdkColor button_color;
    gdk_color_parse("red",&button_color);
    gtk_widget_modify_bg(GTK_WIDGET(button_fetch),GTK_STATE_NORMAL,&button_color);

    gtk_widget_show(button_fetch);
    
    
    player_widget = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    // playpause_button = gtk_button_new_with_label("gtk-media-play");
    // gtk_button_set_use_stock(GTK_BUTTON(playpause_button), TRUE);
    stop_button = gtk_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
    // g_signal_connect(playpause_button, "clicked", G_CALLBACK(on_playpause),NULL);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop), NULL);
    hbuttonbox = gtk_hbutton_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), BORDER_WIDTH);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);
    // gtk_box_pack_start(GTK_BOX(hbuttonbox), playpause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, FALSE, 0);

    vlc_inst = libvlc_new(0, NULL);
    media_player = libvlc_media_player_new(vlc_inst);
    g_signal_connect(G_OBJECT(player_widget), "realize",G_CALLBACK(player_widget_on_realize), NULL);


    GdkColor color;
    gdk_color_parse("gray",&color);
    gtk_widget_modify_bg(GTK_WIDGET(window),GTK_STATE_NORMAL,&color);

    gtk_widget_show_all(window);

    gtk_main();
    
    return 0;
}