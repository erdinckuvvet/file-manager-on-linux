#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PIPE_NAME "file_manager"

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
    char response[20];
    if (read(pipe_fd, response, sizeof(response)) < 0)
    {
        perror("read");
        exit(1);
    }

    // kullanıcıdan girdi alarak file_manager'a gönderilecek komutları işleyin
    while (1)
    {
        // komutu alın
        char command[10];
        printf("Enter command (create, delete, read, write, exit): ");
        scanf("%s", command);

        // "exit" komutu işlendiğinde döngüden çıkın
        if (strcmp(command, "exit") == 0)
        {
            break;
        }

        // komutu file_manager'a gönderin
        if (write(pipe_fd, command, sizeof(command)) < 0)
        {
            perror("write");
            exit(1);
        }

        // create, delete, read ve write komutları için gerekli ek bilgiyi alın ve gönderin
        if (strcmp(command, "create") == 0)
        {
            char file_name[100];
            printf("Enter file name: ");
            scanf("%s", file_name);

            if (write(pipe_fd, file_name, sizeof(file_name)) < 0)
            {
                perror("write");
                exit(1);
            }
        }
        else if (strcmp(command, "delete") == 0 || strcmp(command, "read") == 0 || strcmp(command, "write") == 0)
        {
            int index;
            printf("Enter index: ");
            scanf("%d", &index);

            if (write(pipe_fd, &index, sizeof(index)) < 0)
            {
                perror("write");
                exit(1);
            }

            if (strcmp(command, "write") == 0)
            {
                char data[100];
                printf("Enter data: ");
                scanf("%s", data);

                if (write(pipe_fd, data, sizeof(data)) < 0)
                {
                    perror("write");
                    exit(1);
                }
            }
        }

        // işlem sonucunu alın ve yazdırın
        if (read(pipe_fd, response, sizeof(response)) < 0)
        {
            perror("read");
            exit(1);
        }
        printf("Result: %s\n", response);
    }

    close(pipe_fd);
    return 0;
}