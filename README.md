# Internet-Tv
Building a multi user Internet TV


### Running server side
```bash
g++ server.cpp -o server -lpthread
```
### Running client side
```bash
gcc -o client client.c `pkg-config --libs gtk+-2.0 libvlc` `pkg-config --cflags gtk+-2.0 libvlc`
```