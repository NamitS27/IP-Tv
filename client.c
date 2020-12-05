// ---------------------- Headers ---------------------------------- //
/* Client */
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

// -------------------------- Declarations for TCP and UDP and GTK ----------------//
#define TCP_PORT 8080 // Initial communication happens at this port
#define TCP_PORT2 8081 // Information about removed channels is communicated through this port
#define TCP_IP "127.0.0.1"
#define MAX_STATION_SIZE 30
#define BORDER_WIDTH 6

// ------------------------ Function Prototypes -----------------------------------//
void destroy(GtkWidget *widget, gpointer data);
void player_widget_on_realize(GtkWidget *widget, gpointer data);
void open_media(const char *uri);
void play(void);
void pause_player(void);
void on_stop(GtkWidget *widget, gpointer data);

libvlc_media_player_t *media_player;
libvlc_instance_t *vlc_inst;
GtkWidget *playpause_button;

// ----------------- The struct used for information regarding channels which is send via TCP -------//
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

// ----------------- This is the actual stuct which is sent over TCP --------------//
typedef struct information{
    int size;
    stats data[MAX_STATION_SIZE];
} station_information;

station_information infos;
int current_radio_channel;
int flag;
int multi_sockfd[MAX_STATION_SIZE];
int recieving[MAX_STATION_SIZE];


/* GTK(GUI) related functions are declared below */

/* This function is used to destoy the gtk widget */
void destroy(GtkWidget *widget, gpointer data) { gtk_main_quit(); }

/* Used to set the window of GTK widget */
void player_widget_on_realize(GtkWidget *widget, gpointer data)
{
    libvlc_media_player_set_xwindow(media_player, GDK_WINDOW_XID(gtk_widget_get_window(widget)));
}

// Used to open files saved by each station/channel
void *on_open(void *input)
{
    usleep(5000000);  // sleep for 5 seconds
    /* Change the path as per your system */
    char path[] = "file:///home/namit27/Documents/Internet-Tv/";
    strcat(path, (char *)input);
    open_media(path);
}

// opens the video file using vlc package in the GTK window (given its path)
void open_media(const char *uri)
{
    libvlc_media_t *media;
    media = libvlc_media_new_location(vlc_inst, uri);
    libvlc_media_player_set_media(media_player, media);
    play();
    libvlc_media_release(media);
}

// stops the video file played using vlc
void on_stop(GtkWidget *widget, gpointer data)
{
    pause_player();
    libvlc_media_player_stop(media_player);
}

// plays the video file using vlc package
void play(void)
{
    libvlc_media_player_play(media_player);
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-pause");
}

/* Pauses the player */
void pause_player(void)
{
    libvlc_media_player_pause(media_player);
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-play");
}

/* This function is used to receive data over UDP and then signals the playback of the video */
void *play_channel(void *input)
{

    // variable declarations for receiving and playing channel specified by the user
    int opt = 1;
    int sn = ((struct station_list *)input)->station_number;
    recieving[((struct station_list *)input)->station_number] = 1;
    gtk_button_set_label(GTK_BUTTON(playpause_button), "gtk-media-pause");

    int multicast_address = ((struct station_list *)input)->multicast_address;
    int data_port = ((struct station_list *)input)->data_port;
    int info_port = ((struct station_list *)input)->info_port;
    int bit_rate = ((struct station_list *)input)->bit_rate;
    
    // printing info for debugging
    char fname[20];
    strncpy(fname, ((struct station_list *)input)->station_name, sizeof(((struct station_list *)input)->station_name));
    strcat(fname, ".mp4");
    printf("File : %s\n", fname);
    printf("Multi-cast adrress : %d\n", multicast_address);
    printf("Data Port : %d\n", data_port);
    printf("Info Port : %d\n", info_port);
    printf("Bit Rate : %d\n", bit_rate);

    /*----------------------    SOCKET MULTI-CAST   --------------------*/
    
    struct sockaddr_in servaddr;
    struct ip_mreq mcastjoin_req;      /* multicast join struct */

    // creates a datagram socket
    if ((multi_sockfd[sn] = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("receiver: socket");
        exit(1);
    }
    printf("Socket Created\n");

    // specified channel address and port are set
    memset((char *)&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(data_port);

    // socket is binded here
    if (bind(multi_sockfd[sn], (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Receiver: bind()");
    }

    // multicast address is set for the specified channel
    mcastjoin_req.imr_multiaddr.s_addr = multicast_address;
    mcastjoin_req.imr_interface.s_addr = htonl(INADDR_ANY);

    // adding channel address with the multicast socket
    if (setsockopt(multi_sockfd[sn], IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mcastjoin_req, sizeof(mcastjoin_req)) < 0)
    {
        perror("Receiver: setsockopt()");
    }

    /*----------------------    BUFFER DECLAARATIONS ------------------*/
    char buffer[bit_rate];
    int recieve_size;
    /*-----------------------------------------------------------------*/

    /*----------------------    FILE DECLARATIONS   -------------------*/
    FILE *mediaFile;
    /* Opening the file in the write mode */
    mediaFile = fopen(fname, "w");

    if (mediaFile == NULL) {
        printf("Error has occurred. Image file could not be opened\n");
        exit(1);
    }

    // This creates thread to start playing video as soon as we start receiving it
    pthread_t vlc;
    pthread_create(&vlc, NULL, on_open, (void *)fname);
    printf("\nReady to listen!\n\n");

    memset(buffer, 0, sizeof(buffer));

    int cnt = 0;
    // We receive data in the while loop below of "bit rate" size at a time. Further we store it into its designated media file
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        if ((recieve_size = recvfrom(multi_sockfd[sn], buffer, bit_rate, 0, NULL, 0)) < 0)
        {
            perror("receiver: recvfrom()");
            continue;
        }
        /* Check for the end of the file */
        if(strcmp(buffer,"end")==0) {
            printf("Video completed\n");
            // pause_player();
            // libvlc_media_player_stop(media_player);
        }
        /* For debugging purposes */
        // printf("DATA : %d\n",recieve_size);
        // writing file data in designated media file
        fwrite(buffer, 1, recieve_size, mediaFile);
    }
    
     // closing connection from receiver end once complete data is received
    printf("Closing media file, data receiving finished\n");
    
    fclose(mediaFile);
    close(multi_sockfd[sn]);
    printf("Successfully Received Channel Data!!");
}

// We receive data about the stations using the function below using TCP.
void get_station_list(gpointer data)
{
    // Creating TCP sockets to read station data
    int TCP_sockfd = 0, TCP_read;
    struct sockaddr_in TCP_servaddr;
    if ((TCP_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        exit(1);
    }
    printf("Socket Created!\n");

    if (inet_pton(AF_INET, TCP_IP, &TCP_servaddr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        exit(1);
    }

    printf("IP Checked!!\n");

    TCP_servaddr.sin_family = AF_INET;
    TCP_servaddr.sin_port = htons(TCP_PORT);

    // TCP socket is connected here
    if (connect(TCP_sockfd, (struct sockaddr *)&TCP_servaddr, sizeof(TCP_servaddr)) < 0)
    {
        printf("\nConnection Failed \n");
        exit(1);
    }

    // Reading from the TCP socket
    if ((TCP_read = read(TCP_sockfd, &infos, sizeof(station_information))) < 0)
    {
        perror("Failed to read");
        exit(-1);
    }

    gtk_clist_clear((GtkCList *)data);

    char *send_data[infos.size][2];

    for (int i = 0; i < infos.size; i++)
    {
        printf("--------------------------------------------------\n");
        printf("Station number : %d\n", infos.data[i].station_number);
        printf("Station name : %s\n", infos.data[i].station_name);
        printf("Multicast_address : %d\n", infos.data[i].multicast_address);
        printf("Video filename : %s\n", infos.data[i].video_filename);
        printf("--------------------------------------------------\n");

        char sta_num[10];
        snprintf(sta_num, sizeof(sta_num), "%d", infos.data[i].station_number);
        char sta_add[10];
        snprintf(sta_add, sizeof(sta_add), "%d", infos.data[i].multicast_address);
        

        send_data[i][0] = sta_num;
        send_data[i][1] = infos.data[i].station_name;
        

        /* Appending the data rows to the table */
        gtk_clist_append((GtkCList *)data, send_data[i]);
        close(TCP_sockfd);
    }
}

/* This function is used to check for any channel deletions. It also uses TCP for sending details of the station*/
void *check_channel(void *input){
    // Initial socket related declarations
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Socket creation
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
    address.sin_port = htons(TCP_PORT2);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    // Getting the data of deleted channels
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        int input_data;

        if(read(new_socket, &input_data, sizeof(input_data))<0){
            perror("Read failed");
            exit(EXIT_FAILURE);
        }
        // Upon receiving which channel has been deleted, we change the value to 0 in this array to indicate it has been disabled
        recieving[input_data] = 0;
        
    }
    close(new_socket);
}

// callback function when user selects a station/channel from the list
void choose_station(GtkWidget * clist, gint row, gint column,GdkEventButton * event, gpointer data) {

    // separate thread for processing further steps
    pthread_t my_radio_channel;
    // struct args *station = (struct args *)malloc(sizeof(struct args));
    struct station_list *ind_station = (struct station_list *)malloc(sizeof(struct station_list));

    ind_station->multicast_address = infos.data[row].multicast_address;
    ind_station->data_port = infos.data[row].data_port;
    ind_station->info_port = infos.data[row].info_port;
    strcpy(ind_station->station_name, infos.data[row].station_name);
    ind_station->station_number = infos.data[row].station_number;
    ind_station->bit_rate = infos.data[row].bit_rate;

    current_radio_channel = row;
    printf("---------------------------------------------------\n");
    printf("Station name : %s\n", ind_station->station_name);
    printf("Station number : %d\n", ind_station->station_number);
    printf("Data port : %d\n", ind_station->data_port);
    printf("Info port : %d\n", ind_station->info_port);
    printf("Bit rate : %d\n", ind_station->bit_rate);
    printf("Multicast address : %d\n", ind_station->multicast_address);
    printf("-----------------------------------------------------\n");
    printf("Received : %d\n",recieving[ind_station->station_number]);
    char fname[20];
    strcpy(fname,ind_station->station_name);
    strcat(fname,".mp4");

    /* Check whether the video of the station has been received previously or not */
    recieving[ind_station->station_number]==0 ? pthread_create(&my_radio_channel, NULL, play_channel, (void *)ind_station) : pthread_create(&my_radio_channel, NULL, on_open, (void *)fname);
}

int main(int argc, gchar *argv[])
{
    // GTK widgets used for creating the GUI window       
    GtkWidget *window;
    GtkWidget *vbox, *hbox;
    GtkWidget *scrolled_window, *clist;
    GtkWidget *button_fetch;
    GtkWidget *hbuttonbox;
    GtkWidget *player_widget;
    GtkWidget *stop_button;

    pthread_t channel_check_thread;
    /* Thread actively hearing for the server to tell if it has deleted any station or not */
    pthread_create(&channel_check_thread,NULL,check_channel,NULL);

    // 2 column table
    gchar *titles[2] = {"Channel No.", "Channel Name"};

    // initializing GTK
    gtk_init(&argc, &argv);

    // Main window of dimension 1000x600
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(GTK_WIDGET(window), 1000, 600);

    gtk_window_set_title(GTK_WINDOW(window), "Internet TV");
    gtk_signal_connect(GTK_OBJECT(window), "destroy",GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show(scrolled_window);
    
    // table containing channel list 
    clist = gtk_clist_new_with_titles(2, titles);

    gtk_clist_set_column_width(GTK_CLIST(clist), 0, 450);
    gtk_clist_set_column_width(GTK_CLIST(clist), 1, 450);
    

    /* Connecting the function with on click on the row of the table */
    gtk_signal_connect(GTK_OBJECT(clist), "select_row",GTK_SIGNAL_FUNC(choose_station), NULL);

    gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_OUT);

    gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
    
    gtk_widget_show(clist);
    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show(hbox);
    // Fetch button for fetching the stations
    button_fetch = gtk_button_new_with_label("Fetch All Channels");

    gtk_box_pack_start(GTK_BOX(hbox), button_fetch, TRUE, TRUE, 10);
    /* Connecting the function for fetching the station on clicked attribute of the button */
    gtk_signal_connect_object(GTK_OBJECT(button_fetch), "clicked",
                              GTK_SIGNAL_FUNC(get_station_list),
                              (gpointer)clist);

    GdkColor button_color;
    gdk_color_parse("red",&button_color);
    gtk_widget_modify_bg(GTK_WIDGET(button_fetch),GTK_STATE_NORMAL,&button_color);

    gtk_widget_show(button_fetch);
    
    // Drawing area containing video blocks
    player_widget = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    
    stop_button = gtk_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
    /* Connecting function for stoping the player with the stop button */
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop),NULL);
    hbuttonbox = gtk_hbutton_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), BORDER_WIDTH);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);
    
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, FALSE, 0);

    vlc_inst = libvlc_new(0, NULL);
    media_player = libvlc_media_player_new(vlc_inst);
    g_signal_connect(G_OBJECT(player_widget), "realize",G_CALLBACK(player_widget_on_realize), NULL);

    /* Setting color of GUI here */
    GdkColor color;
    gdk_color_parse("gray",&color);
    gtk_widget_modify_bg(GTK_WIDGET(window),GTK_STATE_NORMAL,&color);
    /* Showing all the widgets */
    gtk_widget_show_all(window);
    // shows the final GTK widget
    gtk_main();

    return 0;
}