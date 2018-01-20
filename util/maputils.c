#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>



unsigned get_height(const char* filename){
	int fd = open(filename, O_RDONLY);
	unsigned height = 0;
	int r = read(fd, &height, sizeof(unsigned));
	return height;
}

unsigned get_width(const char* filename){
	int fd = open(filename, O_RDONLY);
	unsigned width = 0;
	lseek(fd, sizeof(unsigned), SEEK_SET);
	int r = read(fd, &width, sizeof(unsigned));
	return width;
}

unsigned get_objects(const char* filename){
	int fd = open(filename, O_RDONLY);
	unsigned objects = 0;
	lseek(fd, 2*sizeof(unsigned), SEEK_SET);
	int r = read(fd, &objects, sizeof(unsigned));
	return objects;
}

void get_info(const char* filename){
    printf("height = %u, width = %u, objects = %u\n", get_height(filename),get_width(filename), get_objects(filename));
}

void set_height(const char* filename, unsigned new_height){
    int fd = open(filename, O_RDWR, 777);
    unsigned original_height=0;
    unsigned original_width=0;
    unsigned nb_objects = 0;
    read(fd, &original_height, sizeof(unsigned));
    read(fd, &original_width, sizeof(unsigned));
    read(fd, &nb_objects,sizeof(unsigned));
    int descr[2];
    pipe(descr);
    //agrandissement 
    for(int j = 0; j < (int) (new_height - original_height); j++){
        //ajouter case vide
        int buff = -1;
        for (int i =0; i < original_width; i++)
        {
            write(descr[1], &buff, sizeof(buff));
        }
    }
    int kept_height;
    if (original_height > new_height) {
        kept_height = new_height;
    } else {
        kept_height = original_height;
    }
    lseek(fd, (original_height - kept_height) * original_width * sizeof(int), SEEK_CUR);
    for (int i = 0; i < kept_height; i++){
        //taille d'une ligne du tableau buff;
        //read(fd, buff, sizeof(buff)) ;
        //write(decr[1], buff, sizeof(buff));
        int buffer;
        for (int i =0; i < original_width; i++)
        {
            read(fd, &buffer, sizeof(int));
            write(descr[1], &buffer, sizeof(buffer));
        }
    }
    lseek(fd,0, SEEK_SET);
    write(fd,&new_height, sizeof(new_height));
    write(fd,&original_width,sizeof(original_width));
    write(fd,&nb_objects, sizeof(nb_objects));
    
    int buffer2;
    for (int i = 0; i < (int) (new_height * original_width); i++)
    {
        read(descr[0], &buffer2, sizeof(int));
        write(fd, &buffer2, sizeof(int));
    }
    close(descr[0]);
    close(descr[1]);
    close(fd);
}

void set_width(const char* filename, unsigned new_width){
    int fd = open(filename, O_RDWR, 777);
    unsigned original_height=0;
    unsigned original_width=0;
    unsigned nb_objects = 0;
    read(fd, &original_height, sizeof(unsigned));
    read(fd, &original_width, sizeof(unsigned));
    read(fd, &nb_objects,sizeof(unsigned));
    int descr[2];
    pipe(descr);
    //agrandissement 
    for (int i= 0; i<original_height;i++){
        //taille d'une ligne du tableau buff;
        //read(fd, buff, sizeof(buff)) ;
        //write(decr[1], buff, sizeof(buff));
        int kept_width;
        if (original_width > new_width) {
            kept_width = new_width;
        } else {
            kept_width = original_width;
        }

        int buffer;
        for (int i = 0; i < kept_width; i++)
        {
            read(fd, &buffer, sizeof(int));
            write(descr[1], &buffer, sizeof(buffer));
        }
        lseek(fd, (original_width - kept_width) * sizeof(int), SEEK_CUR);
        
        for(int j = 0; j < (int) (new_width - original_width); j++){
            //ajouter case vide
            int buff = -1;
            write(descr[1], &buff, sizeof(buff));
        }
    }
    lseek(fd,0, SEEK_SET);
    write(fd,&original_height, sizeof(original_height));
    write(fd,&new_width,sizeof(new_width));
    write(fd,&nb_objects, sizeof(nb_objects));
    
    int buffer2;
    for (int i = 0; i < original_height * new_width; i++)
    {
        read(descr[0], &buffer2, sizeof(int));
        write(fd, &buffer2, sizeof(int));
    }
    close(descr[0]);
    close(descr[1]);
    close(fd);}

void set_map(const char* filename, unsigned new_width, unsigned new_height){

}

void set_objects(char * savefile, int nb_frames, int solidity, int destructible, int collectible, int generator){
//./util/maputil maps/saved.map --setobjects "images/ground.png" 1 solid ot-destructible not-collectible not-generator 
//"images/wall.png" 1 solid not-destructible not-collectible not-generator
//
    int fd = open("maps/map_blocks.save", O_RDONLY);
    int n, r;
    while(1){
        r = read(fd, &n, sizeof(int));
        if(r==0)
            break;
        char filename[n];
        read(fd, &filename, n);
        if(filename==savefile){
            n = nb_frames;
            write(fd, &n, sizeof(int));
            n = solidity;
            write(fd, &n, sizeof(int));
            n = destructible;
            write(fd, &n, sizeof(int));
            n = collectible;
            write(fd, &n, sizeof(int));
            n = generator;
            write(fd, &n, sizeof(int));
        }
        else {
            perror("No such object(s).");
        }
    }
}


/*fichier passé en argument*/
int main(int argc, char const *argv[])
{
	if(argc<1){
		perror("not enough parameters\n");
		exit(1);
	}	
	const char * filename = argv[1];

	/*12 32 1*/
    for (int i = 1; i < argc; i++){  /* Skip argv[0] (program name). */
    	if (strcmp(argv[i], "--getwidth") == 0){  
            get_width(filename);
        }
        else if (strcmp(argv[i], "--getheight") == 0){  
            get_height(filename);
        }
        else if (strcmp(argv[i], "--getobjects") == 0){  
            get_objects(filename);
        }
        else if (strcmp(argv[i], "--getinfo") == 0){  
            get_info(filename);
        }
        else if (strcmp(argv[i], "--setwidth") == 0){  /* Process optional arguments. */
            if (i + 1 < argc){  /* There are enough arguments in argv. */
                ++i;
                unsigned w = atoi(argv[i]);  /* Convert string to int. */
                set_width(filename, w);
            }
            else{
                perror("Please insert a width.\n");
            }
        }
        else if (strcmp(argv[i], "--setheight") == 0){  
            if (i + 1 < argc){ 
            	++i;
                unsigned h = atoi(argv[i]);
                set_height(filename, h);
            }  
            else{
                perror("Please insert a height.\n");
            }
        }
        else if (strcmp(argv[i], "--setobjects") == 0){ 
            set_objects("images/coin.png", 20, 0, 0, 0, 0)
        }
        /*else if (strcmp(argv[i], "--setobjects") == 0){  
            ECRIRE LE FOR I IN ARGC
            if (i + 6 < argc){
            	++i;  
                char* file = argv[i];
                ++i;
                int nb_frames = atoi(argv[i]);
                ++i;
                int solidity = atoi(argv[i]);
                ++i;
                int destructible = atoi(argv[i]);
                ++i;
                int collectible = atoi(argv[i]);
                ++i;
                int generator = atoi(argv[i]);
                set_objects(file, nb_frames, solidity, destructible, collectible, generator);
            }
            else{
                perror("Please insert the correct arguments.\n")
            }
        }*/
    }
    //get_info(filename);
	return 0;
}

/*
prune_objects()

*/