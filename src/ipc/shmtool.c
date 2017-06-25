/*****************************************************************************
 Excerpt from "Linux Programmer's Guide - Chapter 6"
 (C)opyright 1994-1995, Scott Burkett
 ***************************************************************************** 
 MODULE: shmtool.c
 *****************************************************************************
 A command line tool for tinkering with shared memory 

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
#if HAVE_SYS_SHM_H
# include <sys/shm.h>
#endif

#define SEGSIZE 100

int
usage ()
{
  fprintf (stderr, "shmtool - A utility for tinkering with shared memory\n");
  fprintf (stderr, "\nUSAGE:  shmtool (w)rite <text>\n");
  fprintf (stderr, "                (r)ead\n");
  fprintf (stderr, "                (d)elete\n");
  fprintf (stderr, "                (m)ode change <octal mode>\n");
  exit (1);
}

int
writeshm (int shmid, char *segptr, char *text)
{
  strcpy (segptr, text);
  printf ("Done...\n");
}

int
readshm (int shmid, char *segptr)
{
  printf ("segptr: %s\n", segptr);
}

int
removeshm (int shmid)
{
  shmctl (shmid, IPC_RMID, 0);
  printf ("Shared memory segment marked for deletion\n");
}

int
changemode (int shmid, char *mode)
{
  struct shmid_ds myshmds;

  /* Get current values for internal data structure */
  shmctl (shmid, IPC_STAT, &myshmds);

  /* Display old permissions */
  printf ("Old permissions were: %o\n", myshmds.shm_perm.mode);

  /* Convert and load the mode */
  sscanf (mode, "%o", &myshmds.shm_perm.mode);

  /* Update the mode */
  shmctl (shmid, IPC_SET, &myshmds);

  printf ("New permissions are : %o\n", myshmds.shm_perm.mode);
}

int
main (int argc, char *argv[])
{
  key_t key;
  int shmid, cntr;
  char *segptr;

  if (argc == 1)
    usage ();

  /* Create unique key via call to ftok() */
  key = ftok (".", 'S');

  /* Open the shared memory segment - create if necessary */
  if ((shmid = shmget (key, SEGSIZE, IPC_CREAT | IPC_EXCL | 0666)) == -1)
    {
      printf ("Shared memory segment exists - opening as client\n");

      /* Segment probably already exists - try as a client */
      if ((shmid = shmget (key, SEGSIZE, 0)) == -1)
        {
          perror ("shmget");
          exit (1);
        }
    }
  else
    {
      printf ("Creating new shared memory segment\n");
    }

  /* Attach (map) the shared memory segment into the current process */
  if ((segptr = shmat (shmid, 0, 0)) == ((char *) -1))
    {
      perror ("shmat");
      exit (1);
    }

  switch (tolower (argv[1][0]))
    {
    case 'w':
      writeshm (shmid, segptr, argv[2]);
      break;
    case 'r':
      readshm (shmid, segptr);
      break;
    case 'd':
      removeshm (shmid);
      break;
    case 'm':
      changemode (shmid, argv[2]);
      break;
    default:
      usage ();

    }
}
