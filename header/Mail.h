#ifndef TWMAILER_MAIL
#define TWMAILER_MAIL

#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <uuid/uuid.h>
#include <string>
#include <iostream>

#define UUID_SIZE 37 //laenge einer UUID (36-byte string + tailing \0)

class Mail {
    private:
        char* id;
        char *sender, *recipient, *subject, *content;
        
        int saveMail(char* dirString);

        //UUID
        void generate_UUID(char* id);
    public:
        Mail(char* _sender, char* _recipient, char* _subject, char* _content);

        void displayValues();
        int saveToMaildir(char* maildir);
};

#endif