#define _XOPEN_SOURCE 700
#include <SDL.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include "timer.h"

// Retourne le nombre de µsec écoulées depuis... un bon moment
static unsigned long get_time (void)
{
  struct timeval tv;

  gettimeofday (&tv ,NULL);

  // Compte seulement les secondes depuis début 2016 (pas le 1er janvier 1970)
  tv.tv_sec -= 3600UL * 24 * 365 * 46;
  
  return tv.tv_sec * 1000000UL + tv.tv_usec;
}

#ifdef PADAWAN

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Structure de la liste d'évènements
typedef struct linked_list
{
  struct itimerval timer;
  void *param;
  unsigned long time_signal;
  struct linked_list *next;
} linked_list;

/*
Insère un évènement event dans la liste ll en le triant dans l'ordre
chronologique de délivrance du signal
*/
void insert(linked_list **ll, linked_list **event)
{
  linked_list *tmp = NULL;
  linked_list *cll = *ll;
  while(cll && cll->time_signal < (*event)->time_signal)
  {
    tmp = cll;
    cll = cll->next;
  }
  (*event)->next = cll;
  if(tmp)
    tmp->next = (*event);
  else
    *ll = (*event);
}

// Supprime le premier élément de la liste, le deuxième devient premier
void pop(linked_list **ll)
{
  linked_list *tmp = (*ll)->next;
  free(*ll);
  *ll = tmp;
}

// Création de la liste nulle d'évènements
linked_list *first_event = NULL;

// Gestionnaire des signaux SIGALRM
void handler(int sig)
{
  // Protection des accès aux structures de données partagées
  pthread_mutex_lock(&mutex);

  /* Si un évènement suit le premier évènement de la liste, on déclenche
     le premier évènement et on arme un temporisateur pour le deuxième */
  if(first_event->next != NULL)
  {
    unsigned long current = first_event->time_signal;
    unsigned long after = first_event->next->time_signal;
    unsigned long diff = after - current;
    sdl_push_event (first_event->param);
    pop(&first_event);
    if(first_event != NULL)
    {
      first_event->timer.it_value.tv_sec = diff/1000000;
      first_event->timer.it_value.tv_usec = (diff%1000000);
      first_event->timer.it_interval.tv_sec = 0;
      first_event->timer.it_interval.tv_usec = 0;
      if(setitimer(ITIMER_REAL, &(first_event->timer), NULL) == -1)
      {
        perror("setitimer handler");
        exit(1);
      }
    }
    else
    {
      return;
    }
  }
  else
  {
      sdl_push_event (first_event->param);
      pop(&first_event);
  }

  // Fin de la protection des accès aux structures de données partagées
  pthread_mutex_unlock(&mutex);
}

/*
Gestionnaire qui va attendre les signaux SIGALRM
(signaux envoyés à un processus lorsqu'une limite de temps s'est écoulée)
et gérer les évènements
*/
void *daemon(void *arg)
{
  sigset_t mask; 
  sigfillset(&mask);
  sigdelset(&mask, SIGALRM);

  struct sigaction sa;
  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);
  
  sigaction(SIGALRM, &sa, NULL);

  while(1)
  {
    sigsuspend(&mask);
  }
}

// Retourne 1 si les temporisateurs sont complètement implémentés, 0 sinon
int timer_init (void)
{
  sigset_t mask; 
  sigemptyset(&mask);
  sigaddset(&mask, SIGALRM);
  pthread_sigmask(SIG_BLOCK, &mask, NULL);

  pthread_t thread;

  if(pthread_create(&thread, NULL, daemon, NULL) == -1)
  {
    perror("pthread_create");
    return EXIT_FAILURE;
  }

  return 1;
}

// Arme un temporisateur
void timer_set (Uint32 delay, void *param)
{
  linked_list *event = malloc(sizeof(struct linked_list));

  // Protection des accès aux structures de données partagées
  pthread_mutex_lock(&mutex);

  event->timer.it_value.tv_sec = delay/1000;
  event->timer.it_value.tv_usec = (delay%1000)*1000;
  event->timer.it_interval.tv_sec = 0;
  event->timer.it_interval.tv_usec = 0;

  event->param = param;

  event->time_signal = get_time() + delay*1000;

  event->next = NULL;

  insert(&first_event, &event);

  if(event == first_event)
  {
    if(setitimer(ITIMER_REAL, &(first_event->timer), NULL) == -1)
    {
      perror("setitimer timer_set");
      exit(1);
    }
  }

  // Fin de la protection des accès aux structures de données partagées
  pthread_mutex_unlock(&mutex);
}

#endif