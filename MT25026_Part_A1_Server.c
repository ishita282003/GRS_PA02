// MT25026 - A1 Server (send)
// Thread-per-connection server; duration controlled externally by client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "MT25026_common.h"

#define BACKLOG 10   // Maximum pending connections in listen queue

/* Arguments passed to each worker thread */
typedef struct {
    int client_fd;   // Connected client socket
    int msg_size;    // Size of each message field
} thread_arg_t;

/* 
 * Allocate and initialize a message consisting of NUM_FIELDS
 * heap-allocated buffers of equal size.
 */
message_t *create_message(int size) {
    message_t *msg = malloc(sizeof(message_t));
    msg->field_size = size;

    for (int i = 0; i < NUM_FIELDS; i++) {
        msg->fields[i] = malloc(size);
        memset(msg->fields[i], 'A' + i, size); // Fill with distinct data
    }
    return msg;
}

/* Free all heap memory associated with a message */
void destroy_message(message_t *msg) {
    for (int i = 0; i < NUM_FIELDS; i++)
        free(msg->fields[i]);
    free(msg);
}

/* 
 * Thread handler: continuously sends message fields to a single client
 * until the connection is closed or an error occurs.
 */
void *client_handler(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    int fd = targ->client_fd;
    int size = targ->msg_size;

    message_t *msg = create_message(size);

    while (1) {
        for (int i = 0; i < NUM_FIELDS; i++) {
            ssize_t sent = send(fd, msg->fields[i], size, 0);
            if (sent <= 0)
                goto cleanup;  // Exit on client disconnect or error
        }
    }

cleanup:
    close(fd);
    destroy_message(msg);
    free(targ);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <port> <msg_size> <max_threads>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    int msg_size = atoi(argv[2]);
    int max_threads = atoi(argv[3]);

    /* Create TCP server socket */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, BACKLOG);

    printf("A1 Server listening on port %d | msg=%d | max_threads=%d\n",
           port, msg_size, max_threads);

    int active_threads = 0;

    /* Accept connections and spawn one thread per client */
    while (active_threads < max_threads) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0)
            continue;

        thread_arg_t *arg = malloc(sizeof(thread_arg_t));
        arg->client_fd = client_fd;
        arg->msg_size = msg_size;

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, arg);
        pthread_detach(tid);   // No join required

        active_threads++;
    }

    /* Keep server alive; experiment duration is controlled externally */
    pause();
}