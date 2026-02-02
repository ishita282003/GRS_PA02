// MT25026 - A2 Server (sendmsg + mmap pre-registered buffer)
// One-copy optimized implementation using scatter-gather I/O

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/mman.h>

#define NUM_FIELDS 8
#define BACKLOG 10   // Maximum pending connections in listen queue

/* Arguments passed to each worker thread */
typedef struct {
    int client_fd;   // Connected client socket
    int msg_size;    // Size of each message field
} thread_arg_t;

/*
 * Thread handler:
 * Sends a composite message to a single client using sendmsg()
 * with scatter-gather I/O. A single mmap()-allocated buffer is
 * reused to avoid intermediate user-space concatenation.
 */
void *client_handler(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    int fd = targ->client_fd;
    int size = targ->msg_size;

    /* Allocate one contiguous buffer using mmap() */
    size_t total_size = NUM_FIELDS * size;
    char *send_buffer = mmap(NULL,
                             total_size,
                             PROT_READ | PROT_WRITE,
                             MAP_PRIVATE | MAP_ANONYMOUS,
                             -1,
                             0);

    if (send_buffer == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        free(targ);
        return NULL;
    }

    /* Initialize buffer once; reused across all sends */
    for (int i = 0; i < NUM_FIELDS; i++) {
        memset(send_buffer + i * size, 'A' + i, size);
    }

    /* iovec entries reference different regions of the same buffer */
    struct iovec iov[NUM_FIELDS];
    for (int i = 0; i < NUM_FIELDS; i++) {
        iov[i].iov_base = send_buffer + i * size;
        iov[i].iov_len  = size;
    }

    /* Message header for scatter-gather transmission */
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = NUM_FIELDS;

    /* Continuously send data until client disconnects */
    while (1) {
        ssize_t sent = sendmsg(fd, &msg, 0);
        if (sent <= 0)
            break;   // Client closed connection or error
    }

    /* Cleanup resources */
    munmap(send_buffer, total_size);
    close(fd);
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

    /* Allow fast port reuse between experiments */
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, BACKLOG);

    printf("A2 Server listening on port %d | msg=%d | max_threads=%d\n",
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
        pthread_detach(tid);

        active_threads++;
    }

    /* Keep server alive; experiment duration controlled by client/script */
    pause();
}