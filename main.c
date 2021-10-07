#include<stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <wait.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

typedef struct Applicant
{
   char identificationNum[12];
   char firstName[100];
   char lastName[100];
   int birthday;
   char phNum[12];
   bool pay;
   bool vaccinated;
}Applicant;


void deleteData();
void addData();
void createList();
Applicant *readData();
void modifyData();
void printApplicant(Applicant app);
void WriteInFile(Applicant app);
void menu();
void writePipe(Applicant *p, int size);
Applicant *readPipe(int size);
int countApplicants();
int generator(int max);
void  updateList(Applicant *vaccinated, int size);
Applicant *selData(int bus);
Applicant *deleteOneRandomly(Applicant *p, int size, int ind);
void handler(int signum){
    printf("Fight UP Signal\n");
}

int main()
{

    menu();
    createList();
    if ( countApplicants() > 4 )
    {
        unlink("/tmp/e9tz67300");
        int pipemain = mkfifo("/tmp/e9tz67300", S_IRUSR|S_IWUSR ); // creating named pipe file
        signal(SIGTERM, handler);
        pid_t child_A=fork();
        if (child_A>0)
        {
            pause();
            sleep(2);
            writePipe(selData(1), 5);
            printf("Data sent to bus 1\n");
            Applicant *vaccinated = readPipe(4);
            printf("Data received from bus 1\n");
            printf("Updated\n");
            updateList(vaccinated, 4);

            int status;
            waitpid(child_A,&status,0);

            if(countApplicants() == 10)
            {
                pid_t  child_B=fork();
                if (child_B>0)
                {
                    pause();
                    sleep(2);
                    writePipe(selData(2), 5);
                    printf("Data sent to bus 2\n");
                    Applicant *vaccinated = readPipe(4);
                    printf("Data received from bus 2\n");
                    printf("Updated\n");
                    updateList(vaccinated, 4);


                }
                else
                {
                    sleep(2);
                    kill(getppid(),SIGTERM);
                    sleep(2);
                    Applicant *vaccinatedSent = deleteOneRandomly(readPipe(5), 5, generator(5));
                    printf("Data received\n");
                    writePipe(vaccinatedSent, 4);
                    printf("Data sent back and vaccinated\n");

                }
                int status;
                waitpid(child_B, &status,0);
            }
        }
        else
        {
            sleep(2);
            kill(getppid(),SIGTERM);
            sleep(2);
            Applicant *vaccinatedSent = deleteOneRandomly(readPipe(5), 5, generator(5));
            printf("Data received\n");
            writePipe(vaccinatedSent, 4);
            printf("Data sent back and vaccinated\n");
            sleep(2);

        }

    }else{
        printf("Not enough applicants to start vaccination\n");
    }

    return 0;
}



Applicant *deleteOneRandomly(Applicant *p, int size, int ind)
{
    Applicant *apps = malloc(sizeof(Applicant) * (size-1) );
    int j = 0;
    for(int i=0; i<size; i++)
    {
        if(i != ind)
        {
            apps[j] = p[i];
            j++;
        }

    }
    return apps;
}

Applicant *selData(int bus)
{
    FILE *fp=fopen("data.txt","r");
    if(bus==1){
        Applicant applicant;
        int cnt = 0;
        int last = 5;
        Applicant *applicants = malloc(sizeof(Applicant) * 5);
        while(fread(&applicant, sizeof(applicant), 1, fp))
        {
        if(cnt >= 0 &&  cnt < 5){
            applicants[cnt] = applicant;
        }
        cnt++;
        }
    fclose(fp);
    return applicants;
    }
    if(bus == 2){
        Applicant applicant;
        int cnt = 0;
        int last = 10;
        Applicant *applicants = malloc(sizeof(Applicant) * 5);
        while(fread(&applicant, sizeof(applicant), 1, fp))
        {
        if(cnt >= 5 &&  cnt < 10){
            applicants[cnt-5] = applicant;
        }
        cnt++;
        }
        fclose (fp);
        return applicants;
    }

}

void  updateList(Applicant *vaccinated, int size)
{
    Applicant app;
    FILE *fp=fopen("data.txt", "r");
	FILE *fpTemp=fopen("temp.txt", "w+");
	if (!fp | !fpTemp)
	    printf("Unable to open file\n");
	while (fread(&app, sizeof(app), 1, fp))
    {
        for(int i=0; i<size; i++)
        {
            if(strcmp(app.identificationNum, vaccinated[i].identificationNum) == 0)
                app.vaccinated = true;
        }
        fwrite(&app, sizeof(app), 1, fpTemp);
	}
	fclose(fp);
	fclose(fpTemp);
	remove("data.txt");
	rename("temp.txt", "data.txt");
}

void writePipe(Applicant *p, int size)
{
    int pip = open("/tmp/e9tz67300",O_WRONLY);
    write(pip, p, sizeof(Applicant) * size);
    close(pip);
}
Applicant *readPipe(int size)
{
    Applicant *applicants = malloc(sizeof(Applicant) * size );
    int pip =open("/tmp/e9tz67300",O_RDONLY);
    read(pip , applicants ,sizeof(Applicant) * size);
    close(pip);
    return applicants;
}


int countApplicants()
{
    FILE *fp=fopen("data.txt","r");
    Applicant app;
    int lineno = 0;
    while(fread(&app, sizeof(app), 1, fp))
        lineno++;
    return lineno;
}

int generator(int max)
{
    srand(time(NULL));
    return rand() % max;
}
void menu()
{
    int choice;
    bool done = false;

    do
    {
        printf(" Welcome!\n");
        printf("For vaccine registration, please choose one of the below options\n");
        printf("1. Add\n");
        printf("2. Modify\n");
        printf("3. Delete\n");
        printf("4. List\n");
        printf("5. Exit\n");
        printf("0. Start vaccination\n");
        printf("Which option do you want?");

        scanf("%d", &choice);
        switch(choice)
        {
        case 1:
            addData();
            break;
        case 2:
            modifyData();
            break;
        case 3:
            deleteData();
            break;
        case 4:
            createList();
            break;
        case 5:
            printf("Have a nice day\n");
            exit(1);
            break;
        case 0:
            printf("Starting..\n");
            done=true;
            break;

        default :
            printf("Invalid" );

        }
    } while(done != true);

}

void addData()
{
    Applicant *app = readData();
    WriteInFile(*app);
    free(app);
}

Applicant *readData()
{
    Applicant *app = malloc(sizeof *app);
    char pay[4];
    printf("Give me the required data\n");
    printf("Id number: ");
    scanf("%s", app->identificationNum);
    printf("First Name: ");
    scanf("%s", app->firstName);
    printf("Last Name: ");
    scanf("%s", app->lastName);
    printf("Birthday: ");
    scanf("%d",  &app->birthday);
    printf("Phone number: ");
    scanf("%s", app->phNum);
    printf("Are you willing to pay? ");
    scanf("%s", pay);
    app->pay = strcmp(pay, "yes");
    app->vaccinated=false;

    return app;
}

void printApplicant(Applicant app)
{
    printf("%s, %s, %s, %d, %s, ",app.identificationNum, app.firstName, app.lastName, app.birthday, app.phNum);
    if(app.pay==0){
        printf("Yes ");
    }
    else{
        printf("No ");
    }
    if(app.vaccinated==1){
        printf("Vaccinated\n");
    }
    else{
        printf("\n");
    }


}
void WriteInFile(Applicant app)
{
    char  *file = "data.txt";
    FILE *fp;
    fp = fopen (file, "a");
    fwrite(&app, sizeof(app), 1, fp);
    fclose (fp);
}

void createList()
{
    FILE *fp=fopen("data.txt","r");
    Applicant app;
    while(fread(&app, sizeof(app), 1, fp))
        printApplicant(app);
    fclose (fp);
}

void modifyData()
{
    Applicant app;
    char ID[12];


    printf("Your Identification number");
    scanf("%s", ID);

    FILE *fp=fopen("data.txt", "r");
	FILE *fpTemp=fopen("temp.txt", "w+");

	while (fread(&app, sizeof(app), 1, fp))
    {

		if (strcmp (app.identificationNum, ID) == 0)
        {
            Applicant *tempAPP = readData();
            fwrite(tempAPP, sizeof(Applicant), 1, fpTemp);
		}
        else
			fwrite(&app, sizeof(app), 1, fpTemp);
	}

	fclose(fp);
	fclose(fpTemp);
    remove("data.txt");
	rename("temp.txt", "data.txt");

}
void deleteData()
{
    Applicant app;
    char ID[12];


    printf("Which Identification number would you like to delete? ");
    scanf("%s", ID);

    char *file = "data.txt";
    char *temp = "temp.txt";

    FILE *fp;
    fp = fopen(file, "r");

	FILE *fpTemp;
    fpTemp = fopen(temp, "w");

	while (fread(&app, sizeof(app), 1, fp))
    {
		if (strcmp (app.identificationNum, ID) != 0)
        {
            fwrite(&app, sizeof(app), 1, fpTemp);
		}
	}

	fclose(fp);
	fclose(fpTemp);

    remove("data.txt");
	rename("temp.txt", "data.txt");


}
