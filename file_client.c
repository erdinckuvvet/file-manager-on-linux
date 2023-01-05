#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PIPE_NAME "/tmp/file_manager"
#define BUFFER_LENGTH 1024

char buffer[BUFFER_LENGTH];

int _write_pipe(char *pipeName, char *msg)
{
    int fd = open(pipeName, O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    if (write(fd, msg, strlen(msg) + 1) < 0)
    {
        perror("write");
        return -1;
    }
    close(fd);
    printf("%s sended to pipe\n", msg);
    return 0;
}

int _read_pipe(char *pipeName)
{
    int fd = open(pipeName, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }
    size_t len = read(fd, buffer, BUFFER_LENGTH);
    if (len < 0)
    {
        perror("write");
        return -1;
    }
    close(fd);

    return len;
}

int main()
{
    // file_manager ile iletişim kurmak için named pipe açılıyor

    _write_pipe(PIPE_NAME, "baglan");

    // file_manager'tan gelen işlem sonuçlarını alır uzunluğunu da bir değişkende tutar
    size_t pipeNameLen = _read_pipe(PIPE_NAME);
    char pipeName[pipeNameLen];

    // gelen komut pipeName olarak alınır
    strcpy(pipeName, buffer);

    // tüm pipelar kullanımda ise
    if (strcmp("not enough pipe", pipeName) == 0)
    {
        printf("CLIENT: not enough pipe, closing...");
        return 0;
    }

    // kullanıcıdan girdi alarak file_manager'a gönderilecek komutları işle
    while (1)
    {
        // komutu al
        printf("Enter command (create, delete, read, write, exit): \n");
        fgets(buffer, BUFFER_LENGTH, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; //\n ilk görüldüğü yeri \0 ile değiştirir
        printf("entered buffer =>%s\n", buffer);

        _write_pipe(pipeName, buffer);

        // "exit" komutu işlendiğinde döngüden çık
        if (strcmp(buffer, "exit") == 0)
            break;

        // işlem sonucunu al ve yazdır
        _read_pipe(pipeName);
        printf("Result: %s\n", buffer);
    }

    return 0;
}