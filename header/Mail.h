#ifndef TWMAILER_MAIL
#define TWMAILER_MAIL

#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

class Mail {
    private:
        int id;
        char *sender, *recipient, *subject, *content;
        
        int saveMail(char* dirString);
    public:
        Mail(char* _sender, char* _recipient, char* _subject, char* _content);

        void displayValues();
        int saveToMaildir(char* maildir);
};

#endif