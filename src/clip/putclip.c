/**
 * putclip.exe   copy stdin to Windows clipboard
 *
 * Copyright 2001,... by Charles Wilson <cwilson@ece.gatech.edu>.
 * All rights reserved.
 *
 * Additions by Rob Siklos <rob3@siklos.ca>
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

#if HAVE_CONFIG_H
#  include "config.h"
#endif
#include <io.h>

#include "common.h"
#define CLIPCHUNK 1024

#define ENDLMODE_UNIX   0
#define ENDLMODE_DOS    1
#define ENDLMODE_NOCONV 2

#define PUTCLIP_OK     0
#define PUTCLIP_ERR    1
#define PUTCLIP_ARGERR 2

static const char versionID[] = PACKAGE_VERSION;
static const char revID[] =
    "$Id$";
static const char copyrightID[] =
    "Copyright (c) 2001,...\nCharles S. Wilson. All rights reserved.\nLicensed under GPL v2.0\n";

typedef struct
{
  int endl_mode; /* ENDLMODE_DOS  ENDLMODE_UNIX  ENDLMODE_NOCONV      */
  int unixflag;  /* indicates that the user specified -u or --unix    */
  int dosflag;   /* indicates that the user specified -d or --dos     */
  int noconvflag;/* indicates that the user specified -n or --no-conv */
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
  int ec = PUTCLIP_OK;
  flags_struct flags = { ENDLMODE_NOCONV, FALSE, FALSE, FALSE };

  struct poptOption generalOptionsTable[] = {
    { "dos",  'd',  POPT_ARG_NONE, NULL, 'd', \
        "Clipboard text will have DOS line endings.", NULL},
    { "unix", 'u', POPT_ARG_NONE, NULL, 'U', \
        "Clipboard text will have UNIX line endings.", NULL},
    { "no-conv", 'n', POPT_ARG_NONE, NULL, 'n', \
        "Do not translate line endings.", NULL},
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
                 flags.endl_mode = ENDLMODE_DOS;
                 break;
      case 'U':  flags.unixflag = TRUE;
                 flags.endl_mode = ENDLMODE_UNIX;
                 break;
      case 'n':  flags.noconvflag = TRUE;
                 flags.endl_mode = ENDLMODE_NOCONV;
                 break;
    }
  }
  if (rc < -1 ) {
    fprintf(stderr, "%s: bad argument %s: %s\n",
      program_name, poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
      poptStrerror(rc));
    ec = PUTCLIP_ARGERR;
    goto exit;
  }
  if (flags.dosflag && flags.unixflag) {
    fprintf(stderr, "%s: can't specify both --unix and --dos (-u and -d)\n", program_name);
    ec = PUTCLIP_ARGERR;
    goto exit;
  }
  if (flags.dosflag && flags.noconvflag) {
    fprintf(stderr, "%s: can't specify both --dos  and --no-conv (-d and -n)\n", program_name);
    ec = PUTCLIP_ARGERR;
    goto exit;
  }              
  if (flags.unixflag && flags.noconvflag) {
    fprintf(stderr, "%s: can't specify both --unix  and --no-conv (-u and -n)\n", program_name);
    ec = PUTCLIP_ARGERR;
    goto exit;
  } 

  rest = poptGetArgs(optCon);

  if (rest == NULL)
    ec |= putclip(stdin, flags, stderr, program_name);
  else
  {
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
  fprintf(f, "%s is part of cygutils version %s\n", name, getVersion());
  fprintf(f, "  Copy stdin to the Windows Clipboard\n\n");
}

static void printBottomDescription(FILE * f, char * name)
{
  fprintf(f, "NOTE: by default, no line ending conversion is performed.\n");
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
  HANDLE hData;            /* handle to clip data */
  char* clipbuf = NULL;    /* pointer to clip data */
  char chunk[CLIPCHUNK+1];
  char *buf = NULL;
  char *convbuf = NULL;
  char *buf_old = NULL;
  char *ptr;
  int len = 0;
  int convlen = 0;
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
          return(PUTCLIP_ERR);
        }
        memcpy(buf_old, buf, len);
        free(buf);
      }
      if ((buf = malloc ((len+numread) * sizeof(char))) == NULL)
      {
        fprintf(stderr, "%s: memory allocation error\n", name);
        return(PUTCLIP_ERR);
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

  /* format the string according to flags */
  if (buf)
  {
    const char* CYGWIN_NATIVE = "CYGWIN_NATIVE_CLIPBOARD";
    UINT cygnativeformat;

    OpenClipboard(0);
    cygnativeformat = RegisterClipboardFormat (CYGWIN_NATIVE);
    CloseClipboard();

    // if flags.endl_mode == ENDLMODE_NOCONV 
    convbuf = buf;
    convlen = len;

    if (flags.endl_mode == ENDLMODE_UNIX)
    {
      /* remove all instances of '\r' */
      int i, cnt;
      char *newStr = NULL;
      char* prev;
      char* curr;
      char* pos;

      /* allocate memory for a new string */
      if ((newStr = (char*)malloc(convlen * sizeof(char))) == NULL) {
        fprintf(stderr, "%s: memory allocation error\n", name);
        return (PUTCLIP_ERR);
      }

      cnt = 0;      /* # of occurrences of \r\n */
      pos = newStr; /* the current position in the new string */
      prev = buf;
      curr = buf;
      for (i = 1; i < convlen; i++) {
        ++curr;
        if ((*prev == '\r') && *curr == '\n') {
          cnt++;
        } else {
          *pos++ = *prev;
        }
        prev = curr;
      }
      /* always copy the last char -- it can't be the '\r' of a '\r\n' pair */
      *pos++ = *prev;
      convbuf = newStr;
      convlen -= cnt;
    }

    if (flags.endl_mode == ENDLMODE_DOS)
    {
      /* change all instances of [!\r]\n to \r\n */
      int i, cnt;
      char *newStr = NULL;
      char* prev;
      char* curr;
      char* pos;

      /* count the instances [!\r]\n */
      cnt = 0;
      prev = buf;
      curr = buf;
      if (buf[0] == '\n') { cnt++; }
      for (i = 1; i < convlen; i++) {
        ++curr;
        if ((*curr == '\n') && (*prev != '\r')) {
          cnt++;
        }
        prev = curr;
      }

      /* allocate memory for a new string */
      if ((newStr = (char*)malloc((convlen+cnt+1)* sizeof(char))) == NULL) {
        fprintf(stderr,"%s: memory allocation error\n", name);
        return (PUTCLIP_ERR);
      }

      /* do the search and replace */
      pos = newStr;
      prev = buf;
      curr = buf;
      if (buf[0] == '\n') {
        *pos++ = '\r';
        *pos++ = '\n';
      } else {
        *pos++ = buf[0];
      }
      for (i = 1; i < convlen; i++) {
        ++curr;
        if ((*curr == '\n') && (*prev != '\r')) {
          *pos++ = '\r';
          *pos++ = '\n';
        } else {
          *pos++ = *curr;
        }
        prev = curr;
      }
      convbuf = newStr;
      convlen += cnt;
    }

    if (buf != convbuf) free(buf);
    /* Now, the only buffer left to be freed is convbuf */

    /* Allocate memory and copy the string to it */
    /* cygwin native format */
    OpenClipboard(0);
    if (!(hData = GlobalAlloc(GMEM_MOVEABLE, convlen + sizeof (size_t))))
    {
      fprintf(f, "Couldn't allocate global buffer for write.\n");
      if (convbuf) free(convbuf);
      return (PUTCLIP_ERR);
    }
    if (!(clipbuf = (char*) GlobalLock(hData)))
    {
      fprintf(f, "Couldn't lock global buffer.\n");
      free(convbuf);
      return (PUTCLIP_ERR);
    }

    *(size_t *) (clipbuf) = convlen;
    memcpy(clipbuf + sizeof (size_t), convbuf, convlen);
    GlobalUnlock (hData);
    EmptyClipboard();
    if (! SetClipboardData (cygnativeformat, hData))
    {
      fprintf(f, "Couldn't write native format to the clipboard %04x %x\n",
              cygnativeformat, hData);
      free(convbuf);
      return (PUTCLIP_ERR);
    }
    CloseClipboard();
    if (GlobalFree(hData))
    {
      fprintf(f, "Couldn't free global buffer after write to clipboard.\n");
      free(convbuf);
      return (PUTCLIP_ERR);
    }
    hData = NULL;
    clipbuf = NULL;

    /* CF_TEXT format */
    OpenClipboard(0);
    if (!(hData = GlobalAlloc(GMEM_MOVEABLE, convlen + 2)))
    {
      fprintf(f, "Couldn't allocate global buffer for write.\n");
      free(convbuf);
      return (PUTCLIP_ERR);
    }
    if (!(clipbuf = (char*) GlobalLock(hData)))
    {
      fprintf(f, "Couldn't lock global buffer.\n");
      free(convbuf);
      return (PUTCLIP_ERR);
    }

    memcpy(clipbuf, convbuf, convlen);
    *(clipbuf + convlen) = '\0';
    *(clipbuf + convlen + 1) = '\0';

    GlobalUnlock (hData);
    if (! SetClipboardData (CF_TEXT, hData))
    {
      fprintf(f, "Couldn't write CF_TEXT format to the clipboard.\n");
      free(convbuf);
      return (PUTCLIP_ERR);
    }
    CloseClipboard();
    if (GlobalFree(hData))
    {
      fprintf(f, "Couldn't free global buffer after write to clipboard.\n");
      free(convbuf);
      return (PUTCLIP_ERR);
    }
    hData = NULL;
    clipbuf = NULL;

    free(convbuf);
  }
  else /* if (buf) */
  {
    OpenClipboard(0);
    EmptyClipboard();
    CloseClipboard();
  }
  return (PUTCLIP_OK);
}
