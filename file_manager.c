#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define PIPE_NAME "/tmp/file_manager"
#define MAX_FILES 10
#define BUFFER_LENGTH 1024
int _read_pipe(char *pipeName, char *buffer);
int _write_pipe(char *pipeName, char *msg);
void create_pipe(char *pipeName);
void *communicate_with_client(void *arg);
void handle_create_command(char *file_name, char *pipeName);
void handle_delete_command(char *file_name, char *pipeName);
void handle_read_command(char *file_name, char *pipeName);
void handle_write_command(char *file_name, char *content, char *pipeName);
void listen_commands();
int get_empty_index();

typedef struct
{
    pthread_t thread;
    char *name;
    int status;
} Pipe;

Pipe pipeList[5];

char *file_list[MAX_FILES]; // dosya isimlerini tutacak dizi

// named pipe'ı oluşturan fonksiyon
void create_pipe(char *pipeName)
{
    // named pipe oluşturma işlemleri
    unlink(pipeName);               // eğer pipe zaten oluşturulmuşsa silinir
    if (mkfifo(pipeName, 0666) < 0) // pipe oluşturulur
    {
        perror("mkfifo");
        exit(1);
    }
}

// bos index bulan fonksiyon
int get_empty_index()
{
    // Find an empty slot in the file list
    for (int i = 0; i < MAX_FILES; i++)
    {
        if (file_list[i] == NULL)
        {
            // Return the index of the empty slot
            return i;
        }
    }

    // If no empty slot is found, return -1
    return -1;
}

int get_file_index(char *fileName)
{

    for (int i = 0; i < MAX_FILES; i++)
    {
        if (file_list[i] && strcmp(file_list[i], fileName) == 0)
        {
            return i;
        }
    }

    return -1;
}

// dosya adını kontrol eden fonksiyon
int isFileExist(char *fileName)
{
    printf("is file exist\n");
    for (int i = 0; i < MAX_FILES; i++)
    {
        int len = strlen(fileName);

        if (file_list[i] != NULL)
        {
            if (strcmp(file_list[i], fileName) == 0)
            {
                return 1;
            }
        }
        printf("end loop\n");
    }

    return 0;
}

// file_client ile iletişim kuracak thread fonksiyonu
void *communicate_with_client(void *index)
{
    // file_client ile iletişim kurma işlemleri
    int *i = (int *)index;
    char *pipeName = pipeList[*i].name;

    printf("pipe name => %s\n", pipeName);
    char file_name[100]; // dosya ismini tutacak dizi
    // int index;           // dosya indexini tutacak değişken
    char content[1024]; // dosyadaki veriyi tutacak dizi
    char command[1024];
    char buffer[BUFFER_LENGTH]; // komutu tutacak dizi

    // named pipe oluşturma işlemleri
    create_pipe(pipeName);

    while (1)
    {
        // file_client'tan komut okunur
        _read_pipe(pipeName, buffer);

        sscanf(buffer, "%s %s %[^\n]", command, file_name, content);
        printf("buffer =>%s\ncommand => %s\nfile_name => %s\ncontent => %s\n", buffer, command, file_name, content);
        if (strcmp(command, "create") == 0) // create komutu
        {
            handle_create_command(file_name, pipeName); // create işlemi yapılır
        }
        else if (strcmp(command, "delete") == 0) // delete komutu
        {
            handle_delete_command(file_name, pipeName); // delete işlemi yapılır
        }
        else if (strcmp(command, "read") == 0) // read komutu
        {
            // file_client'tan dosya indexi okunur
            handle_read_command(file_name, pipeName); // read işlemi yapılır
        }
        else if (strcmp(command, "write") == 0) // write komutu
        {
            // file_client'tan dosya indexi ve veri okunur
            handle_write_command(file_name, content, pipeName); // write işlemi yapılır
        }
        else if (strcmp(command, "exit") == 0) // exit komutu
        {
            printf("SERVER exit received\n");
            pipeList[*i].status = 0;
            break; // iletişim koparılır ve döngüden çıkılır
        }
        else // bilinmeyen komut
        {
            _write_pipe(pipeName, "gecersiz komut");
            printf("Bilinmeyen komut: %s\n", command);
        }
    }
    free(index);
    return NULL;
}

// create komutunu işleyen fonksiyon
void handle_create_command(char *file_name, char *pipeName)
{
    // dosya ismi boş değilse
    if (strlen(file_name) <= 0)
    {
        _write_pipe(pipeName, "basarisiz");
        return;
    }

    if (isFileExist(file_name))
    {
        _write_pipe(pipeName, "dosya mevcut");
        return;
    }

    // file_list dizisi içinde dosya ismi aranır
    int index = get_empty_index();

    if (index == -1) // dosya ismi bulunamadıysa
    {
        _write_pipe(pipeName, "basarisiz");
        return;
    }

    // file_list'e dosya ismi eklenir ve dosya oluşturulur
    file_list[index] = (char *)malloc(1024);
    memcpy(file_list[index], file_name, strlen(file_name) + 1);

    FILE *fp = fopen(file_name, "w");
    if (fp == NULL)
    {
        perror("fopen");
        exit(1);
    }
    fclose(fp);

    // file_client'a başarılı mesajı gönderilir
    _write_pipe(pipeName, "Basarili");
}

// delete komutunu işleyen fonksiyon
void handle_delete_command(char *file_name, char *pipeName)
{
    int index = get_file_index(file_name);
    if (index == -1)
    {
        _write_pipe(pipeName, "dosya mevcut degil");
        return;
    }

    //  Dosya ismini dosya listesi dizisinden sil
    free(file_list[index]);
    file_list[index] = NULL;

    unlink(file_name);

    _write_pipe(pipeName, "dosya silindi");
}

// read komutunu işleyen fonksiyon
void handle_read_command(char *file_name, char *pipeName)
{
    if (!isFileExist(file_name))
    {
        _write_pipe(pipeName, "dosya mevcut degil");
        return;
    }
    // Dosyayı aç
    FILE *fd;
    fd = fopen(file_name, "r");
    if (!fd)
    {
        _write_pipe(pipeName, "dosya acilamadi");
        return;
    }

    char content[2048];
    int index = 0;
    char c;
    while ((c = fgetc(fd)) != EOF)
    {
        content[index++] = c;
    }
    content[index] = '\0';

    _write_pipe(pipeName, content);
}

// write komutunu işleyen fonksiyon
void handle_write_command(char *file_name, char *content, char *pipeName)
{
    // int index = get_file_index(&file_name);
    // printf("index bulundu");
    if (!isFileExist(file_name))
    {
        _write_pipe(pipeName, "dosya mevcut degil");
        return;
    }
    // Dosyayı aç
    FILE *fd;
    fd = fopen(file_name, "a");
    if (!fd)
    {
        _write_pipe(pipeName, "dosya acilamadi");
        return;
    }

    // Veriyi dosyaya yaz
    fprintf(fd, "%s", content);
    fprintf(fd, "%s", "\n");

    // sonucu yazdir
    _write_pipe(pipeName, "dosyaya yazma islemi basarili");

    fclose(fd);
}

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
    return 0;
}

int _read_pipe(char *pipeName, char *buffer)
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

void listen_commands()
{
    // file_server sürekli dinleme moduna geçirilir
    while (1)
    {
        // file_client'tan komut okunur
        char command[BUFFER_LENGTH];
        _read_pipe(PIPE_NAME, command);

        int connectionStatus = 0;
        for (size_t i = 0; i < 5; i++)
        {
            if (!pipeList[i].status)
            {
                _write_pipe(PIPE_NAME, pipeList[i].name);
                int *param = malloc(sizeof(int));
                *param = i;
                pthread_create(&pipeList[i].thread, NULL, communicate_with_client, param);
                pipeList[i].status = 1;
                connectionStatus = 1;
                break;
            }
        }

        if (!connectionStatus)
        {
            printf("not enough pipe\n");
            _write_pipe(PIPE_NAME, "not enough pipe");
        }
    }
}

int main()
{

    for (size_t i = 0; i < 5; i++)
    {
        file_list[i] = NULL;
    }

    create_pipe(PIPE_NAME); // pipe oluşturulur
    for (int i = 0; i < 5; i++)
    {
        pipeList[i].status = 0;
        char *name = malloc(25);
        sprintf(name, "pipe_%d", i);
        printf("created pipe name => %s\n", name);
        pipeList[i].name = name;
    }

    // file_manager komutlarını dinlemeye başlar
    listen_commands();

    return 0;
}
