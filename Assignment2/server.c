# include<stdio.h>
# include<stdlib.h>
# include<string.h>
# include<unistd.h>
# include<sys/types.h>
# include<sys/socket.h>
# include<netinet/in.h>
# include<arpa/inet.h>
# include <ctype.h>

# define PORT 8888
# define MAX_SIZE 2048
# define RECORD_NUM 3

# define LOG_ACK "[ACKNOWLEDGEMENT] "
# define REJ_ACK "[REJECT] "
# define UNPAID_ERROR "Unpaid subsriber "
# define NOT_EXIST_ERROR "Subscriber does not exist "
# define PERMIT_LOG "Access permitted "

# define UNPAID_CODE "FFF9"
# define NOT_EXIST_CODE "FFFA"
# define PERMIT_CODE "FFFB"

# define REPONSE_HEADER "0xFFFF"
# define END_PACKET "FFFF\n"


char read_buffer[MAX_SIZE], send_buffer[MAX_SIZE];
char records[RECORD_NUM][MAX_SIZE], temp_buffer[MAX_SIZE], number[MAX_SIZE];
char cliend_id[MAX_SIZE], segment_num[MAX_SIZE], length[MAX_SIZE], tech[MAX_SIZE];
char record_number[MAX_SIZE], record_tech[MAX_SIZE], record_paid[MAX_SIZE];
int i = 0;

void error(char *msg){
    perror(msg);
    exit(1);
}

/* Clear the buffer before receiving message */
void clear_buffer(){
    bzero(read_buffer, MAX_SIZE);
    bzero(send_buffer, MAX_SIZE);
    bzero(cliend_id, MAX_SIZE);
    bzero(segment_num, MAX_SIZE);
    bzero(length, MAX_SIZE);
    bzero(tech, MAX_SIZE);
}

/* Cast decimal number to hex */
void cast_to_hex(char format[], int num){
    sprintf(number, format, num);
    for(int i = 0; i < sizeof(number); i++)
        number[i] = toupper(number[i]);
}


int validate_packet(char *packet){
    setvbuf(stdout, NULL, _IONBF, 0);
    strncpy(cliend_id, packet + 6, 2);
    strncpy(segment_num, packet + 12, 2);
    strncpy(length, packet + 14, 2);
    strncpy(tech, packet + 16, 2);
    strncpy(number, packet + 18, 8);

    for(int x = 0; x < 3; x++){
        bzero(record_number, MAX_SIZE);
        bzero(record_tech, MAX_SIZE);
        bzero(record_paid, MAX_SIZE);
        strncpy(record_number, records[x], 8);
        strncpy(record_tech, records[x] + 9, 2);
        strncpy(record_paid, records[x] + 12, 1);
        setvbuf(stdout, NULL, _IONBF, 0);
        
        if(strcmp(record_number, number) == 0){
            if(strcmp(record_tech, tech) == 0){
                if(strcmp(record_paid, "1") == 0){
                    return 0;
                }else{
                    return 1;
                }
            }
        }
    }
    return 2;
}

/* Generate different response based on status number */
void generate_response(int status){
    switch(status){
        int len_prefix = 0;
        case 0:
            strcpy(send_buffer, LOG_ACK);
            strcat(send_buffer, PERMIT_LOG);
            strcat(send_buffer, REPONSE_HEADER);
            strcat(send_buffer, cliend_id);
            strcat(send_buffer, PERMIT_CODE);
            strcat(send_buffer, segment_num);
            strcat(send_buffer, length);
            strcat(send_buffer, tech);
            strcat(send_buffer, number);
            strcat(send_buffer, END_PACKET);
            break;
        case 1:
            strcpy(send_buffer, REJ_ACK);
            strcat(send_buffer, UNPAID_ERROR);
            strcat(send_buffer, REPONSE_HEADER);
            strcat(send_buffer, cliend_id);
            strcat(send_buffer, UNPAID_CODE);
            strcat(send_buffer, segment_num);
            strcat(send_buffer, length);
            strcat(send_buffer, tech);
            strcat(send_buffer, number);
            strcat(send_buffer, END_PACKET);
            break;
        case 2:
            strcpy(send_buffer, REJ_ACK);
            strcat(send_buffer, NOT_EXIST_ERROR);
            strcat(send_buffer, REPONSE_HEADER);
            strcat(send_buffer, cliend_id);
            strcat(send_buffer, NOT_EXIST_CODE);
            strcat(send_buffer, segment_num);
            strcat(send_buffer, length);
            strcat(send_buffer, tech);
            strcat(send_buffer, number);
            strcat(send_buffer, END_PACKET);
            break;
    }
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
    
    FILE *fp = fopen("verification_database.txt", "r");
    if(fp == NULL){
        error("[ERROR] Failed to open the txt file...");
        return -1;
    }
    i = 0;
    while(!feof(fp)){
        bzero(temp_buffer, MAX_SIZE);
        bzero(number, MAX_SIZE);
        fgets(temp_buffer, sizeof(temp_buffer), fp);
        temp_buffer[strcspn(temp_buffer, "\n")] = '\0';

        strncpy(number, temp_buffer, 10);
        cast_to_hex("%lx", atoi(number));
        
        strcat(records[i], number);
        strcat(records[i], temp_buffer + 10);
        i++;
    }
    fclose(fp);

    /* Show what have been stored in database */
    for(int i = 0; i < 3; i++){
        printf("%s\n", records[i]);
        fflush(stdout);
    }

    setvbuf(stdout, NULL, _IONBF, 0);
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

