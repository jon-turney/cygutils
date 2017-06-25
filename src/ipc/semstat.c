/*****************************************************************************
 Excerpt from "Linux Programmer's Guide - Chapter 6"
 (C)opyright 1994-1995, Scott Burkett
 ***************************************************************************** 
 MODULE: semstat.c
 *****************************************************************************
 A companion command line tool for the semtool package.  semstat displays
 the current value of all semaphores in the set created by semtool.
 *****************************************************************************/
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See the COPYING file for full license information.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "common.h"

#if HAVE_SYS_IPC_H
# include <sys/ipc.h>
#endif
#if HAVE_SYS_SEM_H
# include <sys/sem.h>
#endif

/* arg for semctl system calls. */
union semun
{
  int val;                      /* value for SETVAL */
  struct semid_ds *buf;         /* buffer for IPC_STAT & IPC_SET */
  ushort *array;                /* array for GETALL & SETALL */
  struct seminfo *__buf;        /* buffer for IPC_INFO */
  void *__pad;
};

int get_sem_count (int sid);
void show_sem_usage (int sid);
int get_sem_count (int sid);
void dispval (int sid);

int
main (int argc, char *argv[])
{
  key_t key;
  int semset_id;

  /* Create unique key via call to ftok() */
  key = ftok (".", 's');

  /* Open the semaphore set - do not create! */
  if ((semset_id = semget (key, 1, 0666)) == -1)
    {
      printf ("Semaphore set does not exist\n");
      exit (1);
    }

  show_sem_usage (semset_id);
  return (0);
}

void
show_sem_usage (int sid)
{
  int cntr = 0, maxsems, semval;

  maxsems = get_sem_count (sid);

  while (cntr < maxsems)
    {
      semval = semctl (sid, cntr, GETVAL, 0);
      printf ("Semaphore #%d:  --> %d\n", cntr, semval);
      cntr++;
    }
}

int
get_sem_count (int sid)
{
  int rc;
  struct semid_ds mysemds;
  union semun semopts;

  /* Get current values for internal data structure */
  semopts.buf = &mysemds;

  if ((rc = semctl (sid, 0, IPC_STAT, semopts)) == -1)
    {
      perror ("semctl");
      exit (1);
    }

  /* return number of semaphores in set */
  return (semopts.buf->sem_nsems);
}

void
dispval (int sid)
{
  int semval;

  semval = semctl (sid, 0, GETVAL, 0);
  printf ("semval is %d\n", semval);
}
