//
//  Networking.h
//  A general purpose network socket library wrapping the neccesary work into 2 simple functions
//
//  Created by Vinny on 6/28/18.
//  Copyright © 2018 Vincent W. Trolia. All rights reserved.


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Hey Kids! Have you ever wanted to be a, a cool web programmer? With this web socket wrapping header file, it's all as easy as pie! (more like Pie-thon!     //
//  yeah, I know I suck) Basically this is a library designed to make creating and maintaining a server or client and creating the connection. It uses all      //
//  of the standard Unix-style socket commands, but I have a Windows-based library named "Win_Networking.h" if you're on team Microsoft.                        //
//  (that version is cleaner, we can get rid of all those nasty #includes) If I'm smart, it's in the same place as this library. All you have to do once        //
//  you import this library is run either connect_to_server() or connect_to_client() and you're good to go. This library's job is to set up the connection for  //
//  you, as setting up send() and receive() is often very dependant on your specific use case, and I don't want to force you to work around my specific way.    //
//  This software is licensed under Apache 2.0 Open Source License so feel free to use it and make money off of what you make. email me at                      //
//  vtrolia@protonmail.com if you have comments, concerns, questions or suggestions. Now go, enjoy this amazing technology of the future called the "Internet"  //
//  with your own C programs!                                                                                                                                   //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef Networking_h
#define Networking_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>

// This is a wrapper object so we can return not only the socket number, but also all of the other host info so that our functions can run
// independently of each other.
typedef struct{
    int socket;
    struct addrinfo *connectioninfo;
}
connection;

// This is so we can keep track of the listen() connection and the send() receive() connection on the server side
typedef struct{
    int sockaddr;
    int dataaddr;
}
tuple;


/*
 * This is our first networking wrapper function. This takes all of the complicated C socket operations and boils it down to a simple function.
 * Check for the Windows version I released so that for any project you work on, you can always use the same interface and
 * this can be a simple process for those who want to build network based programs.
 * @params domain: This is the domain name of the server you are trying to connect to, obviously a string.
 * @params port: A string representing the port you want to connect to.
 * @returns a special connection wrapper object that holds a socket address and the addrsinfo object of the server you are trying to reach. If <0, there is an error
 */
connection make_connection(const char *domain, const char *port)
{
    // Open up our errors log in case something goes wrong
    FILE *errors = fopen("net_errors.log", "a");
    
    // Create our structs to hold our connection info later. Create a class of our wrapper object so that we can return both the socket address and the connection
    // info to be used later.
    struct addrinfo my_info;
    struct addrinfo *domain_info;
    connection domain_connection;
    
    // Now, we set the client's info to be filled by the OS and set up for a reliable connection. We want TCP
    memset(&my_info, 0, sizeof(my_info));
    my_info.ai_family = AF_UNSPEC;
    my_info.ai_socktype = SOCK_STREAM;
    my_info.ai_flags = AI_PASSIVE;
    
    // We try to find tbhe IP address and other info for the host we are trying to connect to and pass it into domain_info, if not, exit the program
    if (getaddrinfo(domain, port, &my_info, &domain_info) != 0)
    {
        fprintf(errors, "address info not found\n");
        domain_connection.socket = -1;
        fclose(errors);
        return domain_connection;
    }
    
    // we want to keep track of the returned socket number if possible in order to establish the connection. This creates a socket that we can use in order
    // to later identify that connection
    int socketaddr = socket(domain_info->ai_family, domain_info->ai_socktype, domain_info->ai_protocol);
    if (socketaddr < 0)
    {
        fprintf(errors, "error in creating socket\n");
        domain_connection.socket = -2;
        fclose(errors);
        return domain_connection;
    }
    
    // Now, open the port on our end, and hope that it isn't currently in use. If it is in use, try recalling the function with a different one.
    // We need to populate an internet socket address struct and pass it in
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(atoi(port));
    if (bind(socketaddr, (struct sockaddr *) &local, sizeof(local)) != 0)
    {
        fprintf(errors, "port already in use\n");
        fclose(errors);
        domain_connection.socket = -3;
        return domain_connection;
    }
    
    // now, if everything else has gone perfectly, we populate our wrapper object with the relevant info and return it to be used with either client or server.
    // designed this way for interoperability between the two types.
    domain_connection.socket = socketaddr;
    domain_connection.connectioninfo = domain_info;
    fclose(errors);
    return domain_connection;
    
}


/*
 * This is our function designed for a client to connect to a server. simply creates the connection, doesn't send any data.
 * @params domain: this is the string of the domain name you want to connect to
 * @params port: a string representing the port we wish to open a connection with. Can be NULL, in which one is provided for you
 * @returns: if the connection is established, it returns the address of the socket. If the connection is not established, returns a negative int
 */
int connect_to_server(const char *domain, char *port)
{
    // open up our log in case of errors
    FILE *errors = fopen("net_errors.log", "a");
    
    // The implimentation of socket connections in C dictates that the port # of a connection must be a string, so if the port
    // is not supplied by the caller, we convert any number larger than 1049 to see if it works and take the first successful one to make the connection
    char str[6];
    connection server_data;
    if (port)
    {
        strcpy(str, port);
    }
    else
    {
        for (int i = 1050; i < 65536; i++)
        {
            snprintf(str, 6, "%i", i);
            server_data = make_connection(domain, str);
            if (server_data.socket > 0)
            {
                break;
            }
        }
    }
    
    // if we can successfully connect to the server, return the address of the successful connection, else return the error code.
    int success = connect(server_data.socket, server_data.connectioninfo->ai_addr, server_data.connectioninfo->ai_addrlen);
    if (success > 0)
    {
        fprintf(errors, "connection could not be established\n");
        fclose(errors);
        return success;
    }
    else
    {
        fclose(errors);
        return server_data.socket;
    }
}


/*
 * This is the function for a server that would like to listen to and connect to a client. Takes in no params b/c we are finding the addr info of our
 * own computer to send off to the client when they use the function above
 * @params: port, a string representing the port to be used, can be NULL, then one will be selected for you.
 * @returns sn_rec: a tuple that contains the listen socket and the data transfer socket.
 */
tuple connect_to_client(char *port)
{
    // same as connect to server, use the port supplied by the user, if not, try all of our ports to find an open one and see if it works.
    // The only difference is that the host we are trying to get information is our own
    char str[6];
    connection server_data;
    if (port)
    {
        strcpy(str, port);
    }
    else
    {
        for (int i = 1050; i < 65536; i++)
        {
            snprintf(str, 5, "%i", i);
            server_data = make_connection(NULL, str);
            if (server_data.socket > 0)
            {
                break;
            }
            
            // if we used a random port, we print it out so you can keep track of which one you're using
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
    
    // Now, we supply the empty structs to fill with client data and start the connection. Save both our listening socket and the data connection socket
    // and return that in a tuple
    unsigned int ad_size = sizeof(struct sockaddr);
    tuple sn_rec;
    sn_rec.sockaddr = server_data.socket;
    struct sockaddr *client_info = malloc(sizeof(struct sockaddr));
    sn_rec.dataaddr = accept(server_data.socket, client_info, &ad_size);
    return sn_rec;
}

#endif /* Networking_h */
