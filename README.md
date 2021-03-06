# Networking.h

A simple library of functions to make network programming in C much easier, now with SSL/TLS Support with OpenSSL! Just make sure you have it installed, as I learned that lesson the hard way. 

## Example Code:

### Client:
```C
#include "Networking.h"

int main(void)
{
    initialize_ssl();
    ssl_tuple server_connection = secure_connect_to_server("127.0.0.1", 7754, 7755);
    char message[256];
    
    do
    {
        memset(&message, 0, 256);
        fgets(message, 255, stdin);
        secure_send(server_connection.ssl_connection, message, (int) strlen(message));
    }
    while (message[0] != 'q');
    secure_close(server_connection, true);
    return 0;
}

```

input:
```
Vinnys-MacBook-Pro:Network-Program-Directory vinny$ ./client
Hello, World! 
This is an example
of what you can do with
the Networking.h library! In
this library, all you need to do   
is make a few short function calls and
then you're off to the races! Feel free to 
read through and check out what's under the hood!
I designed this library to be robust and able to support
complex network programs, but to be simple enough that you 
don't have to go through pages upon pages of socket documentation!
Please, give it a try and tell me what you think! I love to get feedback!
q
Vinnys-MacBook-Pro:Network-Program-Directory vinny$ 
```
### Server:
```C
#include "Networking.h"

int main (void)
{
    initialize_ssl();
    ssl_tuple connection = secure_connect_to_client("privkey.pem", "cacert.pem", 7754);
    char mes[256];
    
    do
    {
        memset(&mes, 0, 256);
        secure_recieve(connection.ssl_connection, mes, 256);
        printf("%s", mes);
    }
    while (mes[0] != 'q');
    
    secure_close(connection, true);
    shutdown(connection.listen, 2);
    return 0;
    
}
```
output:
```
Vinnys-MacBook-Pro:Network-Program-Directory vinny$ ./server
Hello, World!
This is an example
of what you can do with
the Networking.h library! In
this library, all you need to do
is make a few short function calls and
then you're off to the races! Feel free to
read through and check out what's under the hood!
I designed this library to be robust and able to support
complex network programs, but to be simple enough that you 
don't have to go through pages upon pages of socket documentation!
Please, give it a try and tell me what you think! I love to get feedback!
q
Vinnys-MacBook-Pro:Network-Program-Directory vinny$ 
```

## Structs:

### connection: (int socket, addrinfo *connectioninfo)
-	This struct is designed as a general-purpose holder for our information when creating a connection. It contains a bounded socket to be used for a connection and a pointer to a struct that contains all of the info for the computer you wish to connect to. Generated by the make_connection() function and utilized in the connect_to_client() and connect_to_server() functions. 

### tuple: (int sockaddr, int dataaddr)
-	This struct is used to keep track of both sockets being used by a server program. The sockaddr is the descriptor of the socket that is listening for new connections, and dataaddr is generated for each new accepted connection. The only time you will run into this struct is when you are running a server.

### ssl_tuple: (SSL *ssl_connection, SSL_CTX *ctx, int socket, int listen)
-	Our final struct is one designed for memory clearing once our server or client is ready to close. Ssl_connection is a pointer to the file descriptor of an encrypted OpenSSL connection, and ctx is a pointer to the current SSL context being used. Socket is the descriptor of data transfer socket. To add more portability and customizability for those who want to make more complicated applications, I added the listening socketfd to the server's ssl_tuple struct. That way if you want to run multiple clients with fork() or even just have more control over the server process, you are now free to do as such.

## Functions:

### char *get_datetime_s(void):
This function will get the local date and time for the system from the OS and returns it in a string. It returns the date and time in the format: year-month-day hours:minutes:seconds. Takes no arguments, and does not use malloc() for the string it returns, so you do not need to worry about using free() with the pointer this returns.

### connection make_connection( const char *domain, unsigned int port,  unsigned int client_port):
-	*Domain*: a string that either holds the ip address or the domain name of the server you wish to connect to
-	*Port*: This is the port being used on the server that you wish to connect to
-	*Client_port*: represents the port YOU want to use on your local computer. If you are using this for a server application, pass in 0.

This function will create a socket for you that is ready to be connected to a client or a server and passes you the information you need to connect back in a connection struct. If you want to make custom connecting to client and connecting to server functions, use this. If not, this function’s main purpose is to be used by the higher-level functions when they actually will connect to you to the computer you want or listen for a client to connect. It will create an IPv4 TCP connection.


### int connect_to_server(const char *domain, unsigned int port, unsigned int client_port):
-	The  parameters for this function are exactly the same as those for make_connection(), scroll up a bit to see for yourself.

This function will actually complete every part of the connection process for you up to the send() and recv() functions are needed. Because send and receive depend heavily on your specific use case, I didn’t want to lock you into only one possible way of sending data. It returns the socketfd of your connection to the server.

### tuple connect_to_client(unsigned int port):
-	*Port*: The port you wish to run your server on, every other part of the equation is simply part of your local host, so we can generate it in the function itself

In this function, we start a server and listen until a client attempts to connect. If it is successful, we return a tuple that has both the address of the listening socket that is still open, and the newly formed data socket to send and receive. After calling this function, send() and recv() are all that you need, aside from calling shutdown() when you are finished.

### void initialize_ssl(void):

This function loads up all of the errors, algorithms and libraries needed in order to start a working OpenSSL SSL/TLS server. You must call this function first before any of the other functions using SSL or else they will not work. Put this at the top of your program, or at least at the top of any of your networking functionality.

### unsigned long secure_send(SSL *ssl, char *message, int size) &
 ### unsigned long secure_recieve(SSL *ssl, char *buffer, int size):
-	*Ssl*: A pointer to the ssl enabled socket we are using for our connection
-	*Message/buffer*: these are the pointers to the strings where our message comes from, and where it will be put into
-	*Size*: the size of the message (in bytes)

These are wrapper functions for sending and receiving SSL data. I made these so that you don’t have to go through the OpenSSL documentation to figure out how to use them. It doesn’t add or take away any of the functionality or sending or receiving, I just put the functions in an easy to understand, simple interface included in my library.

### void create_authorization(void):
This is a function to be used with testing or if you want to make self-signed certificates. The requirement is of course that you have OpenSSL’s binary compiled and ready to run. It will make a private key file in the directory your program is kept in called “privkey.pem” and it creates the self-signed certificate as “cacert.pem.” If you are making a full production program, I suggest using the actual OpenSSL program so you can get exactly what you want, as all this does is make a system call to the basic certification listed in OpenSSL’s README file.


### ssl_tuple secure_connect_to_client(const char *prikey_file, const char *cert_file, 
### unsigned int port):
-	*prikey_file*: This string can be either a relative or absolute path to the SSL private key you have either generated with the function above or on your own with the OpenSSL binary
-	*cert_file*: This string is the relative or absolute path to the certificate (either self-signed or registered with a CA) that belongs to your program. Both of these are used with the new SSL connection to encrypt/decrypt the data, as well as help the client generate their own keys
-	*port*: This is the port that you want to listen on

This is our SSL/TLS equivalent to the connect_to_client() function. In fact, it first calls this function so that they keys and information can be exchanged before we move to secured data transfer. We cast the regular TCP connection into a Secure Socket Layer encryption. This function returns and ssl_tuple of all the information required for secure_send(), secure_recieve() and secure_close().

### ssl_tuple secure_connect_to_server(char *hostname, unsigned int port, unsigned int user_port):
-	for this function, the params are the same as the unsecure method’s parameters, with user_port serving the same function as client_port.

Much like its unsecure counterpart, this function will create a connection to the host and port passed in on the user_port you specify. Once this is complete, we cast the port as a Secure Socket Layer connection. We take all of this info and pack it into an ssl_tuple that has all the info you need for secure_send(), secure_recieve() and secure_close().

### void secure_close(ssl_tuple running_ssl):
-	*running_ssl*: this is the ssl_tuple that was either passed back to you by secure_connect_to_server() or secure_connect_to_client().

This function packages together all of the steps needed to free all of the memory used by our ssl connection. It closes all of our sockets and our stored SSL functions and variables. 

### void generate_SHA256_hash(char *tocrypt, char result[65]):
- *tocrypt*: The string of characters you wish to encrypt 
- *result*: this is the final string that you want to store your hash in, 65 characters long. 




***ALL ERRORS GENERATED BY THESE FUNCTIONS ARE STORED IN THE FILE
      NET_ERRORS.LOG ***

