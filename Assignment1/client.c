# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<unistd.h>
# include<sys/types.h>
# include<sys/socket.h>
# include<netinet/in.h>
# include<arpa/inet.h>

# define PORT 6666
# define PKT_NUM 5
# define MAX_SIZE 2048
# define TIMEOUT_LIMIT 3

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(){
    int sock, i, attemp_times, ret, error_ret;
    unsigned int len_server;
    struct sockaddr_in server;
    char trans_buffer[PKT_NUM][MAX_SIZE];
    char buffer[MAX_SIZE];
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        error("[ERROR] Create socket error");
    }
    
    /* Server settings */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    len_server = sizeof(server);

    /* Read from .txt, two cases, normal_case or abnormal_case */
    FILE *fp = fopen("abnormal_case.txt", "r");
    if(fp == NULL){
        error("[ERROR] Failed to open the txt file...");
        return -1;
    }
    i = 0;
    while(!feof(fp)){
        fgets(trans_buffer[i], sizeof(trans_buffer[i]) - 1, fp);
        i++;
    }
    fclose(fp);

    /* Initialize the timer, with maximum 3 times for retry */
    i = 0;
    attemp_times = 0;
    struct timeval ack_timer;
    socklen_t len_t = sizeof(ack_timer);
    ack_timer.tv_sec = TIMEOUT_LIMIT;
    ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*) &ack_timer.tv_sec, len_t);
    if(ret < 0){
        error("[ERROR] Setting timer");
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    while(i < PKT_NUM){
        if(attemp_times >= 3){
            break;
        }

        ret = sendto(sock, trans_buffer[i], strlen(trans_buffer[i]), 0, (struct sockaddr *) &server, len_server);   
        if(ret < 0) error("Sending Error");
    
        bzero(buffer, MAX_SIZE);
        ret = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &server, &len_server);
        if(ret < 0){
            // check if it is timeout, if so, resend the current packet and accumulate attempt times
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &error_ret, &len_t);
            if(error_ret == 0){
                attemp_times += 1;
                printf("[WARNING] Timeout when receiving, retry %d out of 3\n", attemp_times);
            }
        }else{
            printf("%s", buffer);
            i++;
        }
    }

    if(attemp_times >= 3){
        error("\n[ERROR] Server does not respond...");
    }else{
        printf("\n[Log] Ready to close the client...");
    }
    close(sock);
    return 0;
}