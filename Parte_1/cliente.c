#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#define TAM_MAX_HOST 500
#define TAM_MAX_PATH 500
#define TAM_MAX_FILENAME 500
#define TAM_BUFFER 1024

void parse_url(const char *url, char *host, char *path, int *porta) {
    int i = 0, j = 0, k = 0;
    const char *protocolo = "http://";

    for (i = 0; protocolo[i] != '\0'; i++) {
        if (protocolo[i] != url[i]) {
            printf("Protocolo inválido.\n");
            exit(1);
        }
    }

    *porta = 80;

    j = 0;
    while (url[i] != '\0' && url[i] != '/' && url[i] != ':') {
        if (j < TAM_MAX_HOST - 1) {
            host[j] = url[i];
            j++;
        }
        i++;
    }
    host[j] = '\0';

    if (url[i] == ':') {
        i++;
        char porta_str[10];
        int p = 0;
        while (url[i] >= '0' && url[i] <= '9' && p < (int)sizeof(porta_str) - 1) {
            porta_str[p++] = url[i++];
        }
        porta_str[p] = '\0';
        *porta = atoi(porta_str);
    }

    if (url[i] == '/') {
        k = 0;
        while (url[i] != '\0' && k < TAM_MAX_PATH - 1) {
            path[k] = url[i];
            k++;
            i++;
        }
        path[k] = '\0';
    } else {
        strcpy(path, "/");
    }
}


void get_filename(char *path, char *filename) {
    int i = 0, j = 0;
    int ultima_barra = -1;

    while (path[i] != '\0') {
        if (path[i] == '/') {
            ultima_barra = i;
        }

        i++;
    }

    if (ultima_barra >= 0 && path[ultima_barra + 1] != '\0') {
        for (i = ultima_barra + 1; path[i] != '\0' && j < TAM_MAX_FILENAME - 1; i++) {
            filename[j] = path[i];
            j++;
        }
        
        filename[j] = '\0';
    } else {
        strcpy(filename, "index.html");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <URL>\n", argv[0]);
        return 1;
    }

    char host[TAM_MAX_HOST] = {}, path[TAM_MAX_PATH] = {}, filename[TAM_MAX_FILENAME] = {};
    int porta;
    parse_url(argv[1], host, path, &porta);
    get_filename(path, filename);

    printf("Host: %s\n", host);
    printf("Path: %s\n", path);
    printf("Arquivo: %s\n", filename);
    printf("Porta: %d\n", porta);

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd;
    char request[TAM_BUFFER];
    char buffer[TAM_BUFFER];
    FILE *fp;

    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "Erro: host não encontrado.\n");
        return 2;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Erro ao abrir socket");
        return 3;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(porta);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro na conexão.");
        close(sockfd);
        return 4;
    }

    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, host);

    if (write(sockfd, request, strlen(request)) < 0) {
        perror("Erro ao enviar requisição");
        close(sockfd);
        return 5;
    }

    fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Erro ao criar arquivo");
        close(sockfd);
        return 6;
    }

    int cabecalho_fim = 0;
    char *dados_inicio;
    int n;

    while ((n = read(sockfd, buffer, sizeof(buffer))) > 0) {
        if (!cabecalho_fim) {
            dados_inicio = strstr(buffer, "\r\n\r\n");
            if (dados_inicio != NULL) {
                cabecalho_fim = 1;
                dados_inicio += 4;
                fwrite(dados_inicio, 1, n - (dados_inicio - buffer), fp);
            }
        } else {
            fwrite(buffer, 1, n, fp);
        }
    }

    fclose(fp);
    close(sockfd);

    printf("Download concluído: %s\n", filename);
    return 0;
}
