# Distributed Event Coordination System
This project is to implement a distributed event coordination system (DEC) system in C++ on a UNIX-based platform. This system will consist of a DEC server which can interact and communicate with multiple DEC clients at the same time.

dec_server is the server component of the Distributed Event Coordination (DEC) System. It will listen on a certain port number for the incoming requests from DEC clients.


There can be three different requests coming from clients:

## Insert Request
This request from any client will enter new event ordering information to the server using the happened–before relationship. 
In this relationship, events will be represented with capital letters (i.e., A, B, C, D… etc), and the happened before relationship will be represented with “->” (dash and right angle bracket). The event orderings will be separated with “white space” and will end with a semicolon.

Example for an insert request:
```shell
dec_client1$ insert A->B B->C C->D;
dec_client1$ response from server: INSERT DONE.
dec_client1$ _
```

This event insertion request is sent to the server from dec_client1, and it includes four events (A, B, C, D). 
Event A happened before event B; event B happened before event C; and event C happened before event D.

After receiving the insert request, the DEC server will insert this event ordering information to its global event ordering graph. If the insertion is successful, it will return to the client an “INSERT DONE” message.

## Query request
This request from any client will ask the relative ordering between any two events. The queried events in this request will be separated with “white space” and will end with a semicolon.

Example for a query request:
```shell
dec_client1$ query A D;
dec_client1$ response from server: A happened before D.
dec_client1$ _
```

There can be multiple requestssent on the same line:
```shell
dec_client1$ insert E->F; query A E;
dec_client1$ response from server: A concurrent to E.
dec_client1$ _
```

If there is no information available about any particular event at the server, it should return an error message:

```shell
dec_client1$ query A G;
dec_client1$ response from server: Event not found: G.
dec_client1$ _
```

## Reset Request
This request from any client will reset the global event ordering graph at the server. The reset request will not take any arguments and will be followed by a semicolon.

Example for a reset request:
```shell
dec_client1$ reset;
dec_client1$ response from server: RESET DONE.
dec_client1$
```
dec_client is the client component of the Distributed Event Coordination 
(DEC) System. It will connect to the DEC server running on particular host and port number and 
willsend the server one of the three requests mentioned above.

Command Line:
  * Step1: ./decserver [-s] [-h] [-p port-number] [-l file]
    - a) if [-p port-number] is not given then by default it will run on port 9090
    - b) if [-l file] filename is not given then it will print to stdout
    - c) [-h] is optional
    - d) [-s] is necessary
  * Step2: Run Client by using: ./decserver [-c ip of server] [-p port-number of server]
    - a) if [-c ip of server] is not given then it will assume server ip address as localhost address and connect
    - b) if [-p port-number of server] is not given then it will assume server is running on 9090
    - c) if only ./decserver is used to run then client will try to connect on localhost at port 9090
  * Step3: [-h] parameter is used to display the summary of the commands.
  * Step4: Press Ctrl+c to exit from server or client.

Note: Same code is used for client and server
