//
//  Networking.h
//  A general purpose network socket library wrapping C's complicated socket API and OpenSSL's poorly documented API into portable and simple functions
//
//  Created by Vinny Trolia on 6/28/18.
//  Copyright © 2018 Vincent W. Trolia. All rights reserved.


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Hey Kids! Have you ever wanted to be a, a cool web programmer? With this web socket wrapping header file, it's all as easy as pie! (more like Pie-      //
//  thon! yeah, I know I suck) Basically this is a library designed to make creating and maintaining a server or client and creating the connection. It     //
//  uses all of the standard Unix-style socket commands, but I have a Windows-based library named "Win_Networking.h" if you're on team Microsoft.           //
//  (that version is cleaner, we can get rid of all those nasty #includes) If I'm smart, it's in the same place as this library. All you have to do         //
//  once you import this library is run either connect_to_server() or connect_to_client() and you're good to go. This library's job is to set up the        //
//  connection for you, as setting up send() and receive() is often very dependant on your specific use case, and I don't want to force you to work around  //
//  my specific way. This software is licensed under MIT License so feel free to use it and make money off of what you make. email me at                    //
//  vtrolia@protonmail.com if you have comments, concerns, questions or suggestions. Now go, enjoy this amazing technology of the future called the         //
//  Internet" with your own C programs! Now with added SSL support! For this, we use OpenSSL                                                                //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Networking.h"

// This is a function to get a string of the current date and time
char *get_datetime_s(void)
{
    // create the strcuts
    struct timeval time;
    time_t now;
    struct tm *now;
    char timestr[64], buff[64];
    
    // ask the operating system for the machine's local time, then return it in human-readable form
    gettimeofday(&time, NULL);
    now_t = time.tv_sec;
    now = localtime(&now_t);
    strftime(timestr, sizeof timestr, "%Y-%m-%d %H:%M:%S", now);
    snprintf(buff, sizeof buff, "%s.%06d", timestr, time.tv_usec);
    return buff;
    
}


/**
 * This is the first networking wrapper function. It takes all of the complicated C socket operations and boils it down to a simple function.
 * Check for the Windows version to be released so that for any project you work on, you can always use the same interface and
 * this can be a simple process for those who want to build network based programs.
 * @param domain: This is the domain name of the server you are trying to connect to, obviously a string.
 * @param port: A string representing the port to connect to.
 * @param client_port: an optional string representing the port to bind the connection to. pass NULL in as the value if you are running a
 * server and add a value if you are running a client application.
 * @return domain_connection: a special connection wrapper object that holds a socket address and the addrsinfo object of the server you are
 * trying to reach. If <0, there is an error
 */
connection make_connection(const char *domain, const char *port, char *client_port)
{
    // Open up the errors log in case something goes wrong
    FILE *errors = fopen("net_errors.log", "a");
    
    // Create the structs to hold connection info later. Creates a class of the wrapper object so that it returns both the socket address and the
    // connection info to be used later.
    struct addrinfo my_info;
    struct addrinfo *domain_info;
    connection domain_connection;
    
    // The goal is to keep track of the returned socket number if possible in order to establish the connection. This creates a socket that can
    // be used in order to later identify that connection
    int unsigned socketaddr = socket(AF_INET, SOCK_STREAM, 0);
    if (socketaddr == 0)
    {
        fprintf(errors, "%s: error in creating socket\n", get_datetime_s());
        domain_connection.socket = -2;
        fclose(errors);
        return domain_connection;
    }
    
    // Set the client's info to be filled by the OS and set up for a reliable connection. We want TCP
    memset(&my_info, 0, sizeof(my_info));
    my_info.ai_family = AF_UNSPEC;
    my_info.ai_socktype = SOCK_STREAM;
    my_info.ai_flags = AI_PASSIVE;
    
    // Try to find tbhe IP address and other info for the host you are trying to connect to and pass it into domain_info, if not, exit the program
    if (getaddrinfo(domain, port, &my_info, &domain_info) != 0)
    {
        fprintf(errors, "%s: address info not found\n", get_datetime_s());
        domain_connection.socket = -55;
        fclose(errors);
        return domain_connection;
    }
    
    // Open the port on this end, and hope that it isn't currently in use. If creating a server, use the port passed in
    // as the listening port. Otherwise with a client needs to use a selected port
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    if (client_port)
    {
        local.sin_port = htons(atoi(client_port));
    }
    else
    {
        local.sin_port = htons(atoi(port));
    }
    
    if (bind(socketaddr, (struct sockaddr *) &local, sizeof(local)) != 0)
    {
        fprintf(errors, "%s: port already in use\n", get_datetime_s());
        fclose(errors);
        domain_connection.socket = -3;
        return domain_connection;
    }
    
    // If everything else has gone perfectly, populate the wrapper object with the relevant info and return it to be used with either client or server.
    // designed this way for interoperability between the two types.
    domain_connection.socket = socketaddr;
    domain_connection.connectioninfo = domain_info;
    fclose(errors);
    return domain_connection;
    
}


/**
 * This is the function designed for a client to connect to a server. It simply creates the connection and doesn't send any data.
 * @param domain: this is the string of the domain name to connect to
 * @param server_port: a string representing the port that the server application is listening on
 * @param connection_port: a string representing the port we wish to bind our connection to
 * @return: if the connection is established, it returns the address of the socket. If the connection is not established, returns a negative int
 */
int connect_to_server(const char *domain, char *server_port, char *connection_port)
{
    // open up our log in case of errors
    FILE *errors = fopen("net_errors.log", "a");
    
    // The implimentation of socket connections in C dictates that the port # of a connection must be a string. Because this is the client
    // connection call, it needa both the port used by the server and the port to use on our end.
    connection server_data = make_connection(domain, server_port, connection_port);
    
    /* Sometimes the server_data connection has a negative socketfd, it can cause a segfault in this function. For some
     * reason I thought that the client connection needed to be the one provided by the parameter connection_port, this is
     * obviously not the case, so instead the same function will search around until a usable port is found.
     * Returns -1 if no port in the entire OS can be found now, so that even if this fails there still is no segfault.
     */
    char str[5];
    if (server_data.socket < 0)
    {
        for (int i = 1050; i < 65536; i++)
        {
            snprintf(str, 5, "%i", i);
            server_data = make_connection(domain, server_port, str);
            if (server_data.socket > 0)
            {
                break;
            }
        }
        if (server_data.socket < 0)
        {
            return -1;
        }
    }
    
    // if successfully connected to the server, return the address of the successful connection, else return the error code.
    int success = connect(server_data.socket, server_data.connectioninfo->ai_addr, server_data.connectioninfo->ai_addrlen);
    if (success < 0)
    {
        fprintf(errors, "%s: connection could not be established\n", get_datetime_s());
        printf("%s\n", strerror(errno));
        fclose(errors);
        return success;
    }
    else
    {
        fclose(errors);
        return server_data.socket;
    }
}


/**
 * This is the function for a server that would like to listen to and connect to a client. Takes in no params b/c it is finding the addr info of your
 * own computer to send off to the client when they use the function above
 * @param: port, a string representing the port to be used, can be NULL, then one will be selected for you.
 * @return sn_rec: a tuple that contains the listen socket and the data transfer socket.
 */
tuple connect_to_client(char *port)
{
    /* same as connect to server, use the port supplied by the user, if not, try all of our ports to find an open one and see if it works.
     * The only difference is that the host you are trying to get information is your own. Since this is running a server, the argument for client
     * port when the connection is made will be NULL.
     */
    char str[6];
    connection server_data;
    if (port)
    {
        strcpy(str, port);
        server_data = make_connection("127.0.0.1", str, NULL);
    }
    else
    {
        for (int i = 1050; i < 65536; i++)
        {
            snprintf(str, 5, "%i", i);
            server_data = make_connection("127.0.0.1", str, NULL);
            if (server_data.socket > 0)
            {
                break;
            }
            
            // if a random port is used, print it out for debugging purposes
            printf("using port: %s\n", str);
        }
    }
    
    // if for some reason the listening fails, return a negative tuple called failure
    if(listen(server_data.socket, 10) < 0)
    {
        tuple failure;
        failure.sockaddr = -1;
        failure.dataaddr = -1;
        return failure;
    }
    
    // Now, supply the empty structs to fill with client data and start the connection. Saves the listening socket and the data connection socket
    // and return that in a tuple
    unsigned int ad_size = sizeof(struct sockaddr);
    tuple sn_rec;
    sn_rec.sockaddr = server_data.socket;
    struct sockaddr *client_info = malloc(sizeof(struct sockaddr));
    sn_rec.dataaddr = accept(server_data.socket, client_info, &ad_size);
    free(client_info);
    return sn_rec;
}


// Call this to start up an SSL connection. This should be your first step before calling anything else, and then you are ready to connect to a client or
// server using OpenSSL
void initialize_ssl(void)
{
    SSL_load_error_strings();
    ERR_load_ERR_strings();
    OpenSSL_add_ssl_algorithms();
    SSL_library_init();
    
}


// a wrapper for the SSL function, so you don't need to try and find the horrifying OpenSSL documentation
unsigned long secure_send(SSL *ssl, void *message, int size)
{
    return SSL_write(ssl, message, size);
}

// a wrapper for the SSL function, so you don't have to look around and around on Google or Stack Overflow for the answer
unsigned long secure_recieve(SSL *ssl, void *buffer, int size)
{
    return SSL_read(ssl, buffer, size);
}


/**
 * Here is a function ro use if you want to create your own self-signed certificate in a wrapper. If you are on a Linux distro openssl may
 * be preinstalled but if you have OSX or Windows you'll have to download it, if you want. This is mainly to automate the process and the result is
 * a private key stored in "privkey.pem", and a self-signed certificate "cacert.pem". If you want a certificate that is signed by a
 * CA, that is a much more involved process which you should do manually and not try to do it programatically.
 */
void create_authorization(void)
{
    system("openssl genrsa -out privkey.pem");
    system("openssl req -new -x509 -key privkey.pem -out cacert.pem -days 1095");
}


/**
 * Here is the function to create a Secure Socket Layer connection with a client. It takes care of everything and takes the
 * complicated and often confusing OpenSSL functions and boils it down to a single function call. Of course, this function assumes
 * that initialize_ssl() has already been run to start up everything
 * @param prikey_file: whether you have a CA signed certificate or a self signed, pass the filename of the private key here.
 * @param cert_file: here is the actual filename of the certificate. In order to run an SSL server, a private key AND a certificate are needed
 * @param port: the string representation of the port to use for the server
 * @return: a ssl_tuple that contains a ctx pointer and an SSL pointer that holds the address of a secure socket, be used for sending and recieving later.
 * Make sure you use the wrapper secure_send() and secure_recieve() instead of the usual socket sending and recieving functions. Returns both
 * so they can be freed later
 */
ssl_tuple secure_connect_to_client(const char *prikey_file, const char *cert_file, char *port)
{
    // Make sure the SSL connection's data can be initialized
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(SSLv23_server_method());
    if (!ctx)
    {
        FILE *errors = fopen("net_errors.log", "a");
        ERR_print_errors_fp(errors);
        fclose(errors);
        free(ctx);
        exit(-1);
    }
    
    SSL_CTX_set_ecdh_auto(ctx, 1);
    
    // This makes sure that both the certificate and private key exist and are in the right formst. If not, terminate. Check the logs
    if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0)
    {
        FILE *errors = fopen("net_errors.log", "a");
        fprintf(errors, "%s: ", get_datetime_s());
        ERR_print_errors_fp(errors);
        fclose(errors);
        free(ctx);
        exit(-1);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, prikey_file, SSL_FILETYPE_PEM) <= 0 )
    {
        FILE *errors = fopen("net_errors.log", "a");
        fprintf(errors, "%s: ", get_datetime_s());
        ERR_print_errors_fp(errors);
        fclose(errors);
        free(ctx);
        exit(-1);
    }
    
    // Set up the regular data connection, or wait until the port is open if it is in use. The limit is 10s to avoid an infinite loop of waiting
    tuple unsecure_connection = connect_to_client(port);
    int wait_time = 0;
    while (unsecure_connection.dataaddr < 0 || unsecure_connection.sockaddr  < 0 || wait_time >= 10)
    {
        sleep(1);
        wait_time++;
        unsecure_connection = connect_to_client(port);
    }
    
    // finally, bind the SSL configuration to the regular socket connection and pass back the connection if all goes well
    SSL *ssl;
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, unsecure_connection.dataaddr);
    
    if (SSL_accept(ssl) <= 0)
    {
        FILE *errors = fopen("net_errors.log", "a");
        fprintf(errors, "%s: ", get_datetime_s());
        ERR_print_errors_fp(errors);
        fclose(errors);
        free(ctx);
        free(ssl);
        exit(-1);
    }
    
    // Store all of the connection data into a ssl_tuple struct so it can be used for sending data, as well as close it down and free the memory it uses.
    ssl_tuple server_tuple;
    server_tuple.ctx = ctx;
    server_tuple.ssl_connection = ssl;
    server_tuple.listen = unsecure_connection.sockaddr;
    return server_tuple;
}


/**
 * Simple client connection setup for a Secure Socket Client. This is a simple wrapper so that it works exactly the same as the unsecured
 * connect_to_server. It assumes that init_ssl() has already been run
 * @params hostname: server you want to connect to
 * @params port: a string representation of the port the server is currently listening on
 * @params user_port: this is the string representation of the port the client end should use
 * @returns: a ssl_tuple containing both the ctx pointer and the pointer to the binded ssl socket
 */
ssl_tuple secure_connect_to_server(char *hostname, char *port, char *user_port)
{
    // let's get SSL/TLS started in here
    SSL_CTX *ctx;
    ctx = SSL_CTX_new(SSLv23_client_method());
    if (!ctx)
    {
        FILE *errors = fopen("net_errors.log", "a");
        fprintf(errors, "%s: ", get_datetime_s());
        ERR_print_errors_fp(errors);
        fclose(errors);
        free(ctx);
        exit(-1);
    }
    
    // similar to setting up an unsecure client, but now bind it to ssl
    int unsecure_server = connect_to_server(hostname, port, user_port);
    if (unsecure_server < 0)
    {
        ssl_tuple fail;
        fail.ssl_connection = NULL;
        return fail;
    }
    
    // create ssl contexr
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, unsecure_server);
    
    // connections die when they are killed
    if(SSL_connect(ssl) <= 0)
    {
        FILE *errors = fopen("net_errors.log", "a");
        fprintf(errors, "%s: ", get_datetime_s());
        ERR_print_errors_fp(errors);
        fclose(errors);
        free(ssl);
        free(ctx);
        exit(-1);

    }
    
    // store all pointers and addresses into a tuple so we don't leak memory or leave a connection running forever, unused.
    ssl_tuple server_tuple;
    server_tuple.ctx = ctx;
    server_tuple.ssl_connection = ssl;
    server_tuple.socket = unsecure_server;
    return server_tuple;
    
}


/**
 * With this function, All of the functions needed to call to end the ssl connection for either the client or server are consolidated.
 * @param running_ssl: This is the tuple that contains all the information for the SSL/TLS connection, it should have the socketid of
 * a connection socket, not the one that is listening if you are running a server. If you are shutting down a server, you still have to run
 * shutdown() on the listening socket.
 * @param close_all: This is a Boolean representing whether or not you are completly done with SSL/TLS. If you intend on using multiple connections over
 * SSL/TLS, this should be false because it closes down all SSL/TLS usage.
 */
void secure_close(ssl_tuple running_ssl, bool close_all)
{
    shutdown(running_ssl.socket, 0);
    SSL_free(running_ssl.ssl_connection);
    SSL_CTX_free(running_ssl.ctx);
    if (close_all)
    {
       OPENSSL_cleanup();
    }
    
}


/**
 * This function is an easy wrapper for sha256 encryption, the hot new encryption algorithm that is the hottest new buzzword in the cybersecurity world.
 * Obviously, its good to encrypt data so this function takes care of all of that.
 * @param tocrypt: the string to be encrypted
 * @param result: the 64 character array or string that will house the final sha256 digest
 */
void generate_SHA256_hash(char *tocrypt, char result[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    SHA256_CTX con;
    SHA256_Init(&con);
    SHA256_Update(&con, tocrypt, strlen(tocrypt));
    
    SHA256_Final(hash, &con);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(result + (i * 2), "%02x", hash[i]);
    }
    result[64] = '\0';
}

