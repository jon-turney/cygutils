/* $Id$ */

/*
 *   ipcs
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
#include <errno.h>
#include <getopt.h>
*/

/* arg for semctl system calls. */
union semun {
  int val;			/* value for SETVAL */
  struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
  ushort *array;		/* array for GETALL & SETALL */
  struct seminfo *__buf;	/* buffer for IPC_INFO */
  void *__pad;
};

#define LIMITS 1
#define STATUS 2
#define CREATOR 3
#define TIME 4
#define PID 5

void do_sem (char format);
void do_shm (char format);
void do_msg (char format);

static char *progname;

void usage(void)
{
	printf ("usage : %s -asmq -tclup \n", progname);
	printf ("\t%s [-s -m -q] -i id\n", progname);
	printf ("\t%s -h for help.\n", progname);
	return;
}

void help (void)
{
	printf ("%s provides information on ipc facilities for", progname);
        printf (" which you have read access.\n");
	printf ("Resource Specification:\n\t-m : shared_mem\n\t-q : messages\n");
	printf ("\t-s : semaphores\n\t-a : all (default)\n");
	printf ("Output Format:\n\t-t : time\n\t-p : pid\n\t-c : creator\n");
	printf ("\t-l : limits\n\t-u : summary\n");
	printf ("-i id [-s -q -m] : details on resource identified by id\n");
	usage();
	return;
}

int main (int argc, char **argv)
{
	int opt, msg = 0, sem = 0, shm = 0, id=0, print=0;
	char format = 0;
	char options[] = "atcluphsmqi:";
	
	progname = argv[0];
	while ((opt = getopt (argc, argv, options)) != EOF) {
		switch (opt) {
		case 'i':
			id = atoi (argv[2]);
			print = 1;
			break;
		case 'a':
			msg = shm = sem = 1;
			break;
		case 'q':
			msg = 1;
			break;
		case 's':
			sem = 1;
			break;
		case 'm':
			shm = 1;
			break;
		case 't':
			format = TIME;
			break;
		case 'c':
			format = CREATOR;
			break;
		case 'p':
			format = PID;
			break;
		case 'l':
			format = LIMITS;
			break;
		case 'u':
			format = STATUS;
			break;
		case 'h':
			help();
			exit (0);
		case '?':
			usage();
			exit (0);
		}
	}

	if ( !shm && !msg && !sem)
		msg = sem = shm = 1;
	printf ("\n");
	if (shm) {
		do_shm (0);
		printf ("\n");
	}
	if (sem) {
		do_sem (0);
		printf ("\n");
	}
	if (msg) {
		do_msg (0);
		printf ("\n");
	}
	return 0;
}

void do_shm (char format)
{
	int maxid, shmid, id;
	struct shmid_ds shmseg;
	struct shm_info shm_info;
	struct ipc_perm *ipcp = &shmseg.shm_perm;

	maxid = shmctl (0, SHM_INFO, (struct shmid_ds *) &shm_info);
	if (maxid < 0)
	{
		printf ("kernel not configured for shared memory\n");
		return;
	}

	printf ("---------- Shared Memory Segments --------\n");
	printf ("     %-10s%-10s%-10s%-10s%-12s\n", "shmid", "key",
			"bytes","nattch","status");

	for (id = 0; id <= maxid; id++)
	{
		shmid = shmctl (id, SHM_STAT, &shmseg);
		if (shmid  < 0)
			continue;

		printf ("_shm %-10d", shmid);
		printf ("%-10d%-10d%-10d%-6s%-6s\n",
			(int)shmseg.shm_perm.key, shmseg.shm_segsz,
			shmseg.shm_nattch,
			ipcp->mode & SHM_DEST ? "dest" : " ",
			ipcp->mode & SHM_LOCKED ? "locked" : " ");
	}
	return;
}

void do_sem (char format)
{
	int maxid, semid, id;
	struct semid_ds semary;
	struct seminfo seminfo;
	union semun arg;
	int nb_curr,i;

	arg.array = (ushort *)  &seminfo;
	maxid = semctl (0, 0, SEM_INFO, arg);
	if (maxid < 0) {
		printf ("kernel not configured for semaphore\n");
		return;
	}

	printf ("---------- Semaphore Arrays --------\n");
	printf ("     %-10s%-10s%-10s\n",
		"semid","nsems","key");

	for (id = 0; id <= maxid; id++) {
		arg.buf = (struct semid_ds *) &semary;
		semid = semctl (id, 0, SEM_STAT, arg);
		if (semid < 0)
			continue;
		printf ("_sem %-10d", semid);
		printf ("%-10d%-10d currents: ",
		semary.sem_nsems, (int) semary.sem_perm.key);

		for (i = 0; i < semary.sem_nsems; i++) {
		 nb_curr = semctl (semid, i, GETVAL, arg);
		 if( nb_curr >= 0 )
		 {
		  printf ("%-2d ", nb_curr);
		 }
		 else
		 {
		  printf ("%-2d,errno=%d ", nb_curr, errno);
		 }
		}
		printf ("\n");
	}
	return;
}

void do_msg (char format)
{
	int maxid, msqid, id;
	struct msqid_ds msgque;
	struct msginfo msginfo;

	maxid = msgctl (0, MSG_INFO, (struct msqid_ds *) &msginfo);
	if (maxid < 0) {
		printf ("kernel not configured for message queue\n");
		return;
	}

	printf ("---------- Message Queues --------\n");
	printf ("     %-10s%-12s%-12s\n", "msqid",
		"used-bytes", "messages");

	for (id = 0; id <= maxid; id++) {
		msqid = msgctl (id, MSG_STAT, &msgque);
		if (msqid  < 0)
			continue;
				printf ("_msg %-10d", msqid);
			printf ("%-12d%-12d\n",
			msgque.msg_cbytes,
				msgque.msg_qnum);
	}
	return;
}
