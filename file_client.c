#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PIPE_NAME "file_manager"

int write_pipe(char *pipeName, char *msg)
{
    int fd = open(pipeName, O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    if (write(pipeName, msg, sizeof(msg)) < 0)
    {
        perror("write");
        return -1;
    }
    close(fd);

    return 0;
}

int read_pipe(char *pipeName, char *msg)
{
    int fd = open(pipeName, O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        return -1;
    }

    if (read(pipeName, msg, sizeof(msg)) < 0)
    {
        perror("write");
        return -1;
    }
    close(fd);

    return 0;
}

int main()
{
    // file_manager ile iletişim kurmak için named pipe'ı açın
    int pipe_fd = open(PIPE_NAME, O_WRONLY);
    if (pipe_fd < 0)
    {
        perror("open");
        exit(1);
    }

    // file_manager'tan gelen işlem sonuçlarını alın
    char response[10];
    if (read(pipe_fd, response, sizeof(response)) < 0)
    {
        perror("read");
        exit(1);
    }

    close(pipe_fd);

    // kullanıcıdan girdi alarak file_manager'a gönderilecek komutları işleyin
    while (1)
    {
        // komutu alın
        char command[256];
        printf("Enter command (create, delete, read, write, exit): ");
        scanf("%s", command);

        // "exit" komutu işlendiğinde döngüden çıkın
        if (strcmp(command, "exit") == 0)
        {
            write_pipe(response, "exit");
            break;
        }

        write_pipe(response, command);

        // işlem sonucunu alın ve yazdırın
        if (read(pipe_fd, response, sizeof(response)) < 0)
        {
            perror("read");
            exit(1);
        }
        printf("Result: %s\n", response);
    }

    return 0;
}