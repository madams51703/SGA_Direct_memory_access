#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                                   } while (0)

struct shmid_ds shm_info;
char * name_lookup=NULL;
const char *filepath = NULL;
int fixed_area_shmid;
typedef struct ksmfsv_struct 
{ 
	char * full_text;
	char * version  ;
	char * ksmfsnam  ;
	char * ksmfstyp  ;
	int ksmfssiz ;
	long long ksmfsadr;
} ksmfsv_struct ;

ksmfsv_struct ksmfsv[40000] ;
void parse_args(int argc, char * argv[] );


void parse_args(int argc, char * argv[] )
{
	void * fixed_area_addr=(void *) 0x0000000060000000;
  int c;
    int digit_optind = 0;

   while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"add",     required_argument, 0,  0 },
            {"append",  no_argument,       0,  0 },
            {"delete",  required_argument, 0,  0 },
            {"verbose", no_argument,       0,  0 },
            {"create",  required_argument, 0, 'c'},
            {"file",    required_argument, 0,  0 },
            {0,         0,                 0,  0 }
        };

       c = getopt_long(argc, argv, "f:n:s:",
                 long_options, &option_index);
        if (c == -1)
            break;

       switch (c) {


       case 'f':
		filepath = strdup(optarg);
            	break;

       case 'n':
		name_lookup = strdup(optarg);
            	break;

       case 's':
		fixed_area_shmid = atoi(optarg);
		fixed_area_addr = shmat(fixed_area_shmid, fixed_area_addr, SHM_RDONLY);
		shmctl(fixed_area_shmid,IPC_STAT,&shm_info);

		printf("memory size:%d\n",shm_info.shm_segsz); 
               if (fixed_area_addr == (void *) -1)
                   errExit("shmat");
            	break;

       case '?':
            break;

       default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

   if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

}


int main(int argc, char *argv[]){
int i;
char ** throw_away;
char * ksmfsv_ptr;
int ksmfsv_count=0;
char * token;
char * sub_token;
	parse_args(argc, argv);
    int fd = open(filepath, O_RDONLY);
    if(fd < 0){
        printf("\n\"%s \" could not open\n",
               filepath);
        exit(1);
    }

    struct stat statbuf;
    int err = fstat(fd, &statbuf);
    if(err < 0){
        printf("\n\"%s \" could not open\n",
                       filepath);
        exit(2);
    }
    char *ptr = mmap(NULL,statbuf.st_size,
            PROT_READ|PROT_WRITE,MAP_PRIVATE,
            fd,0);
/*
    char *ptr = mmap(NULL,statbuf.st_size,
            PROT_READ|PROT_WRITE,MAP_SHARED,
            fd,0);
*/
    if(ptr == MAP_FAILED){
        printf("Mapping Failed\n");
        return 1;
    }
    close(fd);

    ksmfsv_ptr=ptr;
		token = (char *) 1;
		for (ksmfsv_count = 0 ; token != NULL ; ksmfsv_count++)
		{
			if ( ksmfsv_count == 0)
			{
  				token = strtok(ksmfsv_ptr,"\n");
			}
			else
			{
  				token = strtok(NULL,"\n") ;
			}
			if (token != NULL)
			{
				ksmfsv[ksmfsv_count].full_text = strdup(token);
			}
		}
	ksmfsv_count--; 				
//	printf ("Found %d entries\n",ksmfsv_count);		
	for ( i=0 ; i <ksmfsv_count ; i++)
	{
		ksmfsv[i].version = strdup(strtok(ksmfsv[i].full_text,",") );
		ksmfsv[i].ksmfsnam = strdup(strtok(NULL,",") );
		ksmfsv[i].ksmfstyp = strdup(strtok(NULL,",") );
		ksmfsv[i].ksmfsadr = strtol(strtok(NULL,","),throw_away,16 );
		ksmfsv[i].ksmfssiz = atoi(strtok(NULL,",") );
	}

	for ( i=0 ; i <ksmfsv_count ; i++)
	{
		if ( (name_lookup != NULL) &&strcmp(name_lookup,ksmfsv[i].ksmfsnam) == 0 )
		{
			printf("Found %s, addr: %p, type:%s, size:%d \n",ksmfsv[i].ksmfsnam,ksmfsv[i].ksmfsadr,ksmfsv[i].ksmfstyp,ksmfsv[i].ksmfssiz);
			if (strcmp(ksmfsv[i].ksmfstyp,"oratext *") == 0 )
			{
				printf("Value:%s\n", (char*) ksmfsv[i].ksmfsadr );
			}

			if (strcmp(ksmfsv[i].ksmfstyp,"ub4") == 0 )
			{
				printf("Value:%u\n", *(unsigned int *) ksmfsv[i].ksmfsadr );
			}

			if (strcmp(ksmfsv[i].ksmfstyp,"sword") == 0 )
			{
				printf("Value:%d\n", *(signed int *) ksmfsv[i].ksmfsadr );
			}

			if (strcmp(ksmfsv[i].ksmfstyp,"ub1") == 0 )
			{
				printf("Value:%u\n", *(unsigned char *) ksmfsv[i].ksmfsadr );
			}

			if (strcmp(ksmfsv[i].ksmfstyp,"sb1") == 0 )
			{
				printf("Value:%d\n", *(signed char *) ksmfsv[i].ksmfsadr );
			}


		}
	}
/*
    ssize_t n = write(1,ptr,statbuf.st_size);
    if(n != statbuf.st_size){
        printf("Write failed");
    }

*/

    err = munmap(ptr, statbuf.st_size);
    if(err != 0){
        printf("UnMapping Failed\n");
        return 1;
    }
    return 0;
}

