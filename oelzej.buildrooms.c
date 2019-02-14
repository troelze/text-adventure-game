/**********************************************************************
 * Name: Tres Oelze
 * Date: 2/13/2019
 * Course Name: CS344-400
 * Description: Creates a series of files that hold descriptions of
 * in-game rooms and how the rooms are connected.
 * *********************************************************************/
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h> 
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

//defines interface to which rooms are defined
struct room
{
    int id;
    char name[12];      //string name
    int numOutboundConnections;
    struct room* outboundConnections[6];    //points to other structs that rooms get connected to

};


/***************************************************************************
 * prints out all the connections made (used for debugging). takes struct
 *  room ptr as input. returns nothing.
 * ************************************************************************/
void PrintRoomOutboundConnections(struct room* input)
{
    printf("the rooms connected to (%s/%d) are :\n", input->name, input->id);

    int i;
    for (i = 0; i < input->numOutboundConnections; i++)
    {
        printf("  (%s/%d)\n", input->outboundConnections[i]->name,
        input->outboundConnections[i]->id);
    }
    
    return;
}

/***************************************************************************
 * takes array of room-structs as input. checks to see if any have less than three.
 * Returns true if all rooms have 3 to 6 outbound connections, false otherwise
 * ************************************************************************/
bool IsGraphFull(struct room *input[])  
{
    int i;
    // for each room, if any has less than three connections, return false because the graph is not full
    for (i = 0; i < 7; i++)
    {
        if (input[i]->numOutboundConnections < 3)
        {
            return false;
        }
    }  
    return true;
}

/***************************************************************************
 * Returns a random Room, does NOT validate if connection can be added
 * ************************************************************************/
struct room* GetRandomRoom(struct room *input[])
{
    int num = rand() % 7;
    return input[num];
  
}

/***************************************************************************
 *  Returns true if a connection can be added from Room x (< 6 outbound connections), false otherwise
 * ************************************************************************/
bool CanAddConnectionFrom(struct room * input) 
{
    if (input->numOutboundConnections < 6)
    {
        return true;
    }
    return false;
}

/***************************************************************************
 * Returns true if a connection from Room x to Room y already exists, false otherwise
 * ************************************************************************/
bool ConnectionAlreadyExists(struct room *x, struct room *y)
{
    int i;
    for (i = 0; i < x->numOutboundConnections; i++)
    {
        if (x->outboundConnections[i]->id == y->id)
        {
            return true;
        }
    }
    return false;
}

/***************************************************************************
 * Connects Rooms x and y together, does not check if this connection is valid
 * ************************************************************************/
void ConnectRoom(struct room *x, struct room *y)
{
    int idx_x = x->numOutboundConnections;
    int idx_y = y->numOutboundConnections;

    x->outboundConnections[idx_x] = y;
    y->outboundConnections[idx_y] = x;

    x->numOutboundConnections++;
    y->numOutboundConnections++;
}

/***************************************************************************
 * Returns true if Rooms x and y are the same Room, false otherwise
 * ************************************************************************/
bool IsSameRoom(struct room *x, struct room *y)
{
   if (x->id == y->id)
    {
        return true;
    }
    return false;
}


int main()
{
    //generate directory for storing room files using process id
    int p_id = getpid();
    char prefix[] = "oelzej.rooms.";        
    int len = sizeof(prefix) + 7;
    char dirName[len];                      //define directory name and copy prefix, process-id into it
    memset(dirName, '\0', len);
    sprintf(dirName, "%s%d", prefix, p_id);
    mkdir(dirName, 0755);           //make the directory with full permissions

    int len2 = sizeof(dirName)+12;
    char temp[len2];
    memset(temp, '\0', len2);
    strcpy(temp, dirName);
    

    //define room names to be max 8 characters 
    char roomNames[10][9];                  //name each room 
    strcpy(roomNames[0], "kitchen");
    strcpy(roomNames[1], "ballroom");
    strcpy(roomNames[2], "bedroom");
    strcpy(roomNames[3], "dining");
    strcpy(roomNames[4], "billiard");
    strcpy(roomNames[5], "library");
    strcpy(roomNames[6], "lounge");
    strcpy(roomNames[7], "hall");
    strcpy(roomNames[8], "basement");
    strcpy(roomNames[9], "attic");

    int indexArray[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    srand(time(0));
    //scramble indexes to make first 7 random number 0-9
    int j;
    for (j = 0; j < 10; j++)
    {
        int tmp = indexArray[j];
        int randIndex = rand() % 10;
        indexArray[j] = indexArray[randIndex];
        indexArray[randIndex] = tmp;
    }

    // create array of struct-pointers (array of rooms!)
    struct room *allRooms[7];
    // define each of the seven rooms as structs and assign them all to indices in array of rooms
    struct room startRoom, midRoom1, midRoom2, midRoom3, midRoom4, midRoom5, endRoom;
    allRooms[0] = &startRoom;
    allRooms[1] = &midRoom1;
    allRooms[2] = &midRoom2;
    allRooms[3] = &midRoom3;
    allRooms[4] = &midRoom4;
    allRooms[5] = &midRoom5;
    allRooms[6] = &endRoom;

    // change room data to reflect id #, name
    int q;
    for (q = 0; q < 7; q++)
    {
        allRooms[q]->id = q;
        int num = indexArray[q];
        strcpy(allRooms[q]->name, roomNames[num]);
        allRooms[q]->numOutboundConnections = 0;
        //printf("index: %d, name: %s\n", allRooms[q]->id, allRooms[q]->name);
        //printRoomOutboundConnections(allRooms[q]);
    }
    
    // Create all connections in graph
    while (IsGraphFull(allRooms) == false)          //while graph is not full, keep building connections of rand rooms
    {
        struct room* A;  // Pointer to structs
        struct room* B;

        while(true)
        {
            A = GetRandomRoom(allRooms);            

            if (CanAddConnectionFrom(A) == true)
                break;
        }

        do
        {
            B = GetRandomRoom(allRooms);
        }
        while(CanAddConnectionFrom(B) == false || IsSameRoom(A, B) == true || ConnectionAlreadyExists(A, B) == true);

        ConnectRoom(A, B);  // Add this connection to the pointers
                    
    }

    /*
    int p;
    for (p = 0; p < 7; p++)
    {
        PrintRoomOutboundConnections(allRooms[p]);
    }
    */


    


    int i;
    // write information about rooms to files
    for ( i = 0; i < 7; i++)
    {
        strcat(temp, "/");
        strcat(temp, allRooms[i]->name);       //concat dirname + roomname
        
        FILE *f = fopen(temp, "w");     //open file for each room
        if (f == NULL)
        {
            printf("Error opening file!\n");
            exit(1);
        }

        fprintf(f, "ROOM NAME: %s\n", allRooms[i]->name);       //write name, connections, room type
        int n;
        for (n = 0; n < allRooms[i]->numOutboundConnections; n++)
        {
            int m = n+1;
            fprintf(f, "CONNECTION %d: %s\n", m, allRooms[i]->outboundConnections[n]->name);
        }

        if (i == 0)
        {
            fprintf(f, "ROOM TYPE: START_ROOM\n");
        }
        if (i == 6)
        {
            fprintf(f, "ROOM TYPE: END_ROOM\n");
        }
        if (i > 0 && i < 6)
        {
            fprintf(f, "ROOM TYPE: MID_ROOM\n");
        }

        fclose(f);

        memset(temp, '\0', sizeof(temp));   //reset temp name of file
        strcpy(temp, dirName);              
    }


   return 0;

}