# Internet-Tv
In this repository, we have worked over a transferring video using TCP and UDP protocols over multicasting in C++ and C. The server has been implemented in C++ while the client has been implemented in C to showcase that the server acts as an API. For developing GUI in client, we have used GTK-2.0 so that the client works cross platform.

We have used TCP protocol to transfer the details about different channels while the actual video transfer happens over UDP to simulate live video transfer.

### Requirements
- The file tree should consists of a folder name **videos** in which there should be files named : Station**i**.mp4 where i >= 1 and should be an integer value.
- Change the path in *client.c* @line #81 to your respective path. 
- For all the videos in the **videos** folder, run the following command so that the videos get converted to streamable videos.
    ```bash
    ffmpeg -i Stationi.mp4 -f mpegts stationi.mp4
   ```


### Installing dependencies
- Installing vlc media player (If vlc is installed with snap then uninstall and again install with apt)
- Uninstalling vlc with snap (if was installed with snap)
```bash
sudo snap uninstall vlc
```
- Installing with apt
```bash
sudo apt install vlc
```

- Install GTK Libraries
```bash
sudo apt install libgtk2.0-dev
sudo apt install libgtk-3-dev
```

- Installing other vlc dependency
```bash
sudo apt install libvlc-dev
```

### Compiling server side
```bash
g++ new_Server.cpp -o server -lpthread
```
### Compiling client side
```bash
gcc -o client client.c `pkg-config --libs gtk+-2.0 libvlc` `pkg-config --cflags gtk+-2.0 libvlc`
```

### Running server side
```bash
./server
```

### Running client side
```bash
sudo ./client
```
> Note : Here sudo is required in order to work with the multicasting.