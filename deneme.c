#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    char buffer[1024];
    strcpy(buffer, "erzurum");
    printf("%s", buffer);


    //printf(" create <file_name> \n delete <file_name> \n read <file_name> \n write <file_name> <data> \n exit \n Enter command: ");
        //fgets(buf, BUFSIZE, stdin);
        //buf[strcspn(buf, "\n")] = '\0';

    return 0;
}
