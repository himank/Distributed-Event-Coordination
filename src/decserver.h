/*
 * decserver.h
 *
 *  Created on: 30-Nov-2012
 *      Author: himank
 */

#ifndef DECSERVER_H_
#define DECSERVER_H_

#ifndef MAXCONNECTION
#define MAXCONNECTION 10
#endif
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include<string.h>

using namespace::std;

struct eventNode
{
	char eventNo;
	eventNode *next;
};

struct centerNode
{
	char eventNo;
	centerNode *next;
	eventNode *child;
	eventNode *parent;
};
u_int16_t get_port_number(sockaddr *);
void *get_ip_address(sockaddr *);
void acceptConnection(centerNode **,string);
bool addNode(char, char, centerNode **,int &);
bool fromEventHandler(char from, char to, centerNode **, int &);
void toEventHandler(char from, char to, centerNode **,centerNode *);
void parse(string, centerNode **,int acceptId);
void isPresent(char, char, centerNode **,int);
void initiateConnection(string,string);
void signal_callback_handler(int signum);
bool checkRequest(vector<string>, centerNode **root);
void reset(centerNode **);
bool checkRequestGraph(char from, char to, centerNode **root);
void printSummary();
void generatingLog(string s);

#endif /* DECSERVER_H_ */
