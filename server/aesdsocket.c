#define _POSIX_C_SOURCE 200112L
#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <syslog.h>

#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <signal.h>

#define LISTEN_BACKLOG 64
#define BUFFER_SIZE 64
#define TMP_PATH "/var/tmp/aesdsocketdata"

static bool shutdown_signal = false;

static void signal_handler(int signal_number) {
    shutdown_signal = true;
}

int main(int argc, char** argv) {
    unsigned char buffer[BUFFER_SIZE];
    assert(sizeof(unsigned char) == 1);

    if(argc > 2) {
        fprintf(stderr, "Usage: %s [-d]", argv[0]);
        exit(-1);
    }

    bool is_daemon = false;
    if(argc == 2) {
        if(strncmp(argv[1], "-d", 3) == 0) {
            is_daemon = true;
        } else {
            fprintf(stderr, "Usage: %s [-d]", argv[0]);
            exit(-1);
        }
    }

    openlog(NULL, 0, LOG_USER);

    // setup signal hander
    struct sigaction sig_act;
    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = signal_handler;
    if(sigaction(SIGTERM, &sig_act, NULL) || sigaction(SIGINT, &sig_act, NULL)) {
        syslog(LOG_ERR, "Cannot register for signal handlers");
        exit(-1);
    }

    // open socket to port 9000; exit with -1 if fails
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);

    struct addrinfo hints;
    struct addrinfo *servinfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    
    if(getaddrinfo(NULL, "9000", &hints, &servinfo)) {
        return -1;
    }

    int reuseaddr = 1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr)) == -1) {
        syslog(LOG_ERR, "Could not set socket option");
        return -1;
    }

    if(bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen)) {
        syslog(LOG_ERR, "Could not bind:");
        return -1;
    }

    free(servinfo);

    if(is_daemon) {
        if(daemon(0,0) == -1) {
            syslog(LOG_ERR, "Could not fork daemon");
            fprintf(stderr, "Could not fork daemon");
            exit(-1);
        }
    }

    while(!shutdown_signal) {
        // Listen + Accept connection
        if(listen(socket_fd, LISTEN_BACKLOG)) {
            syslog(LOG_ERR, "Could not listen to socket");
            return -1;
        }

        struct sockaddr addr;
        socklen_t addrlen = sizeof(addr);
        int server_fd = accept(socket_fd, &addr, &addrlen);
        if(server_fd < 0) {
            syslog(LOG_ERR, "Could not accept connection");
            continue;
        }

        // Log accepted: Accepted connection from $IP
        char ipAddrStr[INET_ADDRSTRLEN];
        struct sockaddr_in *addr_in = (struct sockaddr_in*) &addr;

        if(inet_ntop(AF_INET, &(addr_in->sin_addr), ipAddrStr, INET_ADDRSTRLEN) == NULL) {
            syslog(LOG_ERR, "Could not convert ip to string");
            exit(-1);
        }
        else
            syslog(LOG_DEBUG, "Accepted connection from %s", ipAddrStr);

        // recive from connection and append to /var/tmp/aesdsocketdata
        // packet is terminated by '\n', does not contain '\0'
        FILE* output_file = fopen(TMP_PATH, "a+");
        ssize_t bytes_received;
        do {
            bytes_received = read(server_fd, buffer, BUFFER_SIZE);
            if(bytes_received < 0) {
                syslog(LOG_ERR, "Reading from socket failed: %s", strerror(errno));
                return -1;
            }
            ssize_t bytes_written = fwrite(buffer, 1, bytes_received, output_file);
            if(bytes_written < bytes_received) {
                syslog(LOG_ERR, "Could not write received bytes to buffer");
                return -1;
            }
        } while(memchr(buffer, '\n', bytes_received) == NULL);
        assert(buffer[bytes_received - 1] == '\n'); // check assumption: last char is \n

        // after packet reception, echo back the whole file data to the sender
        fflush(output_file);
        rewind(output_file);
        do {
            ssize_t bytes_remaining = fread(buffer, 1, BUFFER_SIZE, output_file);
            ssize_t total_received = 0;
            while(bytes_remaining > 0) {
                ssize_t written = write(server_fd, buffer + total_received, bytes_remaining);
                if(written == -1) {
                    syslog(LOG_ERR, "Writing of data failed");
                    return -1;
                }
                total_received += written;
                bytes_remaining -= written;
            }
        } while(feof(output_file) == 0);
        fclose(output_file);

        // syslog: Closed connection from $IP
        syslog(LOG_DEBUG, "Closed connection from %s", ipAddrStr);  
        close(server_fd);
    }
    
    close(socket_fd);

    unlink(TMP_PATH);

    return EXIT_SUCCESS;
}
