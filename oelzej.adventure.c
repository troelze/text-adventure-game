/**********************************************************************
 * Name: Tres Oelze
 * Date: 2/13/2019
 * Course Name: 344- 400
 * Description: Provides an interface for playing the game using the most
 * recently generated rooms.
 * *********************************************************************/

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h> 
#include <pthread.h>
#include <time.h>

//struct room where information from each file will be stored
struct room
{
    int id;
    char type[13];  //start or mid or end
    char name[12];  //name of room
    int numConnections;
    char connections[6][12];    //names of room connections
    int connex[6];      //integer room connections
};

pthread_t second_thread;    //for multithreading, define second thread, mutex
pthread_mutex_t myMutex;

/****************************************************************************
 * prints all the connections stored in a struct that is passed as a pointer
 * returns nothing. struct must have name, type, number of connections and an
 * array of those connections filled. Used mainly for debugging.
 * *************************************************************************/
void printConnections(struct room* input)
{
    printf("%s of type %s has %d connections:\n", input->name, input->type, input->numConnections);
    int i;
    for (i = 0; i < input->numConnections; i++)
    {
        printf("  (%s)\n", input->connections[i]);
    }
    
    return;
}

/****************************************************************************
 * prints all the information when navigating to a new room including 
 *  whereto prompt and which rooms the user can jump to. takes room struct
 *  pointer as input and reutrns nothing.
 * *************************************************************************/
void printScreen(struct room* input)
{
    printf("\nCURRENT LOCATION: %s\n", input->name);
    printf("POSSIBLE CONNECTIONS: ");
    int q;
    for (q = 0; q < input->numConnections; q++)
    {
        if (q == (input->numConnections - 1))
        {
            printf("%s.\n", input->connections[q]);
        }
        else
        {
            printf("%s, ", input->connections[q]);
        }
            
    }
    printf("WHERE TO? >");
}

/****************************************************************************
 * checks to see if the room name that was entered is in the list of 
 *  connections for current room. returns true if so. takes string of room 
 *  name input and room struct pointer as parameters.
 * *************************************************************************/
bool isValidConnection(char * string, struct room* input)
{
    int k;
    //printf("You entered: %s.\n", string);
    
    for (k = 0; k < input->numConnections; k++)
    {
        //printf("Comparison to : %s.\n", input->connections[k]);
        if (strcmp(string, input->connections[k]) == 0)
        {
            return true;
        }
    }

    return false;

}

/****************************************************************
* takes name of room, pointers to other rooms and returns
* a struct pointer of a room.
****************************************************************/
struct room* getNextRoom(char * roomName, struct room** allRooms)
{
    int l;
    for (l = 0; l < 7; l++)
    {
        if (strcmp(roomName, allRooms[l]->name) == 0)
        {
            return allRooms[l];
        }
    }

}


/******************************************************************
* Prints the time of day (to be used as second thread) to a file. 
******************************************************************/
void* timePrint(void *arg)
{
    //block this thread by locking already locked mutex
    pthread_mutex_lock(&myMutex);

    //get time info
    time_t seconds; 
    struct tm *info;
    char buffer[80];

    //get time
    time(&seconds);
    info = localtime(&seconds);

    //print formatted to file
    strftime(buffer,80,"%l:%M%P, %A, %B %e, %Y", info);
    //printf("Formatted date & time: %s\n", buffer );

    //open and write data to file
    FILE *time_file;
    time_file = fopen("currentTime.txt", "w");
    fprintf(time_file, "%s", buffer);
    fclose(time_file);
    //unlock mutex so main thread can lock when this thread returns
    pthread_mutex_unlock(&myMutex);
    return (0);
}

        

int main()
{

    //printf("In main: initializing mutex\n");
    if (pthread_mutex_init(&myMutex, NULL) != 0) 
    { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 
    //printf("In main: locking mutex\n");
    pthread_mutex_lock(&myMutex);
    

    int result_code;

    //printf("In main: creating thread\n");
    int error;
    error = pthread_create(&second_thread, NULL, &timePrint, NULL); 
    if (error != 0) 
        printf("\nThread can't be created : [%s]", strerror(error)); 
    


    //Adapted from Reading 2.4 Manipulating Directories
    
    int newestDirTime = -1;
    char targetDirPrefix[32] = "oelzej.rooms.";
    char newestDirName[256];
    memset(newestDirName, '\0', sizeof(newestDirName));

    DIR* dirToCheck;            //holds directory to start in
    struct dirent *fileInDir;   //hols current subdir
    struct stat dirAttributes;

    dirToCheck = opendir(".");  //open directory that program runs in

    if (dirToCheck > 0)
    {
        while ((fileInDir = readdir(dirToCheck)) != NULL)       //check each subdir
        {
            if (strstr(fileInDir->d_name, targetDirPrefix) != NULL) //if entry has prefix
            {
                //printf("Found the prefix: %s\n", fileInDir->d_name);    
                stat(fileInDir->d_name, &dirAttributes);                // get stats on each subdirectory to compare times

                if ((int)dirAttributes.st_mtime > newestDirTime)        //check for newest time stamp
                {
                    newestDirTime = (int)dirAttributes.st_mtime;        //set newest time stamp, as well as name
                    memset(newestDirName, '\0', sizeof(newestDirName));
                    strcpy(newestDirName, fileInDir->d_name);
                    //printf("Newer subdir: %s, new time: %d\n",                 
                    //fileInDir->d_name, newestDirTime);
                }
            }
        }
    }

    //close directory program is ran in
    closedir(dirToCheck);

    //printf("Newest entry found is : %s\n", newestDirName);

     // create array of struct-pointers (array of rooms!)
    struct room *allRooms[7];
    // define each of the seven rooms as structs and assign them all to indices in array of rooms (stored as ptrs)
    struct room room1, room2, room3, room4, room5, room6, room7;
    allRooms[0] = &room1;
    allRooms[1] = &room2;
    allRooms[2] = &room3;
    allRooms[3] = &room4;
    allRooms[4] = &room5;
    allRooms[5] = &room6;
    allRooms[6] = &room7;


    // open newest directory and read from all files in it
    DIR* newestDir;
    struct dirent *fileIn;
    
    newestDir = opendir(newestDirName);
    
    //store name of directory to be used later to open file for reading
    char fullName[sizeof(newestDirName) + 9];
    memset(fullName, '\0', sizeof(fullName));   //store temp name of file so that file can be opened
    strcpy(fullName, newestDirName);
    
    //if successful in opening
    if (newestDir > 0)
    {
        int c = 0;  //skipper (See below)
        int d = 0;  //allRooms index counter
        while ((fileIn = readdir(newestDir)) != NULL)
        {
            if (c > 1)//skip the first two subdir ('.' & '..')
            {

                strcat(fullName, "/");
                strcat(fullName, fileIn->d_name);
                //printf("Opening file: %s\n", fullName); //open file using proper directory prefix
                FILE *f = fopen(fullName, "r");         //open for read-only
                if (f == NULL)
                {
                    printf("Error opening file!\n");        //dispaly error message if open fails
                    exit(1);
                }

                //these are used to store each word read in from each line of each file
                char one[11];
                char two[6];
                char three[11];
                

                int counter = 0;        //this counter is used simply to identify the first line read in 
                                        //(since we know it will contain the room name)

                allRooms[d]->numConnections = 0;    //initialize # of connections to 0
                
                while (fscanf(f, "%s %s %s", one, two, three) != EOF)   //read in each line and split up into separate words until end of file
                {
                        if (counter == 0)
                        {
                            strcpy(allRooms[d]->name, three);       //copy name from file
                        }
                        if (strcmp(one, "CONNECTION") == 0)
                        {
                            strcpy(allRooms[d]->connections[allRooms[d]->numConnections], three);   //copy each connection from file into array of room names
                            allRooms[d]->numConnections++;                  //increment the number of connections each room has
                        }
                        if (strcmp(two, "TYPE:") == 0)
                        {
                            strcpy(allRooms[d]->type, three);           //copy type of room from file (whether it is start, end, mid)

                        }
                    counter++;
                }



                //close the file
                fclose(f);
                
                memset(fullName, '\0', sizeof(fullName));   //reset temp name of file
                strcpy(fullName, newestDirName);   
                //printf("First line of file: %s\n", fileIn->d_name);
                d++;
            }
            c++;
            
        }
    }

    //close the directory we used to access files
    closedir(newestDir);


    //Now we can start the game. We have all the information stored in an array of 'room' struct-pointers
    int p;
    int idx;
    //find which room is the starting room
    for (p = 0; p < 7; p++)
    {
        //printConnections(allRooms[p]);

        if (strcmp(allRooms[p]->type, "START_ROOM") == 0)
        {
            idx = p;
        }
    }

    //printf("start index: %d\n", idx);

    //steps will hold the names of each room stepped into (max of 50 steps) the program will end at 50 steps
    //this could have been implemented as a dynamic array so in theory could have been unlimited steps
    //but since no direction was given I chose to make it max 50.
    int stepCount = 0;
    char steps[50][12];

    struct room* current;   //points to the current room user is in

    current = allRooms[idx];        //points to starting room now


    //until reaching the end, run the game simulation!
    while (strcmp(current->type, "END_ROOM") != 0)
    {
        //printf("type: %s.\n", current->type);
        printScreen(current);


        int numCharsEntered = -5;
        size_t bufferSize = 0;
        char* lineEntered = NULL;

        numCharsEntered = getline(&lineEntered, &bufferSize, stdin);    //get next room from user

        lineEntered[numCharsEntered -1] = '\0';
        //printf("you entered: %s.", lineEntered);

        

        while(!isValidConnection(lineEntered, current)) //as long as input is invalid, keep asking
        {
            //unless input is time then change threads
            if (strcmp(lineEntered, "time") == 0)
            {
                //unlock the mutex, causing second thread to unblock
                pthread_mutex_unlock(&myMutex);
                pthread_join(second_thread, NULL);      //block until secondthread completes
                
                
                //re-lock mutex 
                pthread_mutex_lock(&myMutex);
                //re-create the second thread
                error = pthread_create(&second_thread, NULL, &timePrint, NULL); 
                if (error != 0) 
                    printf("\nThread can't be created : [%s]", strerror(error)); 

                
                FILE *timeFileIn = fopen("currentTime.txt", "r");         //open for read-only
                if (timeFileIn == NULL)
                {
                    printf("Error opening file!\n");        //dispaly error message if open fails
                    exit(1);
                }
                printf("\n");
                int c;
                while((c = getc(timeFileIn)) != EOF)        //print contents of file
                    putchar(c);
                fclose(timeFileIn);
                

                printf("\n\nWHERE TO? >");      //re-prompt for next room
                free(lineEntered);
                lineEntered = NULL;
                numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
                lineEntered[numCharsEntered -1] = '\0';
            }
            else
            { 
                printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
                //printf("type: %s.\n", current->type);
                printScreen(current);

                //re-prompt for next room
                free(lineEntered);
                lineEntered = NULL;
                numCharsEntered = getline(&lineEntered, &bufferSize, stdin);
                lineEntered[numCharsEntered -1] = '\0';
            }
        }

        
       

        current = getNextRoom(lineEntered, allRooms);   //point to next room
        strcpy(steps[stepCount], current->name);
        stepCount++;        //increment number of steps taken

        if (stepCount == 50)        //end program if user has taken 50 steps and still can't find room.
        {
            exit(0);
        }
        
    }

    //print end of game message as well as path to victory thru steps array
    printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
    printf("YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", stepCount);
    int b;
    for (b = 0; b < stepCount; b++)
    {
        printf("%s\n", steps[b]);
    }

        

        //current = allRooms[]

    //}

    return 0;

}