/**
 * putclip.exe   copy stdin to Windows clipboard
 *
 * Copyright 2001,... by Charles Wilson <cwilson@ece.gatech.edu>.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * See the COPYING file for license information.
 */

#include "common.h"
#include <malloc.h>
#define CLIPCHUNK 1024

static const char versionID[] = 
    "0.1.1";
/* for future CVS */
static const char revID[] =
    "$Id$";
static const char copyrightID[] =
    "Copyright (c) 2001,...\nCharles S. Wilson. All rights reserved.\nLicensed under GPL v2.0\n";

typedef struct
{
  int unixflag;
  int dosflag;
} flags_struct;

static void printTopDescription(FILE * f, char * name);
static void printBottomDescription(FILE * f, char * name);
static const char * getVersion(void);
static void usage(poptContext optCon, FILE * f, char * name);
static void help(poptContext optCon, FILE * f, char * name);
static void version(poptContext optCon, FILE * f, char * name);
static void license(poptContext optCon, FILE * f, char * name);
static int putclip(FILE * in, flags_struct flags, FILE * f, char * name);

static char * program_name;
 
int main(int argc, const char ** argv)
{
  poptContext optCon;
  const char ** rest;
  int rc;
  int ec = 0;
  flags_struct flags = { FALSE, FALSE };

  struct poptOption generalOptionsTable[] = {
    { "dos",  'd',  POPT_ARG_NONE, NULL, 'd', \
        "Clipboard text will have DOS line endings.", NULL},
    { "unix", 'u', POPT_ARG_NONE, NULL, 'U', \
        "Clipboard text will have UNIX line endings.", NULL},
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };

  struct poptOption helpOptionsTable[] = {
    { "help",  '?',  POPT_ARG_NONE, NULL, '?', \
        "Show this help message", NULL},
    { "usage", '\0', POPT_ARG_NONE, NULL, 'u', \
        "Display brief usage message", NULL},
    { "version", '\0', POPT_ARG_NONE, NULL, 'v', \
        "Display version information", NULL},
    { "license", '\0', POPT_ARG_NONE, NULL, 'l', \
        "Display licensing information", NULL},
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };

  struct poptOption opt[] = {
    { NULL, '\0', POPT_ARG_INCLUDE_TABLE, generalOptionsTable, 0, \
        "General options", NULL },
    { NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0, \
        "Help options", NULL },
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };

  if( (program_name = strdup(argv[0])) == NULL ) {
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(1);
  }
  optCon = poptGetContext(NULL, argc, argv, opt, 0);

  while ((rc = poptGetNextOpt(optCon)) > 0) {
    switch (rc) {
      case '?':  help(optCon, stderr, program_name);
                 goto exit;
      case 'u':  usage(optCon, stderr, program_name);
                 goto exit;
      case 'v':  version(optCon, stderr, program_name);
                 goto exit;
      case 'l':  license(optCon, stderr, program_name);
                 goto exit;
      case 'd':  flags.dosflag = TRUE;
                 break;
      case 'U':  flags.unixflag = TRUE;
                 break;
    }
  }
  if (rc < -1 ) {
    fprintf(stderr, "%s: bad argument %s: %s\n",
      program_name, poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
      poptStrerror(rc));
    ec = 2;
    goto exit;
  }
  if (flags.dosflag && flags.unixflag) {
    fprintf(stderr, "%s: can't specify both --unix and --dos (-u and -d)\n", program_name);
    ec = 2;
    goto exit;
  }
  rest = poptGetArgs(optCon);

  if (rest == NULL)
    ec |= putclip(stdin, flags, stderr, program_name);
  else {
    fprintf(stderr, "Extra args ignored: ");
    while (*rest) 
      fprintf(stderr, "%s ", *rest++);
    fprintf(stderr, "\n");
    ec |= putclip(stdin, flags, stderr, program_name);
  }

exit:
  poptFreeContext(optCon);
  free(program_name);
  return(ec);
}

static const char * getVersion()
{
  return versionID;
}

static void printTopDescription(FILE * f, char * name)
{
  fprintf(f, "%s version %s\n", name, getVersion());
  fprintf(f, "  Copy stdin to the Windows Clipboard\n\n");
}

static void printBottomDescription(FILE * f, char * name)
{
  fprintf(f, "\nWARNING: --dos and --unix are not yet implemented.  Pipe input\n");
  fprintf(f,   "through d2u or u2d as a workaround for now.\n");
}

static printLicense(FILE * f, char * name)
{
  fprintf(f, "This program is free software; you can redistribute it and/or\n");
  fprintf(f, "modify it under the terms of the GNU General Public License\n");
  fprintf(f, "as published by the Free Software Foundation; either version 2\n");
  fprintf(f, "of the License, or (at your option) any later version.\n");
  fprintf(f, "\n");
  fprintf(f, "This program is distributed in the hope that it will be useful,\n");
  fprintf(f, "but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
  fprintf(f, "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
  fprintf(f, "GNU General Public License for more details.\n");
  fprintf(f, "\n");
  fprintf(f, "You should have received a copy of the GNU General Public License\n");
  fprintf(f, "along with this program; if not, write to the Free Software\n");
  fprintf(f, "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n");
  fprintf(f, "\n");
  fprintf(f, "See the COPYING file for license information.\n");
}

static void usage(poptContext optCon, FILE * f, char * name)
{
  poptPrintUsage(optCon, f, 0);
}

static void help(poptContext optCon, FILE * f, char * name)
{
  printTopDescription(f, name);
  poptPrintHelp(optCon, f, 0);
  printBottomDescription(f, name);
}

static void version(poptContext optCon, FILE * f, char * name)
{
  printTopDescription(f, name);
}

static void license(poptContext optCon, FILE * f, char * name)
{
  printTopDescription(f, name);
  printLicense(f, name);
}  

int putclip(FILE * in, flags_struct flags, FILE * f, char * name)
{
   HANDLE hData;  /* handle to clip data */
   LPSTR lpData;  /* pointer to clip data */
   LPSTR lpszText;
   char chunk[CLIPCHUNK+1];
   char *buf = NULL;
   char *buf_old = NULL;
   char *ptr;
   int len = 0;
   int numread;

   /* Since we have to copy to the clipboard all in one step,
      read entire stream into memory. However, let's try to 
      do this using the minimum amount of memory, but keep 
      allocating more if needed.
   */
   do
   {
      numread = fread(chunk,sizeof(char),CLIPCHUNK,in);
      if(numread > 0)
      {
	 if (buf)
	 {
	    if ((buf_old = malloc (len * sizeof(char))) == NULL)
	    {
               fprintf(stderr, "%s: memory allocation error\n", name);
	       return(FALSE);
	    }
	    memcpy(buf_old, buf, len);
	    free(buf);
	 }
         if ((buf = malloc ((len+numread) * sizeof(char))) == NULL)
	 {
            fprintf(stderr, "%s: memory allocation error\n", name);
	    return(FALSE);
	 }
	 if (buf_old)
	 {
	    memcpy(buf,buf_old,len);
	    free(buf_old);
	 }
	 ptr = &(buf[len]);
	 memcpy(ptr,chunk,numread);
	 len += numread;
      }
   }
   while(!feof(in) && !ferror(in));

   /* Allocate memory and copy the string to it */
   if (!(hData 
	 = GlobalAlloc(GHND, len+1))) {
      return (TRUE);
   }
   if (!(lpData = GlobalLock(hData))) {
      return (TRUE);
   }

   strcpy((char *)lpData, buf);
   GlobalUnlock(hData);
    
   /* Clear the current contents of the clipboard, and set
    * the data handle to the new string.
    */
   
   if (OpenClipboard(0)) {
      EmptyClipboard();
      SetClipboardData(CF_TEXT, hData);
      CloseClipboard();
   }
   hData = NULL;
   return (TRUE);
}
