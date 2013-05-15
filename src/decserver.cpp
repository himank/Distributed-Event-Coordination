//============================================================================
// Name        : DistributedEvent.cpp
// Author      : Himank Chaudhary
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include <algorithm>
#include <signal.h>
#include <sstream>
#include "../src/decserver.h"
#include <fstream>

using namespace std;

string filename="";
bool logging = false;
int main(int argc, char* argv[])
{
	centerNode *root;
	root = NULL;
	string ip= "127.0.1.1";
	string port = "9090";
	bool server = false;
		int opt = 0;
        opt = getopt( argc, argv,"shc:p:l:" );
        while( opt != -1 ) {
            switch( opt ) {
                case 's':
                	server = true;
                    break;
                case 'c':
                    server = false;
                	ip.assign(optarg);
                    break;
                case 'h':
                    printSummary();
                    break;
                case 'p':
                	port.assign(optarg);
                    break;
                case 'l':
                	filename.assign(optarg);
                    logging = true;
                	break;
                default:
                    break;
            }
            opt = getopt( argc, argv, "shc:p:l:" );
        }
    if(logging)
    	cout<<"Logging is on everything on the server side will be printed on file"<<endl;

    if(server)
    	acceptConnection(&root,port);
    else
    	initiateConnection(ip,port);
    if(root != NULL) {
    	while(root->next != NULL) {
    			centerNode *temp = root;
    			root = root->next;
    			delete temp;
    	}
    	delete root;
    	root = NULL;
    }
    signal(SIGINT, signal_callback_handler);
	return 0;
}

////*****************************************************Below function is used for binding the socket and call the parse function
void acceptConnection(centerNode **root, string port)
{
	struct addrinfo inValue, *serverInfo, *validInfo;
			struct sockaddr_storage clientAddr;
			int acceptId, address,yes;
			int sockId;
			char buffer[100];
			char ip[INET6_ADDRSTRLEN];
			int recvbytes=0;
			//char ip1[INET6_ADDRSTRLEN];
			socklen_t addrlen;
			memset(&inValue, 0, sizeof(inValue));
			inValue.ai_family = AF_UNSPEC;
			inValue.ai_socktype = SOCK_STREAM;
			inValue.ai_flags = AI_PASSIVE;
			yes=1;
			if (getaddrinfo(NULL, port.c_str(), &inValue, &serverInfo) != 0)
				    perror("Get Address:");
			for(validInfo = serverInfo; validInfo != NULL; validInfo = validInfo->ai_next) {
				if((sockId = (socket(validInfo->ai_family, validInfo->ai_socktype,0))) == -1)
						perror("Socket:");
				addrlen = sizeof(serverInfo);
				if (setsockopt(sockId, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
						perror("setsockopt");
				        break;
				}
				if(bind(sockId,validInfo->ai_addr, validInfo->ai_addrlen) == -1)
						perror("Bind:");
				break;
			}				// successfully done with bind
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)validInfo->ai_addr;
			void *addr;
			addr = &(ipv4->sin_addr);
			inet_ntop(validInfo->ai_family,addr, ip, sizeof(ip));
			freeaddrinfo(serverInfo);
			cout<<"dec_server$: "<<"Server is listening on IP: "<<ip<<" Port :"<<port<<endl;
			if(listen(sockId, 10) == -1)
				perror("Listen:");

			while(true) {
				address = sizeof(clientAddr);
				if((acceptId = accept(sockId,(struct sockaddr*)&clientAddr,(socklen_t *)&address)) == -1)
						perror("Accept:");
				if((recvbytes = (recv(acceptId, buffer, sizeof(buffer),0))) == -1) {
					    	perror("Receive:");
					    	exit(1);
					    }
				buffer[recvbytes] = '\0';
				stringstream ss;
				ss<<"dec_server$: request received from ";
				ss<<ip;
				ss<<" : ";
				ss<<buffer;
				string s = ss.str();
				string request(buffer);
				if(logging)
					generatingLog(s);
				else
					cout<<"dec_server$: request received from "<<ip<<" : "<<endl<<buffer<<endl;
				parse(request, &(*root),acceptId);
				//sleep(5);
				close(acceptId);
				//exit(1);
			}


}

////*****************************************************Below function is only work when from cmd this code will be use as client only
void initiateConnection(string ip, string port)
{
	int recvbytes=1;
	char buffer[100];
	int sockfd = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr;

    while(true) {
    	memset(recvBuff, '0',sizeof(recvBuff));
    	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	        cout<<"\n Error : Could not create socket \n";
	        exit(1);
    	}
    	memset(&serv_addr, '0', sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_port = htons(atoi(port.c_str()));
    	if(inet_pton(AF_INET,ip.c_str(), &serv_addr.sin_addr)<=0) {
    		cout<<"\n inet_pton error occured\n";
    		exit(1);
    	}
    	string request;
    	cout<<endl<<"dec_client$: ";
    	getline(cin,request);
    	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    	       cout<<"\n Error : Connect Failed \n";
    	       exit(1);
    	}
    	if (send(sockfd, request.c_str(), strlen(request.c_str()), 0) == -1){
	   		perror("send");
	   		//cout<<"R u Sure"<<endl;
    	}
	   	while(true) {
	   		if((recvbytes = (recv(sockfd, buffer, sizeof(buffer),0))) == -1) {
	   			perror("receive");
	   			break;
	   		}
	   		if(recvbytes == 0)
	   			break;
	   		buffer[recvbytes] = '\0';
	   		cout<<"dec_client$: "<<buffer;
	   	}
	   	close(sockfd);
	}

}
////*****************************************************Below function is used for parsing the incoming request
void parse(string input, centerNode **root, int acceptId)
{
	vector<string> requestLine;
	int initial = 0;
	int next = input.find_first_of(";", initial);
	while(next>0) {
		requestLine.push_back(input.substr(initial,next-initial));
		initial = next+1;
		next = input.find_first_of(";",initial);
	}

	for(vector<string>::iterator it = requestLine.begin();it!=requestLine.end();++it) {
		vector<string> cmd;
		char *request_tokenizer = new char[(*it).size()+1];
		copy((*it).begin(),(*it).end(),request_tokenizer);
		request_tokenizer[(*it).size()] = '\0';
		char *tokens = request_tokenizer;
		tokens = strtok(request_tokenizer," ");
		while(tokens != NULL) {
			string s(tokens);
			cmd.push_back(s);
			tokens = strtok(NULL," ");
		}
		vector<string>::iterator iit = cmd.begin();
		transform((*iit).begin(),(*iit).end(),(*iit).begin(),::toupper);
		if((*iit).compare("INSERT") == 0) {
			bool flag = checkRequest(cmd,&(*root));
			if(flag == true) {
				string s = "response from server: Illegal Event, Insert Request Failed:";
				string temp = "dec_server$ : Illegal Event, Insert Request Failed:";
				if(logging)
					generatingLog(temp);
				else
					cout<<temp<<endl;

				if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
					perror("send");
				if (send(acceptId,"\n" ,1, 0) == -1)
					perror("send");
			} else {
				int dup = 0;
				for(++iit; iit!=cmd.end(); ++iit) {
					char from = (*iit).at(0);
					char to = (*iit).at(3);
					bool f = addNode(from,to, &(*root), dup);
					if(f) {
						string s = "response from server: Illegal Event, Insert Request Failed:";
						string temp = "dec_server$ : Illegal Event, Insert Request Failed:";
						if(logging)
							generatingLog(temp);
						else
							cout<<temp<<endl;
						if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
							perror("send");
						if (send(acceptId,"\n" ,1, 0) == -1)
							perror("send");
					}


				}
				if(dup) {
					string s = "response from server: Duplicate request rejected, other events Inserted Successfully";
					string temp = "dec_server$ : Duplicated Detected, Other Requests Inserted Successfully";
					if(logging)
						generatingLog(temp);
					else
						cout<<temp<<endl;
					if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
						perror("send");
					if (send(acceptId,"\n" ,1, 0) == -1)
						perror("send");


				}else {
					string temp = "dec_server$: Insert Done";
					if(logging)
						generatingLog(temp);
					else
						cout<<temp<<endl;


					string s = "response from server: Insert Successful";
					if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
						perror("send");
					if (send(acceptId,"\n" ,1, 0) == -1)
						perror("send");
				}

			}

		} else if((*iit).compare("QUERY") == 0) {
			for(++iit; iit!=cmd.end(); ++iit) {
				char from = (*iit).at(0);
				++iit;
				char to = (*iit).at(0);
				isPresent(from,to,&(*root),acceptId);
			}
		} else if((*iit).compare("RESET") == 0) {
			reset(&(*root));
			string temp = "dec_server$: Reset Done";
			if(logging)
				generatingLog(temp);
			else
				cout<<temp<<endl;

			string s = "response from server: Reset Done";
			if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
					perror("send");
			if (send(acceptId,"\n" ,1, 0) == -1)
					perror("send");
		}
		delete []request_tokenizer;
	}

}
void reset(centerNode **root)
{
	while((*root)->next != NULL) {
		centerNode *temp = *root;
		(*root) = (*root)->next;
		delete temp;
	}
	delete *root;
	*root = NULL;

}
bool checkRequest(vector<string> request, centerNode **root)
{

	vector<string>::iterator iit = request.begin();
	centerNode *cn = new centerNode;
	cn = NULL;
	int dup =0;
	bool flag = false;
	for(++iit; iit!=request.end(); ++iit) {
		char from = (*iit).at(0);
		char to = (*iit).at(3);
		if(from == to)
			return true;
		flag = addNode(from,to,&cn,dup);
		if(flag == true) {
			while(cn->next != NULL) {
				centerNode *temp = cn;
				cn = cn->next;
				delete temp;
			}
			delete cn;
			cn = NULL;
			return flag;
		}

	}
	if(cn != NULL) {
		while(cn->next != NULL) {
			centerNode *temp = cn;
			cn = cn->next;
			delete temp;
		}
			delete cn;
			cn = NULL;
	}
	iit = request.begin();
	for(++iit; iit!=request.end(); ++iit) {
			char from = (*iit).at(0);
			char to = (*iit).at(3);
			flag = checkRequestGraph(from, to, &(*root));
			if (flag)
				return flag;
	}
return false;
}
bool checkRequestGraph(char from, char to, centerNode **root)
{
	centerNode *temp = *root;
	while(temp != NULL) {
		if(temp->eventNo == from) {
			eventNode *parent = temp->parent;
			while(parent != NULL) {
				if(parent->eventNo == to)
					return true;
				parent = parent->next;
			}
			break;
		}
		temp = temp->next;
	}
	return false;
}
//*****************************************************Below function is used for adding new request
bool addNode(char from, char to, centerNode **root, int &dup)
{
	bool eventIllegal = false;
	if(*root==NULL) {				// when the event start root is null
		*root = new centerNode;
		(*root)->eventNo = from;
		(*root)->child = new eventNode;
		(*root)->child->eventNo = to;
		(*root)->child->next = NULL;
		(*root)->parent = NULL;
		centerNode *temp = *root;
		temp->next = new centerNode;
		temp = temp->next;
		temp->eventNo = to;
		temp->parent = new eventNode;
		temp->parent->eventNo = from;
		temp->parent->next = NULL;
		temp->next = NULL;
		temp->child = NULL;

	}else {

		eventIllegal=fromEventHandler(from,to,&(*root), dup);
	}
return eventIllegal;

}

//*****************************************************Below function is used for handling parent event

bool fromEventHandler(char from, char to, centerNode **root, int &duplic)
{
	centerNode *node = *root;
	centerNode *temp = node;
	int flag =0,illegal = 0;
	while(node != NULL) {
			if(node->eventNo == from) {	// checking if  "from" is already present in our list
				flag =1;
				break;
			}else {
				flag = 0;
			}
	temp = node;
	node = node->next;
	}
	if(flag) {
		eventNode *parent = node->parent;
		while(parent != NULL) {
			if(parent->eventNo == to) {				// checking if "to" is present in "from" parent list tht means event is illegal can't insert
				illegal = 1;
				break;
			}else {
				illegal = 0;
			}
			parent = parent->next;
		}
		if(!illegal) {
					if(node->child == NULL) {
						node->child = new eventNode;
						node->child->eventNo = to;
						node->child->next = NULL;
						toEventHandler(from,to,&(*root),node);		// pass this "from" node into this function so tht we have the trac of parent of "to"

					}else {
						int dup =0;
						eventNode *temp = node->child;
						eventNode *child = node->child;
						while(temp != NULL) {				// checking if the "to" is already present in "from" child list tht means duplicacy
							if (temp->eventNo == to) {
								dup = 1;
								break;
							}else {
								dup = 0;
							}
							child = temp;
							temp = temp->next;
						}
						if(dup) {
							duplic = 1;						// So that in the parse function we can detect duplicate
						}else {

							child->next = new eventNode;
							child = child->next;
							child->eventNo = to;
							child->next = NULL;
							toEventHandler(from,to,&(*root),node);		// pass this "from" node into this function so tht we have the trac of parent of "to"
						}
					}

		}else {
			bool eventIllegal = true;					// If event is illegal return true to function checkRequest
			return eventIllegal;
		}

	}else {								// if "from" is not fount in our main list.
		temp->next = new centerNode;
		temp = temp->next;
		temp->eventNo = from;
		temp->child = new eventNode;
		temp->child->eventNo = to;
		temp->child->next = NULL;
		temp->parent = NULL;
		temp->next = NULL;
		toEventHandler(from,to,&(*root),temp);

	}
return false;

}

//*****************************************************Below function is used for handling event child event
void toEventHandler(char from, char to, centerNode **root,centerNode *node)
{
	centerNode *down = *root;
	centerNode *temp = *root;
	int flag =0;
	while(down != NULL) {
		if(down->eventNo == to) {			// To check if "to" is already present in our main list
			flag = 1;
			break;
		}else {
			flag = 0;
		}
		temp = down;
		down = down->next;

	}
	if(flag) {
		int dup = 0;
		eventNode *tmp = down->parent;
		eventNode *parent = down->parent;
		while(tmp != NULL) {						//check if already "from" is present in the "to" parent list that means duplicacy
			if(tmp->eventNo == from) {
				dup = 1;
				break;
			}
			else {
				dup = 0;
			}
			parent = tmp;
			tmp = tmp->next;
		}
		if(!dup) {									// if not duplicacy then add the "from" node into "to" parent list and also all parent of "from"
			eventNode *fromParent = node->parent;
			parent->next = new eventNode;
			parent = parent->next;
			parent->eventNo = node->eventNo;
			parent->next = NULL;
			while(fromParent != NULL) {						// This part copies the "from" parents into "to" parents
				int duplicate = 0;
				eventNode *addThis;
				parent = down->parent;
				while(parent != NULL) {
					if(parent->eventNo == fromParent->eventNo) {
						duplicate =1;
						break;
					}
					else {
						duplicate =0;
					}
					addThis = parent;
					parent = parent->next;
				}
				if(!duplicate) {
					addThis->next = new eventNode;
					addThis = addThis->next;
					addThis->eventNo = fromParent->eventNo;
					addThis->next = NULL;
				}
				fromParent = fromParent->next;
			}
		}

	}else {												// if we dont have node present then create a new node for "to"
		eventNode *fromParent = node->parent;
		temp->next = new centerNode;
		temp = temp->next;
		temp->eventNo = to;
		temp->child = NULL;
		temp->parent = new eventNode;
		temp->parent->eventNo = node->eventNo;
		temp->parent->next = NULL;
		eventNode *parent = temp->parent;
		while(fromParent != NULL) {					// copying all the parent of "from" node to "to" node
				parent->next = new eventNode;
				parent = parent->next;
				parent->eventNo = fromParent->eventNo;
				parent->next = NULL;
				fromParent = fromParent->next;
		}

		temp->next = NULL;

	}
}

//*****************************************************Below function is used for searching in Graph
void isPresent(char from, char to, centerNode **root, int acceptId)
{
	centerNode *fromNode = *root;
	centerNode *toNode = *root;
	int flag1 = 0,flag2=0;
	while(fromNode != NULL) {
		if(fromNode->eventNo == from) {
			flag2 = 1;
			break;
		}else {
			flag2 =0;
		}
		fromNode = fromNode->next;
	}

	while(toNode != NULL) {
		if(toNode->eventNo == to) {
			flag1 = 1;
			break;
		}else {
			flag1 =0;
		}
		toNode = toNode->next;
	}

	if(flag1 && flag2) {
		int found = 0, found1=0;
		eventNode *toParent = toNode->parent;
		eventNode *fromParent = fromNode->parent;
		while(toParent != NULL) {
			if(toParent->eventNo == from) {
				found = 1;
				break;
			}else {
				found = 0;
			}
			toParent = toParent->next;
		}
		if(!found) {
			while(fromParent != NULL) {
				if(fromParent->eventNo == to) {
					found1 = 1;
					break;
				}else {
					found1 = 0;
				}
				fromParent = fromParent->next;
			}
			if(found1) {
				//cout<<to<<" happened before "<<from;
				stringstream ss;
				ss << "response from server: ";
				ss << to;
				ss << " happened before ";
				ss << from;
				string s = ss.str();
				//ss >> s;
				string temp = "dec_server$: "+s;
				if(logging)
					generatingLog(temp);
				else
					cout<<temp<<endl;

				if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
						perror("send");
				if (send(acceptId,"\n" ,1, 0) == -1)
						perror("send");
			}
		}else if(found) {
			//cout<<from<<" happend before "<<to;
			stringstream ss;
			ss << "response from server: ";
			ss << from;
			ss << " happened before ";
			ss << to;
			string s = ss.str();
			string temp = "dec_server$: "+s;
			if(logging)
				generatingLog(temp);
			else
				cout<<temp<<endl;


			//string s = "response from server: "+ from +" happened before "+to;
			if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
					perror("send");
			if (send(acceptId,"\n" ,1, 0) == -1)
					perror("send");
		}else {

			string temp = "dec_server$: Concurrent Events";
			if(logging)
				generatingLog(temp);
			else
				cout<<temp<<endl;

			string s = "response from server: Concurrent Events";
			if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
					perror("send");
			if (send(acceptId,"\n" ,1, 0) == -1)
					perror("send");
		}
	}else if(!flag1) {
		string temp = "dec_server$: Event Missing";
		if(logging)
			generatingLog(temp);
		else
			cout<<temp<<endl;
		stringstream ss;
		ss << "response from server: Event Not Found:";
		ss<<to;
		string s = ss.str();
		if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
				perror("send");
		if (send(acceptId,"\n" ,1, 0) == -1)
				perror("send");
	}else if(!flag2) {
		string temp = "dec_server$: Event Missing";
		if(logging)
			generatingLog(temp);
		else
			cout<<temp<<endl;

		stringstream ss;
		ss << "response from server: Event Not Found:";
		ss<<from;
		string s = ss.str();

		if (send(acceptId,s.c_str() , strlen(s.c_str()), 0) == -1)
				perror("send");
		if (send(acceptId,"\n" ,1, 0) == -1)
				perror("send");
	}
}

//*****************************************************Below function is used to return the port no of client
u_int16_t get_port_number(struct sockaddr *s)
{
	if(s->sa_family == AF_INET)
		return (((struct sockaddr_in  *)s)->sin_port);
	else
		return (((struct sockaddr_in6 *)s)->sin6_port);
}

//*****************************************************Below function is used to return ipaddress of client
void *get_ip_address(sockaddr *s)
{
	if(s->sa_family == AF_INET)
		return &((sockaddr_in *)s)->sin_addr;
	else
		return &((sockaddr_in6 *)s)->sin6_addr;
}
void signal_callback_handler(int signum)
{

	exit(signum);

}
void printSummary()
{
cout<<endl<<"**************************************************************************"<<endl<<endl;
cout<<"Usage Summary: Same file will act as a server and client based on parameter passed"<<endl;
cout<<"To Start Dec Server: ./decserver [-s] [−h] [-p port-number] [−l file]"<<endl;
cout<<"To Start Dec Client: ./decserver [-c ip of server] [-p port-number of server]"<<endl<<endl;
cout<<"Give -s parameter in args to indicate server mode"<<endl;
cout<<"Give -c parameter in args to indicate client mode, if -c is not given then it will connect to localhost by default"<<endl;
cout<<"Give -h parameter to display the summary"<<endl;
cout<<"Give -p and then portno to change default port number for example: -p 8080"<<endl;
cout<<"Give -l and then filename of logging file for example: logging.txt"<<endl;
cout<<"Press ctrl+c to exit the client and server anytime"<<endl;
cout<<endl<<"**************************************************************************"<<endl<<endl;
exit(1);
}

void generatingLog(string s)
{
	ofstream logfile;
	logfile.open(filename.c_str(), std::ios::app);
	if( !logfile )
	{ 						// file couldn't be opened
	    cout << "Error: file could not be opened" << endl;
	    //exit(1);
	}

	logfile<<s<<endl;
	logfile.close();


}
