// MT25026 - A3 Server (MSG_ZEROCOPY + mmap)
// Zero-copy optimized implementation; thread-per-connection model

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
 * Sends data using sendmsg() with MSG_ZEROCOPY. Message buffers
 * are allocated using mmap() so the kernel can pin user pages
 * and allow the NIC to perform direct DMA from user memory.
 */
void *client_handler(void *arg) {
    thread_arg_t *targ = (thread_arg_t *)arg;
    int fd = targ->client_fd;
    int size = targ->msg_size;

    struct iovec iov[NUM_FIELDS];
    char *bufs[NUM_FIELDS];

    /* Allocate one mmap() buffer per field */
    for (int i = 0; i < NUM_FIELDS; i++) {
        bufs[i] = mmap(NULL,
                       size,
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1,
                       0);

        if (bufs[i] == MAP_FAILED) {
            perror("mmap failed");
            close(fd);
            free(targ);
            return NULL;
        }

        /* Initialize payload once; reused across sends */
        memset(bufs[i], 'A' + i, size);
        iov[i].iov_base = bufs[i];
        iov[i].iov_len  = size;
    }

    /* Message header for scatter-gather zero-copy transmission */
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = iov;
    msg.msg_iovlen = NUM_FIELDS;

    /* Continuously send data until client disconnects */
    while (1) {
        ssize_t ret = sendmsg(fd, &msg, MSG_ZEROCOPY);
        if (ret <= 0)
            break;   // Client closed connection or error
    }

    /* Unmap user buffers after transmission completes */
    for (int i = 0; i < NUM_FIELDS; i++)
        munmap(bufs[i], size);

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

    /* Enable kernel zero-copy support on the socket */
    int enable = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_ZEROCOPY,
               &enable, sizeof(enable));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, BACKLOG);

    printf("A3 Server listening on port %d | msg=%d | max_threads=%d\n",
           port, msg_size, max_threads);
    fflush(stdout);

    int active_threads = 0;

    /* Accept connections and spawn one worker thread per client */
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

    /* Keep server alive; experiment duration controlled by script */
    pause();
}