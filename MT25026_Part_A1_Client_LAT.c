// MT25026 - Latency Measurement Client
// Measures application-level receive latency over a fixed duration

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <limits.h>

#define NUM_FIELDS 8
#define NS_PER_US 1000.0   // Nanoseconds per microsecond

/* Compute time difference between two timestamps in nanoseconds */
static inline long diff_ns(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000000000L +
           (b.tv_nsec - a.tv_nsec);
}

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

    /* Single receive buffer reused for all fields */
    char *buf = malloc(msg_size);

    long count = 0;
    long min_lat = LONG_MAX;
    long max_lat = 0;
    long total_lat = 0;

    time_t start_time = time(NULL);

    /* Run latency measurement for the specified duration */
    while (time(NULL) - start_time < duration) {
        struct timespec t1, t2;
        clock_gettime(CLOCK_MONOTONIC, &t1);

        /* Receive all NUM_FIELDS message segments */
        for (int i = 0; i < NUM_FIELDS; i++) {
            recv(sockfd, buf, msg_size, MSG_WAITALL);
        }

        clock_gettime(CLOCK_MONOTONIC, &t2);

        /* Compute per-message latency in microseconds */
        long lat_ns = diff_ns(t1, t2);
        long lat_us = lat_ns / NS_PER_US;

        count++;
        total_lat += lat_us;
        if (lat_us < min_lat) min_lat = lat_us;
        if (lat_us > max_lat) max_lat = lat_us;
    }

    /* Report latency statistics */
    printf("\nLatency Results (microseconds):\n");
    printf("Avg: %.2f us\n", (double)total_lat / count);
    printf("Min: %ld us\n", min_lat);
    printf("Max: %ld us\n", max_lat);

    free(buf);
    close(sockfd);
}