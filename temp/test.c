#include <gtk/gtk.h>
#include <gdk/gdkx.h>

#define BORDER_WIDTH 6

void greet(GtkWidget *widget, gpointer data);
void destroy(GtkWidget *widget, gpointer data);
static int counter = 0;

GtkWidget *playpause_button;

void greet(GtkWidget *widget, gpointer data)
{
    // printf equivalent in GTK+
    g_print("Welcome to GTK\n");
    g_print("%s clicked %d times\n",
            (char *)data, ++counter);
}

void destroy(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

int main(int argc, char *argv[])
{
GtkWidget *window;
    GtkWidget *vbox, *hbox;
    GtkWidget *scrolled_window, *clist;
    GtkWidget *button_fetch;
    GtkWidget *button_fetc;
    GtkWidget *button_fetchh;
    GtkWidget *hbuttonbox;
    GtkWidget *player_widget;
    GtkWidget *stop_button;

    gchar *titles[3] = {"Channel No.", "Channel Name", "Channel Address"};

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_usize(GTK_WIDGET(window), 1000, 600);

    gtk_window_set_title(GTK_WINDOW(window), "Internet TV");
    // gtk_signal_connect(GTK_OBJECT(window), "destroy",
                    //    GTK_SIGNAL_FUNC(gtk_main_quit), NULL);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    gtk_widget_show(vbox);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    gtk_widget_show(scrolled_window);

    clist = gtk_clist_new_with_titles(3, titles);

    gtk_clist_set_column_width(GTK_CLIST(clist), 0, 380);
    gtk_clist_set_column_width(GTK_CLIST(clist), 1, 380);
    gtk_clist_set_column_width(GTK_CLIST(clist), 2, 380);

    // gtk_signal_connect(GTK_OBJECT(clist), "select_row",GTK_SIGNAL_FUNC(choose_station), NULL);

    gtk_clist_set_shadow_type(GTK_CLIST(clist), GTK_SHADOW_OUT);

    gtk_container_add(GTK_CONTAINER(scrolled_window), clist);
    // get_station_list((gpointer)clist);
    gtk_widget_show(clist);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
    gtk_widget_show(hbox);

    button_fetch = gtk_label_new("Fetch All Channels");
    gtk_widget_set_usize(GTK_WIDGET(button_fetch),100,100);
    gtk_box_pack_start(GTK_BOX(hbox), button_fetch, TRUE, TRUE, 0);

    // gtk_signal_connect_object(GTK_OBJECT(button_fetch), "clicked",
                            //   GTK_SIGNAL_FUNC(get_station_list),
                            //   (gpointer)clist);
    button_fetc = gtk_label_new("Fetch All Channels");
    gtk_box_pack_start(GTK_BOX(hbox), button_fetc, TRUE, TRUE, 0);
    gtk_widget_show(button_fetch);
    
    button_fetchh = gtk_label_new("Fetch All Channels");

    gtk_box_pack_start(GTK_BOX(hbox), button_fetchh, TRUE, TRUE, 0);
    
    player_widget = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    playpause_button = gtk_button_new_with_label("gtk-media-play");
    gtk_button_set_use_stock(GTK_BUTTON(playpause_button), TRUE);
    stop_button = gtk_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
    // g_signal_connect(playpause_button, "clicked", G_CALLBACK(on_playpause),NULL);
    // g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop), NULL);
    hbuttonbox = gtk_hbutton_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), BORDER_WIDTH);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), playpause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbuttonbox, FALSE, FALSE, 0);

    // vlc_inst = libvlc_new(0, NULL);
    // media_player = libvlc_media_player_new(vlc_inst);
    // g_signal_connect(G_OBJECT(player_widget), "realize",G_CALLBACK(player_widget_on_realize), NULL);

    gtk_widget_show_all(window);

    gtk_main();
    
    return 0;
}