#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "error.h"
#include "helplib.h"

#ifdef PADAWAN

#define GROUND_ID 0
#define WALL_ID 1
#define GRASS_ID 2 
#define HERB_ID 3
#define FLOOR_ID 4
#define MARBLE_ID 5
#define MARBLE2_ID 6
#define COIN_ID 7
#define QUESTION_ID 8
#define FLOWER_ID 9



void map_new (unsigned width, unsigned height)
{
  map_allocate (width, height);

  for (int x = 0; x < width; x++){
    map_set (x, height - 1, GROUND_ID); // Ground
    if ( x%4==0 && x!=0)
      map_set(x,height-2, FLOWER_ID); //flower
    if ( x%6==0 && x!=0)
      map_set(x,height-3, COIN_ID); //pieces
  }
for (int y = 0; y < height - 1; y++) {
    map_set (0, y, WALL_ID); // Wall
    map_set (width - 1, y, WALL_ID); // Wall
  }
  
 

  map_object_begin (10);

  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_SOLID); //0
  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID); //1
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID); //2
  map_object_add ("images/herb.png", 1, MAP_OBJECT_AIR); //3
  map_object_add ("images/floor.png", 1, MAP_OBJECT_SEMI_SOLID); //4
  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE); //5
  map_object_add ("images/marble2.png", 1, MAP_OBJECT_SOLID); //6
  map_object_add ("images/coin.png", 20, MAP_OBJECT_AIR | MAP_OBJECT_COLLECTIBLE | MAP_OBJECT_DESTRUCTIBLE); //7
  map_object_add ("images/question.png", 20, MAP_OBJECT_SOLID | MAP_OBJECT_GENERATOR);// 8
  map_object_add ("images/flower.png", 1, MAP_OBJECT_AIR);//9
  map_object_end ();

}

void map_save (char *filename)
{
  int fd = open(filename,O_CREAT | O_TRUNC | O_WRONLY, 0666);
  unsigned height = map_height();
  unsigned width = map_width();
  unsigned nb_objects = map_objects();
  write(fd, &height, sizeof(height));
  write(fd, &width, sizeof(width));
  write(fd, &nb_objects, sizeof(nb_objects));
  for (int y = 0; y < height; y++){
    for(int x = 0; x < width; x++){
      int data = map_get(x,y);
      write(fd, &data, sizeof(data));
    }
  } 

  //ca c'est pour le type des objets. #génial
  char * obj = "util/objets.txt";
  int objects = open(obj, O_RDONLY);
  int map_blocks = open("maps/map_blocks.save", O_WRONLY | O_CREAT | O_TRUNC, 0666);
  lseek(objects, 1, SEEK_SET);
  int r = 1;
  char c;
  int cpt = 0;

  int line = 0;
  while(r >=1){ //tant qu'on est pas à la fin du fichier
    cpt = 0;
    r = read(objects, &c, sizeof(char));

    while(c>34){ //on compte le nombre de caractère que fait le nom du fichier
      cpt++;
      r = read(objects, &c, sizeof(char));
    }

    write(map_blocks, &cpt, sizeof(int));//on écrit dans le fichier de sauvegarde le nombre de caractères
    
    lseek(objects, cpt*-1 - 1, SEEK_CUR); //on reviens à la position avant qu'on compte le nombre de charactere

    for(int i=0 ; i<cpt ; ++i){ //on ecrit le nom du fichier
      r = read(objects, &c, 1);
      write(map_blocks, &c, 1);
    }
    
    while(c<48 || c>57){ //tant que le caractère lu n'est pas entre 0 et 9
      read(objects, &c, 1);
    }

    cpt = 0;
    while(c>34){ //tant que le character lu n'est pas une tabulation ni un espace
      read(objects, &c, 1);
      cpt++;
    }

    char str[6]="";
    lseek(objects, cpt*-1 -1, SEEK_CUR); //on le remet où il était
    for(int i =0 ; i<cpt ; ++i){
      read(objects, &c, 1);
      strncat(str,&c,1);
    }

    int n = atoi(str);
    write(map_blocks, &n, sizeof(int));
    


    //la on vient d'écrire le nombre de frames. Youhou.
    read(objects, &c, 1);

    while(c<33)
      read(objects, &c, 1);

    int tmp = 0;
    if(c=='a'){
      write(map_blocks, &tmp, sizeof(int));
    }
    else{
      read(objects, &c, 1);
      if(c=='e'){
        tmp = 1;
        write(map_blocks, &tmp, sizeof(int));
      }
      else{
        tmp=2;
        write(map_blocks, &tmp, sizeof(int));
      }
      lseek(objects, -2, SEEK_CUR);
      read(objects, &c, 1);
    }
    
    while(c>34){
      read(objects, &c, 1);
    }

    while(c<33){
      read(objects, &c, 1);
    }

    if(c=='d'){
      tmp=4;
      write(map_blocks, &tmp, sizeof(int));
    }
    else{
      tmp = 0;
      write(map_blocks, &tmp, sizeof(int));
    }
    
    while(c>34){
      read(objects, &c, 1);
    }

    while(c<33)
      read(objects, &c, 1);

    if(c=='c'){
      tmp = 8;
      write(map_blocks, &tmp, sizeof(int));
    }
    else{
      tmp = 0;
      write(map_blocks, &tmp, sizeof(int));
    }

    while(c>34){
      read(objects, &c, 1);
    }

    while(c<33)
      read(objects, &c, 1);

    if(c=='g'){
      tmp=16;
      write(map_blocks, &tmp, sizeof(int));
    }
    else{
      tmp = 0;
      write(map_blocks, &tmp, sizeof(int));
    }
    while(c!='\"' && r>0){
      r = read(objects, &c, 1);
    }
  }
  close(fd);
  close(objects);
  close(map_blocks);

  //ne devrait oas petre là mais on ne peux pas modifier le code pour ajouter une option --testsavemap
  //je laisse le commentaire là, décommenter pour tester
  //test_save_map();
}

void map_load (char *filename)
{
  
int load = open(filename, O_RDONLY);
  unsigned height, width,nb_objects;
  int type;
  
  read(load, &height, sizeof(height));
  read(load, &width, sizeof(width));
  read(load, &nb_objects,sizeof(nb_objects));
  map_allocate (width, height);
  for(int y=0; y<height; y++){
    for(int x=0; x<width; x++){
      read(load, &type, sizeof(type));
      map_set(x, y, type);
    }
  }
  close(load);


  int objects = open("maps/map_blocks.save", O_RDONLY);
  int n, frames, solidity, destructible, collectible, generator;
  map_object_begin (10);
  int cpt = 0;
  while(1){
    int r = read(objects, &n, sizeof(int));
    if(r==0)
      break;
    char filename[n];
    read(objects, &filename, n);
    printf("n=%d ", n);
    read(objects, &frames, sizeof(int));
    read(objects, &solidity, sizeof(int));
    read(objects, &destructible, sizeof(int));
    if(destructible==0)
      destructible = solidity;
    read(objects, &collectible, sizeof(int));
    if(collectible==0)
      collectible = solidity;
    read(objects, &generator, sizeof(int));
    if(generator==0)
      generator = solidity;
    if(cpt==2)
      map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID);
    else if(cpt==4)
      map_object_add ("images/floor.png", 1, MAP_OBJECT_SEMI_SOLID);
    else
      map_object_add(filename, frames, solidity | destructible | collectible | generator); 
    cpt++;
  }

  
/*
  // Texture pour le sol
  map_object_add ("images/ground.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_SOLID); //0
  map_object_add ("images/wall.png", 1, MAP_OBJECT_SOLID); //1
  map_object_add ("images/grass.png", 1, MAP_OBJECT_SEMI_SOLID); //2
  map_object_add ("images/herb.png", 1, MAP_OBJECT_AIR); //3
  map_object_add ("images/floor.png", 1, MAP_OBJECT_SEMI_SOLID); //4
  map_object_add ("images/marble.png", 1, MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE); //5
  map_object_add ("images/marble2.png", 1, MAP_OBJECT_SOLID); //6
  map_object_add ("images/coin.png", 20, MAP_OBJECT_AIR | MAP_OBJECT_COLLECTIBLE | MAP_OBJECT_DESTRUCTIBLE); //7
  map_object_add ("images/question.png", 20, MAP_OBJECT_SOLID | MAP_OBJECT_GENERATOR);// 8
  map_object_add ("images/flower.png", 1, MAP_OBJECT_AIR);//9

  */
}

#endif
