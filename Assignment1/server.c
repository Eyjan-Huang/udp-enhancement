# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<unistd.h>
# include<sys/types.h>
# include<sys/socket.h>
# include<netinet/in.h>
# include<arpa/inet.h>

# define PORT 6666
# define MAX_SIZE 2048
# define LEN_CLIENT_ID 2
# define LEN_RECEIVE_SEGMENT 2
# define LEN_REJ_SUBCODE 4
# define LEN_SEGMENT 2
# define LEN_PAYLOAD_LENGTH 2
# define LEN_END_ID 4
# define LEN_FIXED_POSITION 20

# define END_ID "FFFF"

# define OUT_OF_SEQUENCE_CODE "FFF4"
# define LENGTH_MISMATCH_CODE "FFF5"
# define END_PACKET_MISSING_CODE "FFF6"
# define DUPLICATE_PACKET_CODE "FFF7"

# define OUT_OF_SEQUENCE_ERROR "Out of sequence "
# define LENGTH_MISMATCH_ERROR "Length mismatch "
# define END_PACKET_MISSING_ERROR "End packet_id missing "
# define DUPLICATE_PACKET_ERROR "Duplicate packet "

# define LOG_ACK "[ACKNOWLEDGEMENT] "
# define REJ_ACK "[REJECT] "
# define STANDARD_ACK "0xFFFF00FFF200FFFF\n"
# define STANDARD_REJ "0xFFFF00FFF3000000FFFF\n"



int segment_number = -1;
int len_payload = 0;
char read_buffer[MAX_SIZE], send_buffer[MAX_SIZE];
char length[LEN_PAYLOAD_LENGTH], end_id[LEN_END_ID];
char client_id[LEN_CLIENT_ID], received_segment[LEN_RECEIVE_SEGMENT], rej_subcode[LEN_REJ_SUBCODE];

void error(char *msg){
    perror(msg);
    exit(1);
}

/* Clear all the buffers before receiving the data from the client */
void clear_buffer(){
    bzero(read_buffer, MAX_SIZE);
    bzero(send_buffer, MAX_SIZE);
    bzero(length, LEN_PAYLOAD_LENGTH);
    bzero(client_id, LEN_CLIENT_ID);
    bzero(received_segment, LEN_RECEIVE_SEGMENT);
    bzero(rej_subcode, LEN_REJ_SUBCODE);
}

/* Get the length of the header */
int get_log_length(char *msg){
    return strlen(msg);
}

/* Get the number in decimal format from hex */
int get_decimal(char *num){
    return (int) strtol(num, NULL, 16);
}

/* Generate different response based on the sending packet */
void generate_response(int status){
    switch(status){
        int len_prefix = 0;
        case 0:
            strcpy(send_buffer, LOG_ACK);
            len_prefix = get_log_length(send_buffer);
            strncpy(send_buffer + len_prefix, STANDARD_ACK, strlen(STANDARD_ACK));
            strncpy(send_buffer + len_prefix + 6, client_id, 2);
            strncpy(send_buffer + len_prefix + 12, received_segment, 2);
            break;
        case 4:
            strcpy(send_buffer, REJ_ACK);
            strcat(send_buffer, OUT_OF_SEQUENCE_ERROR);
            len_prefix = get_log_length(send_buffer);
            strncpy(send_buffer + len_prefix, STANDARD_REJ, strlen(STANDARD_REJ));
            strncpy(send_buffer + len_prefix + 6, client_id, 2);
            strncpy(send_buffer + len_prefix + 12, OUT_OF_SEQUENCE_CODE, 4);
            strncpy(send_buffer + len_prefix + 16, received_segment, 2);
            break;
        case 5:
            strcpy(send_buffer, REJ_ACK);
            strcat(send_buffer, LENGTH_MISMATCH_ERROR);
            len_prefix = get_log_length(send_buffer);
            strncpy(send_buffer + len_prefix, STANDARD_REJ, strlen(STANDARD_REJ));
            strncpy(send_buffer + len_prefix + 6, client_id, 2);
            strncpy(send_buffer + len_prefix + 12, LENGTH_MISMATCH_CODE, 4);
            strncpy(send_buffer + len_prefix + 16, received_segment, 2);
            break;
        case 6:
            strcpy(send_buffer, REJ_ACK);
            strcat(send_buffer, END_PACKET_MISSING_ERROR);
            len_prefix = get_log_length(send_buffer);
            strncpy(send_buffer + len_prefix, STANDARD_REJ, strlen(STANDARD_REJ));
            strncpy(send_buffer + len_prefix + 6, client_id, 2);
            strncpy(send_buffer + len_prefix + 12, END_PACKET_MISSING_CODE, 4);
            strncpy(send_buffer + len_prefix + 16, received_segment, 2);
            break;
        case 7:
            strcpy(send_buffer, REJ_ACK);
            strcat(send_buffer, DUPLICATE_PACKET_ERROR);
            len_prefix = get_log_length(send_buffer);
            strncpy(send_buffer + len_prefix, STANDARD_REJ, strlen(STANDARD_REJ));
            strncpy(send_buffer + len_prefix + 6, client_id, 2);
            strncpy(send_buffer + len_prefix + 12, DUPLICATE_PACKET_CODE, 4);
            strncpy(send_buffer + len_prefix + 16, received_segment, 2);
            break;
    }

}


/* Validate if the packets are in correct format */
int validate_packet(char *packet){
    strncpy(client_id, packet + 6, 2);
    strncpy(end_id, packet + strlen(packet) - 4, 4);
    strncpy(received_segment, packet + 12, 2);

    if(strcmp(end_id, END_ID) != 0){
        return 6;
    }

    if(segment_number != -1){
        if(get_decimal(received_segment) == segment_number){
            return 7;
        }
        else if(get_decimal(received_segment) != segment_number + 1){
            return 4;
        }
    }

    strncpy(length, packet + 14, 2);
    if(get_decimal(length) * 2 != (strlen(packet) - LEN_FIXED_POSITION)){
        return 5;
    }

    segment_number = get_decimal(received_segment);
    return 0;
}


int main(){
    int sock, n;
    socklen_t len_server, len_client;
    struct sockaddr_in server, client;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0){
        error("Create Socket Error");
    }
    printf("[SUCCESSFUL]: Create socket\n");

    len_server = sizeof(server);
    len_client = sizeof(client);

    /* Clear everything in socket address */
    bzero(&server, len_server);
    bzero(&client, len_client);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    /* Try to bind the socket */
    if(bind(sock, (struct sockaddr *) &server, len_server) < 0){
        error("Binding Error");
    }
    printf("[SUCCESSFUL]: Binding\n");
    
    while (1){
        clear_buffer();

        n = recvfrom(
            sock,
            read_buffer,
            MAX_SIZE,
            0,
            (struct sockaddr *) &client,
            &len_client
        );
        if(n < 0){
            error("Receive Data Error");
        }

        // Replace the last bit to \0 which represents the end of the string
        read_buffer[strcspn(read_buffer, "\n")] = '\0';

        int packet_status = validate_packet(read_buffer);
        generate_response(packet_status);

        n = sendto(
            sock,
            send_buffer, 
            strlen(send_buffer),
            0,
            (struct sockaddr *) &client,
            len_client
        );
        if(n < 0){
            error("Sending Error");
        }
    }

    printf("\n[Log]: Ready to close the server...");
    close(sock);
    return 0;
}

