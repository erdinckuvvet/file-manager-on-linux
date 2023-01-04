#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define PIPE_NAME "file_manager"
#define MAX_FILES 10

void create_pipe();
void *communicate_with_client(void *arg);
void handle_create_command(char *file_name, char *pipeName);
void handle_delete_command(int index, char *pipeName);
void handle_read_command(int index, char *pipeName);
void handle_write_command(int index, char *data, char *pipeName);
void listen_commands();
int get_empty_index();

pthread_t *threadList[5];
char *file_list[10]; // dosya isimlerini tutacak dizi
pthread_t thread_id; // thread id

// named pipe'ı oluşturan fonksiyon
void create_pipe()
{
    // named pipe oluşturma işlemleri
    unlink(PIPE_NAME);               // eğer pipe zaten oluşturulmuşsa silinir
    if (mkfifo(PIPE_NAME, 0666) < 0) // pipe oluşturulur
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

// file_client ile iletişim kuracak thread fonksiyonu
void *communicate_with_client(void *arg)
{
    // file_client ile iletişim kurma işlemleri
    char pipeName[1];
    char *temp = (char *)arg;
    strcpy(pipeName, temp);

    
    char file_name[100]; // dosya ismini tutacak dizi
    int index;           // dosya indexini tutacak değişken
    char data[1000];     // dosyadaki veriyi tutacak dizi

    // named pipe oluşturma işlemleri
    unlink(pipeName);               // eğer pipe zaten oluşturulmuşsa silinir
    if (mkfifo(pipeName, 0666) < 0) // pipe oluşturulur
    {
        perror("mkfifo");
        exit(1);
    }

    write_pipe(PIPE_NAME, pipeName);

    while (1)
    {
        char command[256];    // komutu tutacak dizi

        // file_client'tan komut okunur
        if (read(pipeName, command, sizeof(command)) < 0)
        {
            perror("read");
            exit(1);
        }

        

        if (strcmp(command, "create") == 0) // create komutu
        {
            // file_client'tan dosya ismi okunur
            if (read(pipeName, file_name, sizeof(file_name)) < 0)
            {
                perror("read");
                exit(1);
            }

            handle_create_command(file_name, pipeName); // create işlemi yapılır
        }
        else if (strcmp(command, "delete") == 0) // delete komutu
        {
            // file_client'tan dosya indexi okunur
            if (read(pipeName, &index, sizeof(index)) < 0)
            {
                perror("read");
                exit(1);
            }

            handle_delete_command(index, pipeName); // delete işlemi yapılır
        }
        else if (strcmp(command, "read") == 0) // read komutu
        {
            // file_client'tan dosya indexi okunur
            if (read(pipeName, &index, sizeof(index)) < 0)
            {
                perror("read");
                exit(1);
            }

            handle_read_command(index, pipeName); // read işlemi yapılır
        }
        else if (strcmp(command, "write") == 0) // write komutu
        {
            // file_client'tan dosya indexi ve veri okunur
            if (read(pipeName, &index, sizeof(index)) < 0 || read(pipeName, data, sizeof(data)) < 0)
            {
                perror("read");
                exit(1);
            }

            handle_write_command(index, data, pipeName); // write işlemi yapılır
        }
        else if (strcmp(command, "exit") == 0) // exit komutu
        {
            break; // iletişim koparılır ve döngüden çıkılır
        }
        else // bilinmeyen komut
        {
            printf("Bilinmeyen komut: %s\n", command);
        }
    }

    return NULL;
}

// create komutunu işleyen fonksiyon
void handle_create_command(char *file_name, char *pipeName)
{
    // dosya ismi boş değilse
    if (strlen(file_name) > 0)
    {
        // file_list dizisi içinde dosya ismi aranır
        int index = -1;
        for (int i = 0; i < MAX_FILES; i++)
        {
            if (strcmp(file_list[i], file_name) == 0)
            {
                index = i;
                break;
            }
        }
        if (index == -1) // dosya ismi bulunamadıysa
        {
            // boş index bulunur
            index = get_empty_index();

            if (index != -1) // boş index bulunmuşsa
            {
                // file_list'e dosya ismi eklenir ve dosya oluşturulur
                strcpy(file_list[index], file_name);
                FILE *fp = fopen(file_name, "w");
                if (fp == NULL)
                {
                    perror("fopen");
                    exit(1);
                }
                fclose(fp);

                // file_client'a başarılı mesajı gönderilir
                char response[20] = "Basarili";
                if (write(pipeName, response, sizeof(response)) < 0)
                {
                    perror("write");
                    exit(1);
                }
            }
            else // boş index bulunamadıysa
            {
                // file_client'a dolu mesajı gönderilir
                char response[20] = "Dolu";
                if (write(pipeName, response, sizeof(response)) < 0)
                {
                    perror("write");
                    exit(1);
                }
            }
        }
        else // dosya ismi bulunmuşsa
        {
            // file_client'a zaten var mesajı gönderilir
            char response[20] = "Zaten var";
            if (write(pipeName, response, sizeof(response)) < 0)
            {
                perror("write");
                exit(1);
            }
        }
    }
    else // dosya ismi boşsa
    {
        // file_client'a boş mesajı gönderilir
        char response[20] = "Boş";
        if (write(pipeName, response, sizeof(response)) < 0)
        {
            perror("write");
            exit(1);
        }
    }
}

// delete komutunu işleyen fonksiyon
void handle_delete_command(int index, char *pipeName)
{
    // index geçerli değilse
    if (index < 0 || index >= MAX_FILES || strlen(file_list[index]) == 0)
    {
        // file_client'a geçersiz mesajı gönderilir
        char response[20] = "Geçersiz";
        if (write(pipeName, response, sizeof(response)) < 0)
        {
            perror("write");
            exit(1);
        }
    }
    else // index geçerli ise
    {
        // dosya silinir ve file_list'ten silinir
        if (remove(file_list[index]) == 0)
        {
            memset(file_list[index], 0, sizeof(file_list[index]));

            // file_client'a başarılı mesajı gönderilir
            char response[20] = "Başarılı";
            if (write(pipeName, response, sizeof(response)) < 0)
            {
                perror("write");
                exit(1);
            }
        }
        else // dosya silinemediys
        {
            // file_client'a hata mesajı gönderilir
            char response[20] = "Hata";
            if (write(pipeName, response, sizeof(response)) < 0)
            {
                perror("write");
                exit(1);
            }
        }
    }
}

// read komutunu işleyen fonksiyon
void handle_read_command(int index, char *pipeName)
{
    // index geçerli bir index mi?
    if (index >= 0 && index < MAX_FILES)
    {
        // dosya ismi file_list dizisinde var mı?
        if (strlen(file_list[index]) > 0)
        {
            // dosya açılır
            FILE *fp = fopen(file_list[index], "r");
            if (fp == NULL)
            {
                perror("fopen");
                exit(1);
            }

            // dosyadaki veri okunur
            char data[1000];
            fgets(data, sizeof(data), fp);
            fclose(fp);

            // file_client'a veri gönderilir
            if (write(pipeName, data, sizeof(data)) < 0)
            {
                perror("write");
                exit(1);
            }
        }
        else // dosya ismi yoksa
        {
            // file_client'a yok mesajı gönderilir
            char response[20] = "Yok";
            if (write(pipeName, response, sizeof(response)) < 0)
            {
                perror("write");
                exit(1);
            }
        }
    }
    else // index geçersizse
    {
        // file_client'a geçersiz mesajı gönderilir
        char response[20] = "Geçersiz";
        if (write(pipeName, response, sizeof(response)) < 0)
        {
            perror("write");
            exit(1);
        }
    }
}

// write komutunu işleyen fonksiyon
void handle_write_command(int index, char *data, char *pipeName)
{
    // dosya indexi geçerliysse
    if (index >= 0 && index < MAX_FILES)
    {
        // file_list'ten dosya ismi okunur
        char file_name[100];
        strcpy(file_name, file_list[index]);

        // dosya açılır
        FILE *fp = fopen(file_name, "w");
        if (fp == NULL)
        {
            perror("fopen");
            exit(1);
        }

        // dosyaya veri yazılır
        fprintf(fp, "%s", data);

        // dosya kapatılır
        fclose(fp);

        // file_client'a başarılı mesajı gönderilir
        char response[20] = "Basarili";
        if (write(pipeName, response, sizeof(response)) < 0)
        {
            perror("write");
            exit(1);
        }
    }
    else // dosya indexi geçersizse
    {
        // file_client'a geçersiz index mesajı gönderilir
        char response[20] = "Geçersiz index";
        if (write(pipeName, response, sizeof(response)) < 0)
        {
            perror("write");
            exit(1);
        }
    }
}

void write_pipe(char *pipeName, char *msg)
{
    int fd = open(pipeName, O_WRONLY);
    if (fd< 0)
    {
        perror("open");
        return;
    }

    if (write(pipeName, msg, sizeof(msg)) < 0)
    {
        perror("write");
        return;
    }
    close(fd);
}

void listen_commands()
{
    // file_server sürekli dinleme moduna geçirilir
    while (1)
    {
        // file_client'tan komut okunur
        char command[10];
        if (read(PIPE_NAME, command, sizeof(command)) < 0)
        {
            perror("read");
            exit(1);
        }

        // komut "create" ise dosya oluşturma işlemi yapılır
        if (strcmp(command, "create") == 0)
        {
            // file_client'tan dosya ismi okunur
            char file_name[100];
            if (read(PIPE_NAME, file_name, sizeof(file_name)) < 0)
            {
                perror("read");
                exit(1);
            }

            handle_create_command(file_name, PIPE_NAME);
        }
        // komut "delete" ise dosya silme işlemi yapılır
        else if (strcmp(command, "delete") == 0)
        {
            // file_client'tan dosya indexi okunur
            int index;
            if (read(PIPE_NAME, &index, sizeof(index)) < 0)
            {
                perror("read");
                exit(1);
            }

            handle_delete_command(index,PIPE_NAME);
        }
        // komut "read" ise dosya okuma işlemi yapılır
        else if (strcmp(command, "read") == 0)
        {
            // file_client'tan dosya indexi okunur
            int index;
            if (read(PIPE_NAME, &index, sizeof(index)) < 0)
            {
                perror("read");
                exit(1);
            }

            handle_read_command(index, PIPE_NAME);
        }
        // komut "write" ise dosya yazma işlemi yapılır
        else if (strcmp(command, "write") == 0)
        {
            // file_client'tan dosya indexi ve veri okunur
            int index;
            char data[1000];
            if (read(PIPE_NAME, &index, sizeof(index)) < 0 || read(PIPE_NAME, data, sizeof(data)) < 0)
            {
                perror("read");
                exit(1);
            }

            handle_write_command(index, data, PIPE_NAME);
        }
        // komut "exit" ise iletişim kesilir ve döngüden çıkılır
        else if (strcmp(command, "exit") == 0)
        {
            break;
        }
        // bilinmeyen bir komut ise hata mesajı gönderilir
        else
        {
            char response[20] = "Hatalı komut";
            if (write(PIPE_NAME, response, sizeof(response)) < 0)
            {
                perror("write");
                exit(1);
            }
        }
    }
}
int main()
{
    create_pipe(); // pipe oluşturulur

    // file_client ile iletişim kuracak threadler oluşturulur
    for (int i = 0; i < 5; i++)
    {
        pthread_t thread; // thread değişkeni

        char pipeName[1];
        sprintf(pipeName, "%d", i);

        if (pthread_create(&thread, NULL, communicate_with_client, (void *)pipeName) != 0)
        {
            perror("pthread_create");
            exit(1);
        }
        threadList[i] = thread;
    }

    for (int i = 0; i < 5; i++)
    {
        pthread_join(threadList[i], NULL);
    }

    // file_manager komutlarını dinlemeye başlar
    listen_commands();

    return 0;
}
