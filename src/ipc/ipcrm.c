/* $Id$ */

/*
 *   ipcrm
 *
 *   Copyright (C) 1997 Philippe CHAPUY
 *   Copyright (C) 1998 Ludovic LANGE
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 *   HISTORY:
 *   --------
 *
 *   13/05/1998 : Version 1.00 released
 *                First public release
 *                adress any comments to llange@capgemini.fr
 *   2003/06/13 : (C. Wilson) Ported to the cygutils package from 
 *                the cygipc package.
 *
 */

/************************************************************************/
/* Philippe Chapuy, le 19/05/97						*/
/************************************************************************/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include "common.h"

#if HAVE_SYS_IPC_H
# include <sys/ipc.h>
# if HAVE_SYS_SEM_H
#  include <sys/sem.h>
# endif
# if HAVE_SYS_SHM_H
#  include <sys/shm.h>
# endif
# if HAVE_SYS_MSG_H
#  include <sys/msg.h>
# endif
#endif

/* These includes moved to common.h
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
*/

/* arg for semctl system calls. */
union semun {
  int val;			/* value for SETVAL */
  struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
  ushort *array;		/* array for GETALL & SETALL */
  struct seminfo *__buf;	/* buffer for IPC_INFO */
  void *__pad;
};

int main(int argc, char **argv)
{
	int id;
	union semun arg;
	int LBoucle ;

	arg.val = 0;
	
	if (argc < 3 || strlen(argv[1]) < 3) {
		printf ("usage: %s [shm | msg | sem] id1 ... idn\n", argv[0]);
		exit (1);
	}
	id = atoi (argv[2]);
	switch (argv[1][1])  {
	case 'h':
		for (LBoucle = 2; LBoucle < argc; LBoucle++)
		{
		 id = atoi (argv[LBoucle]) ;
		 if (!shmctl (id, IPC_RMID, NULL))
		 continue ;
		 perror ("shmctl ");
		}
		exit (1);

	case 'e':
		for (LBoucle = 2; LBoucle < argc; LBoucle++)
		{
		 id = atoi (argv[LBoucle]) ;
		 if (!semctl (id, 0, IPC_RMID, arg))
		 continue ;
		 perror ("semctl ");
		}
		exit (1);

	case 's':
		for (LBoucle = 2; LBoucle < argc; LBoucle++)
		{
		 id = atoi (argv[LBoucle]) ;
		 if (!msgctl (id, IPC_RMID, NULL))
		 continue ;
		 perror ("msgctl ");
		}
		exit (1);

	default:
		printf ("usage: %s [-shm | -msg | -sem] id\n", argv[0]);
		exit (1);
	}
	sleep(2) ;
	printf ("resource deleted\n");
	return 0;
}

