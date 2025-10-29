# Trabalho prático 1 de Redes de Computadores

#Parte 1:

Foi feito um cliente HTTP simples, capaz de utilizar um socket para enviar uma requisição GET a um servidor e receber/salvar o arquivo presente no URL.
Isso foi feito da seguinte maneira:
- São iniciados três vetores de caracteres, que são preenchidos com a função parse_url, que armazena o host e o caminho, e em seguida a função get_filename armazena o nome do arquivo;
- Cria structs para armazenar o DNS do host e o endereço do servidor (IP, porta e família);
- Faz a resolução do DNS;
- Cria um socket TCP;
- Preenche o endereço do servidor;
- Estabelece a conexão para o IP definido;
- Cria e envia uma string contendo o request e o cabeçalho HTTP;
- Abre um arquivo para escrita e um loop para ler e copiar o arquivo do corpo para o arquivo criado;
- Por fim, fecha o socket e o arquivo.

#Parte 2:

Foi feito um servidor HTTP simples, capaz de resolver o caminho para um diretório raíz selecionado.
O servidor foi feito da seguinte maneira:

- Recebe o diretório base que será servido como argumento;
- Cria as structs para armazenar o socket do servidor, o socket do cliente e o endereço do servidor;
- Cria um socket TCP e configura para conectar na porta 8080;
- Associa o socket ao servidor local e coloca-o em modo de escuta;
- Entra em um loop infinito aceitando conexões de clientes;
- Ao receber uma conexão, lê a requisição HTTP enviada pelo cliente;
- Interpreta o método e o caminho do arquivo solicitado;
- Monta o caminho completo no sistema de arquivos a partir do diretório raíz selecionado e do caminho requisitado;
- Se o caminho for um diretório, verifica se existe um arquivo index.html e o envia ou gera uma listagem com os arquivos. Se for um arquivo, determina o tipo e envia seu conteúdo ao cliente;
- Caso o arquivo não exista, envia uma resposta de erro 404;
- Após o envio, fecha o socket do cliente e continua aguardando novas conexões.
