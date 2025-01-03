#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <gtk/gtk.h>

#define CONFIG_SIZE 1024
#define SECTION_SIZE 256

void Error(char message[]){
    printf("\033[31mERROR: %s\033[37m\n", message);
}

void Warning(char message[]){
    printf("\033[33mWARNING: %s\033[37m\n", message);
}

char* JoinStrings(const char* part1, const char* part2, int slash){
    size_t len1 = strlen(part1);
    size_t len2 = strlen(part2);

    char* result = malloc(len1 + len2 +2);

    strcpy(result, part1);

    if (part1[len1-1] != '/' && slash){
        strcat(result, "/");
    }

    strcat(result, part2);
    return result;
}

void CreateNewSection(char* name, char* NoteDirectory){
    if(!((mkdir(("%s/%s", NoteDirectory, name), 0777))==0)){
        Error("Fail to create a Section");
    }
}

void ReadConfig(){
    FILE* pConfig;

    pConfig = fopen("config.ini", "r");

    if (pConfig == NULL){
        Error("Could not read config file");
    }

    char ch;
    char* ConfigContent;
    int buffer = CONFIG_SIZE;
    ConfigContent = malloc(sizeof(char) * buffer);

    int p = 1;

    while ((ch = fgetc(pConfig)) != EOF){
        ConfigContent[p-1] = putchar(ch);
        if (p == buffer){
            buffer += CONFIG_SIZE;
            ConfigContent = realloc(ConfigContent, buffer);
        }
        p++;
    }

    printf("%s\n", ConfigContent);

    // Set config values

    free(ConfigContent);
}

void ChackForNoteDirectory(char* NoteDirector){
    struct stat statbuf;

    if(stat(NoteDirector, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)){ return; }

    if(!((mkdir(("%s", NoteDirector), 0777))==0)){
        Error("Fail to create a Note Directory");
    }
}

void AllSections(char* NoteDirectory, char** AllSectionsArr){
    DIR* dir =  opendir(NoteDirectory);
    struct dirent* d;
    
    if(dir == NULL){
        Error("Unable to load Note directory trying to create new one");
        ChackForNoteDirectory(NoteDirectory);

        if(dir == NULL){ return; }
    }

    int i = 0;
    while ((d = readdir(dir)) != NULL){
        if (strcmp(d->d_name, ".") == 0) { continue; }
        if (strcmp(d->d_name, "..") == 0) { continue; }


        AllSectionsArr[i] = strdup(d->d_name);
        i += 1;
    }

    closedir(dir);
}

char** GetSectionContent(char* NoteDirectory,int* SelectionID, char** AllSectionsArr){
    DIR* dir = opendir(JoinStrings(NoteDirectory, AllSectionsArr[*SelectionID], 1));

    struct dirent* d;

    if(dir == NULL){
        Error("Unable to load Section");
        return NULL;
    }

    char** ReturnArray = malloc(256 * sizeof(char*));

    int i = 0;
    while ((d = readdir(dir)) != NULL){
        if (i > 255){
            Warning("There is too many files in Selected Section");
            break;
        }

        if(i < 2){ i += 1; continue; }

        ReturnArray[i-2] = strdup(d->d_name);

        i += 1;
    }
    
    closedir(dir);

    ReturnArray[i] = NULL;
    return ReturnArray;
}

void SetSection(int* SectionID, int InputID){
    if (InputID < 0 || InputID > 255){
        Error("Input id is out of range, SectionID");
        return;
    }

    *SectionID = InputID;
}

void SetProject(int* ProjectID, int InputID){
    if(InputID < 0 || InputID > 255){
        Error("Input id is out of range, ProjectID");
        return;
    }

    *ProjectID = InputID;
}

void CreateProjectFiles(char* LunchCommand, char* NoteDirectory, char** AllSectionsArr, char** ProjectContent, int* SelectionID, int* ProjectID){
    system(JoinStrings(LunchCommand, JoinStrings(NoteDirectory, JoinStrings(AllSectionsArr[*SelectionID], ProjectContent[*ProjectID], 1), 1), 0));
}

void RunProjects(char* LunchCommand, char* NoteDirectory, char** AllSectionsArr, char** ProjectContent, int* SelectionID, int* ProjectID){
    system(JoinStrings(LunchCommand, JoinStrings(NoteDirectory, JoinStrings(AllSectionsArr[*SelectionID], ProjectContent[*ProjectID], 1), 1), 0));
}

void ApplyCssStyle(GtkWidget* widget, char* StyleType){
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError* error = NULL;

    gtk_css_provider_load_from_path(css_provider, StyleType);

    gtk_style_context_add_provider(gtk_widget_get_style_context(widget), GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css_provider);
}

void ActivateApp(GtkApplication* app, gpointer user_data){
    GtkWidget* window;
    GtkWidget* LeftMenu;
    GtkWidget* RightPart;
    
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "NoteAPP");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv){
    // Set up
    char NoteDirectory[] = "/run/media/martin/Elements/School";
    int SelectionID = -1;
    int ProjectID = -1;
    char** AllSectionsArr = malloc(256 * sizeof(char*));
    char** ProjectContent = malloc(256 * sizeof(char*));

    GtkApplication* app = gtk_application_new("com.example.NoteApp", G_APPLICATION_FLAGS_NONE);

    AllSections(NoteDirectory, AllSectionsArr);

    // Main loop
    g_signal_connect(app, "activate", G_CALLBACK(ActivateApp), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    // Cleaning and shut down
    g_object_unref(app);
    return 0;
}