// MT25026 - Clean Client for perf
// Minimal client used to avoid adding measurement overhead during profiling

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define NUM_FIELDS 8   // Number of message segments expected per iteration

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <server_ip> <port> <msg_size> <duration_sec>\n", argv[0]);
        exit(1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int msg_size = atoi(argv[3]);
    int duration = atoi(argv[4]);

    /* Create TCP socket and connect to server */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv = {0};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serv.sin_addr);

    connect(sockfd, (struct sockaddr *)&serv, sizeof(serv));

    /* Reusable receive buffer */
    char *buf = malloc(msg_size);

    /* Receive data continuously for the specified duration */
    time_t start = time(NULL);
    while (time(NULL) - start < duration) {
        for (int i = 0; i < NUM_FIELDS; i++) {
            recv(sockfd, buf, msg_size, MSG_WAITALL);
        }
    }

    /* Cleanup */
    free(buf);
    close(sockfd);
}