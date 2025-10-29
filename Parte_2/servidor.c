#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORTA 8080
#define TAM_BUFFER 1024

void send_file(int cliente, const char *path_completo, const char *tipo) {
    FILE *fp = fopen(path_completo, "rb");
    if (fp == NULL) {
        char resposta[] = "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\n\r\n <h1>404 - Arquivo não encontrado</h1>";
        write(cliente, resposta, strlen(resposta));
        return;
    }

    char cabecalho[TAM_BUFFER];
    snprintf(cabecalho, sizeof(cabecalho), "HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", tipo);
    write(cliente, cabecalho, strlen(cabecalho));

    char buffer[TAM_BUFFER];
    int n;
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        write(cliente, buffer, n);
    }

    fclose(fp);
}

void list_dir(int cliente, const char *diretorio, const char *url_atual) {
    DIR *dir;
    struct dirent *entrada;

    char resposta[TAM_BUFFER * 2];
    snprintf(resposta, sizeof(resposta),
             "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n <html><body><h2>Listagem de arquivos em %s</h2><ul>", url_atual);

    write(cliente, resposta, strlen(resposta));

    dir = opendir(diretorio);
    if (dir != NULL) {
        while ((entrada = readdir(dir)) != NULL) {
            if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0)
                continue;

            char linha[TAM_BUFFER];

            if (strcmp(url_atual, "/") == 0)
                snprintf(linha, sizeof(linha), "<li><a href=\"/%s\">%s</a></li>", entrada->d_name, entrada->d_name);
            else
                snprintf(linha, sizeof(linha), "<li><a href=\"%s/%s\">%s</a></li>", url_atual, entrada->d_name, entrada->d_name);

            write(cliente, linha, strlen(linha));
        }
        closedir(dir);
    }

    char fim[] = "</ul></body></html>";
    write(cliente, fim, strlen(fim));
}

void tratar_req(int cliente, const char *base_dir) {
    char buffer[TAM_BUFFER];
    int n = read(cliente, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        close(cliente);
        return;
    }
    buffer[n] = '\0';

    char metodo[10], path[500];
    sscanf(buffer, "%s %s", metodo, path);

    if (strcmp(metodo, "GET") != 0) {
        char resposta[] = "HTTP/1.0 405 Method Not Allowed\r\n\r\n";
        write(cliente, resposta, strlen(resposta));
        close(cliente);
        return;
    }

    char path_completo[1000];
    snprintf(path_completo, sizeof(path_completo), "%s%s", base_dir, path);

    struct stat info;
    if (stat(path_completo, &info) < 0) {
        char resposta[] = "HTTP/1.0 404 Not Found\r\n\r\n<h1>404 Not Found</h1><p>Arquivo não encontrado.</p>";
        write(cliente, resposta, strlen(resposta));
        close(cliente);
        return;
    }

    if (S_ISDIR(info.st_mode)) {
        char index_path[1050];
        snprintf(index_path, sizeof(index_path), "%s/index.html", path_completo);

        if (stat(index_path, &info) == 0) {
            send_file(cliente, index_path, "text/html");
        } else {
            list_dir(cliente, path_completo, path);
        }
    } else {
        const char *tipo = "application/octet-stream";
        if (strstr(path_completo, ".html")) tipo = "text/html";
        else if (strstr(path_completo, ".jpg")) tipo = "image/jpeg";
        else if (strstr(path_completo, ".png")) tipo = "image/png";
        else if (strstr(path_completo, ".gif")) tipo = "image/gif";
        else if (strstr(path_completo, ".txt")) tipo = "text/plain";

        send_file(cliente, path_completo, tipo);
    }

    close(cliente);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <diretório raiz>\n", argv[0]);
        return 1;
    }

    const char *diretorio_base = argv[1];

    int servidor, cliente;
    struct sockaddr_in addr_servidor, addr_cliente;
    socklen_t tam_cliente = sizeof(addr_cliente);

    servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor < 0) {
        perror("Erro ao criar socket");
        return 2;
    }

    int opt = 1;
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr_servidor, 0, sizeof(addr_servidor));
    addr_servidor.sin_family = AF_INET;
    addr_servidor.sin_addr.s_addr = INADDR_ANY;
    addr_servidor.sin_port = htons(PORTA);

    if (bind(servidor, (struct sockaddr *)&addr_servidor, sizeof(addr_servidor)) < 0) {
        perror("Erro no bind");
        close(servidor);
        return 3;
    }

    listen(servidor, 5);
    printf("Servidor HTTP rodando na porta %d\n", PORTA);
    printf("Diretório selecionado: %s\n", diretorio_base);

    while (1) {
        cliente = accept(servidor, (struct sockaddr *)&addr_cliente, &tam_cliente);
        if (cliente < 0) {
            perror("Erro no accept");
            continue;
        }

        tratar_req(cliente, diretorio_base);
    }

    close(servidor);
    return 0;
}
