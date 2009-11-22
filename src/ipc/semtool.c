/*****************************************************************************
 Excerpt from "Linux Programmer's Guide - Chapter 6"
 (C)opyright 1994-1995, Scott Burkett
 ***************************************************************************** 
 MODULE: semtool.c
 *****************************************************************************
 A command line tool for tinkering with SysV style Semaphore Sets

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

/* cygserver doesn't define this constant where we
   can get it, so just use the default configuration
   value */
#ifndef SEMMSL
# define SEMMSL 60
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

#define SEM_RESOURCE_MAX        1       /* Initial value of all semaphores */

void opensem (int *sid, key_t key);
void createsem (int *sid, key_t key, int members);
void locksem (int sid, int member);
void unlocksem (int sid, int member);
void removesem (int sid);
unsigned short get_member_count (int sid);
int getval (int sid, int member);
void dispval (int sid, int member);
void changemode (int sid, char *mode);
void usage (void);

int
main (int argc, char *argv[])
{
  key_t key;
  int semset_id;

  if (argc == 1)
    usage ();

  /* Create unique key via call to ftok() */
  key = ftok (".", 's');

  switch (tolower (argv[1][0]))
    {
    case 'c':
      if (argc != 3)
        usage ();
      createsem (&semset_id, key, atoi (argv[2]));
      break;
    case 'l':
      if (argc != 3)
        usage ();
      opensem (&semset_id, key);
      locksem (semset_id, atoi (argv[2]));
      break;
    case 'u':
      if (argc != 3)
        usage ();
      opensem (&semset_id, key);
      unlocksem (semset_id, atoi (argv[2]));
      break;
    case 'd':
      opensem (&semset_id, key);
      removesem (semset_id);
      break;
    case 'm':
      opensem (&semset_id, key);
      changemode (semset_id, argv[2]);
      break;
    default:
      usage ();

    }

  return (0);
}

void
opensem (int *sid, key_t key)
{
  /* Open the semaphore set - do not create! */

  if ((*sid = semget (key, 0, 0666)) == -1)
    {
      printf ("Semaphore set does not exist!\n");
      exit (1);
    }

}

void
createsem (int *sid, key_t key, int members)
{
  int cntr;
  union semun semopts;

  if (members > SEMMSL)
    {
      printf ("Sorry, max number of semaphores in a set is %d\n", SEMMSL);
      exit (1);
    }

  printf ("Attempting to create new semaphore set with %d members\n",
          members);

  if ((*sid = semget (key, members, IPC_CREAT | IPC_EXCL | 0666)) == -1)
    {
      fprintf (stderr, "Semaphore set already exists!\n");
      exit (1);
    }

  semopts.val = SEM_RESOURCE_MAX;

  /* Initialize all members (could be done with SETALL) */
  for (cntr = 0; cntr < members; cntr++)
    semctl (*sid, cntr, SETVAL, semopts);
}

void
locksem (int sid, int member)
{
  struct sembuf sem_lock = { 0, -1, IPC_NOWAIT };

  if (member < 0 || member > (get_member_count (sid) - 1))
    {
      fprintf (stderr, "semaphore member %d out of range\n", member);
      return;
    }

  /* Attempt to lock the semaphore set */
  if (!getval (sid, member))
    {
      fprintf (stderr, "Semaphore resources exhausted (no lock)!\n");
      exit (1);
    }

  sem_lock.sem_num = member;

  if ((semop (sid, &sem_lock, 1)) == -1)
    {
      fprintf (stderr, "Lock failed\n");
      exit (1);
    }
  else
    printf ("Semaphore resources decremented by one (locked)\n");

  dispval (sid, member);
}

void
unlocksem (int sid, int member)
{
  struct sembuf sem_unlock = { member, 1, IPC_NOWAIT };
  int semval;

  if (member < 0 || member > (get_member_count (sid) - 1))
    {
      fprintf (stderr, "semaphore member %d out of range\n", member);
      return;
    }

  /* Is the semaphore set locked? */
  semval = getval (sid, member);
  if (semval == SEM_RESOURCE_MAX)
    {
      fprintf (stderr, "Semaphore not locked!\n");
      exit (1);
    }

  sem_unlock.sem_num = member;

  /* Attempt to lock the semaphore set */
  if ((semop (sid, &sem_unlock, 1)) == -1)
    {
      fprintf (stderr, "Unlock failed\n");
      exit (1);
    }
  else
    printf ("Semaphore resources incremented by one (unlocked)\n");

  dispval (sid, member);
}

void
removesem (int sid)
{
  union semun semopts;
  semopts.val = 0;

  semctl (sid, 0, IPC_RMID, semopts);
  printf ("Semaphore removed\n");
}

unsigned short
get_member_count (int sid)
{
  union semun semopts;
  struct semid_ds mysemds;

  semopts.buf = &mysemds;

  /* Return number of members in the semaphore set */
  return (semopts.buf->sem_nsems);
}

int
getval (int sid, int member)
{
  int semval;
  union semun semopts;
  semopts.val = 0;

  semval = semctl (sid, member, GETVAL, semopts);
  return (semval);
}

void
changemode (int sid, char *mode)
{
  int rc;
  union semun semopts;
  struct semid_ds mysemds;

  /* Get current values for internal data structure */
  semopts.buf = &mysemds;

  rc = semctl (sid, 0, IPC_STAT, semopts);

  if (rc == -1)
    {
      perror ("semctl");
      exit (1);
    }

  printf ("Old permissions were %o\n", semopts.buf->sem_perm.mode);

  /* Change the permissions on the semaphore */
  sscanf (mode, "%ho", &semopts.buf->sem_perm.mode);

  /* Update the internal data structure */
  semctl (sid, 0, IPC_SET, semopts);

  printf ("Updated...\n");

}

void
dispval (int sid, int member)
{
  int semval;
  union semun semopts;
  semopts.val = 0;

  semval = semctl (sid, member, GETVAL, semopts);
  printf ("semval for member %d is %d\n", member, semval);
}

void
usage (void)
{
  fprintf (stderr, "semtool - A utility for tinkering with semaphores\n");
  fprintf (stderr, "\nUSAGE:  semtool  (c)reate <semcount>\n");
  fprintf (stderr, "                 (l)ock <sem #>\n");
  fprintf (stderr, "                 (u)nlock <sem #>\n");
  fprintf (stderr, "                 (d)elete\n");
  fprintf (stderr, "                 (m)ode <mode>\n");
  exit (1);
}
