// MT25026 - Throughput Measurement Client (Gbps)
// Measures aggregate data throughput over a fixed duration

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define NUM_FIELDS 8   // Number of message segments per iteration

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

    long messages = 0;
    long long total_bytes = 0;

    time_t start = time(NULL);

    /* Receive data continuously for the specified duration */
    while (time(NULL) - start < duration) {
        for (int i = 0; i < NUM_FIELDS; i++) {
            ssize_t r = recv(sockfd, buf, msg_size, MSG_WAITALL);
            if (r <= 0) goto done;   // Exit on server close or error
            total_bytes += r;
        }
        messages++;

        /* Periodic progress output for long runs */
        if (messages % 100000 == 0) {
            double gb = (total_bytes * 8.0) / 1e9;
            printf("Messages=%ld | Data=%.3f Gb\n", messages, gb);
        }
    }

done:
    time_t end = time(NULL);
    double elapsed = difftime(end, start);

    /* Compute throughput in Gbps */
    double total_gb = (total_bytes * 8.0) / 1e9;
    double throughput_gbps = total_gb / elapsed;

    printf("\nFINAL:\n");
    printf("Messages=%ld\n", messages);
    printf("Data=%.3f Gb\n", total_gb);
    printf("Time=%.2f sec\n", elapsed);
    printf("Throughput=%.3f Gbps\n", throughput_gbps);

    free(buf);
    close(sockfd);
    return 0;
}