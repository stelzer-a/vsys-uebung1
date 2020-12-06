#include "../header/Mail.h"

// Konstruktor
//
// 
Mail::Mail(char* _sender, char* _recipient, char* _subject, char* _content) {
    sender = _sender;
    recipient = _recipient;
    subject = _subject;
    content = _content;
}

// void displayValues()
// Gibt den Inhalt der Mail aus
// 
void Mail::displayValues() {
    printf("Sender: %s\nRecipient: %s\nSubject: %s\nContent: %s\n", sender, recipient, subject, content);
}

// int saveMailTo(Mailspooldirectory)
// Speichert die Mail im Mailspooldirectory des Empfängers
// Gibt -1 zurück, wenn etwas schiefgelaufen ist 
//
int Mail::saveToMaildir(char* maildir) {
    // An das Mailspooldirectory wird ein '/' und der Empfänger angehängt
    // deswegen auch + 2, wegen dem '/' und dem '\0'
    char* recipientDirString = (char*) malloc((strlen(maildir) + strlen(recipient) + 2) * sizeof(char));
    sprintf(recipientDirString, "%s/%s", maildir, recipient);
    recipientDirString[strlen(recipientDirString)] = '\0';

    // Empfängerverzeichnis erstellen oder öffnen, je nachdem ob es existiert oder nicht
    DIR* recipientDir = opendir(recipientDirString);
    if (recipientDir) {
		// Das Verzeichnis existiert
        saveMail(recipientDirString);

	} else if (errno == ENOENT) {
		// Verzeichnis existiert nicht, also erstellen
		if (mkdir(recipientDirString, 0777) == 0) {
            printf(" %s's directory was created\n", recipient);
            saveMail(recipientDirString);
		} else {
			printf("Error creating recipient's directory!\n");
            free(recipientDirString);
			return -1;
		}
	} else {
		// Anderer Fehler ist aufgetreten
		printf("Error accessing recipient's directory!\n");
        free(recipientDirString);
		return -1;
	}

    closedir(recipientDir);
    free(recipientDirString);
    return 0;
}

// int saveMail(Mailspooldirectory des Empfängers)
// Erzeugt ein File im angegebenen Verzeichnis für die Mail und speichert den Inhalt darin
// Gibt -1 zurück, wenn ein Fehler aufgetreten ist
// 
int Mail::saveMail(char* dirString) {
    // ID und Pfadstring für das File erzeugen
    id = (char*) malloc(UUID_SIZE * sizeof(char));
    generate_UUID(id);
    char* filename = (char*) malloc((strlen(dirString) + strlen(id) + 1) * sizeof(char));
    sprintf(filename, "%s/%s", dirString, id);
    filename[strlen(filename)] = '\0';

    // File erstellen
    FILE* newFile = fopen(filename, "w");
    if (newFile) {
        // Inhalt der Mail ins File schreiben
        fprintf(newFile, "Sender   : %s\n", sender);
        fprintf(newFile, "Recipient: %s\n", recipient);
        fprintf(newFile, "Subject  : %s\n\n", subject);
        fprintf(newFile, "%s", content);
    } else {
        printf("Mail could not be saved!\n");
        free(id);
        free(filename);
        return -1;
    }

    fclose(newFile);
    free(id);   
    free(filename);
    return 0;
}

//erzeut eine UUID und gibt diese als String retour.
void Mail::generate_UUID(char* id)
{
    uuid_t binuuid; //binaryUUID
    //char uuid[37]; //laenge einer UUID (36-byte string + tailing \0)

    uuid_generate_random(binuuid); //generate binary UUID
    uuid_unparse_lower(binuuid, id); //converts binary UUID to string
    printf("%s\n", id);

    //return uuidStr.c_str();
    //return uuid;
}