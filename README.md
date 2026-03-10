# TCP-Server
A TCP server/client made in C. Has the ability to handle multiple clients and send messages from client to client through a claude code style TUI

![Server/client demo](https://github.com/user-attachments/assets/d6fea802-315f-41fc-975e-d444000ba5b2)

## How it works
The server keeps track of clients using an array. Once a client wants to send data (known using the `FD_ISSET` function) the server `recv` it and process it.
The processing is handled by a function that parses the string. If the string starts with ``:`` then we know it is meant to be sent to another client.
So we get the char after the ``:`` and parse it into an int which is the client the message is to be sent to.

The client connects to the server and a TUI is drawn inside a while loop. The client is able to type their message into the TUI and send it to the server whenever enter is pressed.
Any message a client receives from the server or another client is shown in a `history_buffer` above the message box.

## How to build it
Must be on a Unix based system (Linux/MacOS)
Using the makefile (uses clang)

```bash
make both
```

or with gcc

```bash
gcc client.c -lcurses -o client.out
gcc server.c -o server
```

## How to run it
Run the server
```bash
./server
```

Then open as many terminal window as you would like and run the client 
```bash
./client.out
```

## Limitations
Server can only handle 10 clients. No server commands. No security.
