#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "threads/synch.h"

struct child_status
{
  tid_t tid;              // ID дочернего процесса
  int exit_status;        // Код завершения ребёнка
  bool exited;            // Флаг "ребёнок уже завершился"
  bool waited;            // Флаг " на него уже вызвали wait "
  struct semaphore sema;  /* Синхронизация родителя и ребёнка,
                             Родитель ждёт пока ребёнок завершится */
  struct semaphore load_sema;
  bool load_success;
  bool load_complete;
  struct list_elem elem;  // Элемент списка детей
};

struct child_status* find_child_status(struct thread *t, tid_t child_tid);
tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);


#endif /* userprog/process.h */
