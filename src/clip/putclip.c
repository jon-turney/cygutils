/**
 * putclip.exe   copy stdin to Windows clipboard
 *
 * Copyright 2001,2002,2009,2012 by Charles Wilson
 * All rights reserved.
 *
 * Additions by Rob Siklos <rob3@siklos.ca>
 *
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

#include <io.h>
#include <wchar.h>
#include <sys/cygwin.h>
#include <w32api/stringapiset.h>

#define DEBUGGING 0 // Set 1 to display debug output on stderr

typedef struct
{
  struct timespec timestamp;
  size_t          len;
  char            data[1];
} cygcb_t;

typedef struct
{
  unsigned short api_major;
  unsigned short api_minor;
  unsigned short dll_major;
  unsigned short dll_minor;
  unsigned short dll_micro;
} cygwin_version_data;

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
  "Copyright (c) 2001,2002,2009\nCharles S. Wilson. All rights reserved.\nLicensed under GPLv3+\n";

typedef struct
{
  int endl_mode;                /* ENDLMODE_DOS  ENDLMODE_UNIX  ENDLMODE_NOCONV      */
  int unixflag;                 /* indicates that the user specified -u or --unix    */
  int dosflag;                  /* indicates that the user specified -d or --dos     */
  int noconvflag;               /* indicates that the user specified -n or --no-conv */
} flags_struct;

static void printTopDescription (FILE * f, char *name);
static void printBottomDescription (FILE * f, char *name);
static const char *getVersion (void);
static void usage (FILE * f, char *name);
static void help (FILE * f, char *name);
static void version (FILE * f, char *name);
static void license (FILE * f, char *name);
static void getCygVer(cygwin_version_data* verInfo);
static int putclip (FILE * in, flags_struct flags, FILE * f, char *name);

static char *program_name;
static poptContext optCon;

int
main (int argc, const char **argv)
{
  const char **rest;
  int rc;
  int ec = PUTCLIP_OK;
  flags_struct flags = { ENDLMODE_NOCONV, FALSE, FALSE, FALSE };

  struct poptOption generalOptionsTable[] = {
    {"dos", 'd', POPT_ARG_NONE, NULL, 'd',
     "Clipboard text will have DOS line endings.", NULL},
    {"unix", 'u', POPT_ARG_NONE, NULL, 'U',
     "Clipboard text will have UNIX line endings.", NULL},
    {"no-conv", 'n', POPT_ARG_NONE, NULL, 'n',
     "Do not translate line endings.", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

  struct poptOption helpOptionsTable[] = {
    {"help", '?', POPT_ARG_NONE, NULL, '?',
     "Show this help message", NULL},
    {"usage", '\0', POPT_ARG_NONE, NULL, 'u',
     "Display brief usage message", NULL},
    {"version", '\0', POPT_ARG_NONE, NULL, 'v',
     "Display version information", NULL},
    {"license", '\0', POPT_ARG_NONE, NULL, 'l',
     "Display licensing information", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

  struct poptOption opt[] = {
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, generalOptionsTable, 0,
     "General options", NULL},
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0,
     "Help options", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

  if ((program_name = strdup (argv[0])) == NULL)
    {
      fprintf (stderr, "%s: memory allocation error\n", argv[0]);
      exit (1);
    }
  optCon = poptGetContext (NULL, argc, argv, opt, 0);

  while ((rc = poptGetNextOpt (optCon)) > 0)
    {
      switch (rc)
        {
        case '?':
          help (stdout, program_name);
          goto exit;
        case 'u':
          usage (stdout, program_name);
          goto exit;
        case 'v':
          version (stdout, program_name);
          goto exit;
        case 'l':
          license (stdout, program_name);
          goto exit;
        case 'd':
          flags.dosflag = TRUE;
          flags.endl_mode = ENDLMODE_DOS;
          break;
        case 'U':
          flags.unixflag = TRUE;
          flags.endl_mode = ENDLMODE_UNIX;
          break;
        case 'n':
          flags.noconvflag = TRUE;
          flags.endl_mode = ENDLMODE_NOCONV;
          break;
        }
    }
  if (rc < -1)
    {
      fprintf (stderr, "%s: bad argument %s: %s\n",
               program_name, poptBadOption (optCon, POPT_BADOPTION_NOALIAS),
               poptStrerror (rc));
      ec = PUTCLIP_ARGERR;
      goto exit;
    }
  if (flags.dosflag && flags.unixflag)
    {
      fprintf (stderr,
               "%s: can't specify both --unix and --dos (-u and -d)\n",
               program_name);
      ec = PUTCLIP_ARGERR;
      goto exit;
    }
  if (flags.dosflag && flags.noconvflag)
    {
      fprintf (stderr,
               "%s: can't specify both --dos  and --no-conv (-d and -n)\n",
               program_name);
      ec = PUTCLIP_ARGERR;
      goto exit;
    }
  if (flags.unixflag && flags.noconvflag)
    {
      fprintf (stderr,
               "%s: can't specify both --unix  and --no-conv (-u and -n)\n",
               program_name);
      ec = PUTCLIP_ARGERR;
      goto exit;
    }

  rest = poptGetArgs (optCon);

  if (rest == NULL)
    ec |= putclip (stdin, flags, stderr, program_name);
  else
    {
      fprintf (stderr, "Extra args ignored: ");
      while (*rest)
        fprintf (stderr, "%s ", *rest++);
      fprintf (stderr, "\n");
      ec |= putclip (stdin, flags, stderr, program_name);
    }

exit:
  poptFreeContext (optCon);
  free (program_name);
  return (ec);
}

static const char *
getVersion ()
{
  return versionID;
}

static void
printTopDescription (FILE * f, char *name)
{
  fprintf (f, "%s is part of cygutils version %s\n", name, getVersion ());
  fprintf (f, "  Copy stdin to the Windows Clipboard\n\n");
}

static void
printBottomDescription (FILE * f, char *name)
{
  fprintf (f, "NOTE: by default, no line ending conversion is performed.\n");
}

static void
printLicense (FILE * f, char *name)
{
  fprintf (f,
           "This program is free software: you can redistribute it and/or modify\n"
           "it under the terms of the GNU General Public License as published by\n"
           "the Free Software Foundation, either version 3 of the License, or\n"
           "(at your option) any later version.\n\n"
           "This program is distributed in the hope that it will be useful,\n"
           "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
           "GNU General Public License for more details.\n\n"
           "You should have received a copy of the GNU General Public License\n"
           "along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n"
           "See the COPYING file for full license information.\n");
}

static void
usage (FILE * f, char *name)
{
  poptPrintUsage (optCon, f, 0);
}

static void
help (FILE * f, char *name)
{
  printTopDescription (f, name);
  poptPrintHelp (optCon, f, 0);
  printBottomDescription (f, name);
}

static void
version (FILE * f, char *name)
{
  printTopDescription (f, name);
}

static void
license (FILE * f, char *name)
{
  printTopDescription (f, name);
  printLicense (f, name);
}

#define CYGPREFIX (sizeof ("%%% Cygwin ") - 1)
static void getCygVer(cygwin_version_data* verInfo)
{
  /* adapted from cygcheck.cc */
  static char dummy[] = "\0\0\0\0\0\0\0";
  char *dll_major = dummy;

  const char* buf = (const char*) cygwin_internal(CW_GETVERSIONINFO);
  size_t size = strlen(buf);
  const char *bufend = buf + size;

  verInfo->api_major = 0;
  verInfo->api_minor = 0;
  verInfo->dll_major = 0;
  verInfo->dll_minor = 0;
  verInfo->dll_micro = 0;

  while (buf < bufend)
    if ((buf = (char *) memchr (buf, '%', bufend - buf)) == NULL)
      break;
    else if (strncmp ("%%% Cygwin ", buf, CYGPREFIX) != 0)
      buf++;
    else
      {
        char *p = strchr (buf += CYGPREFIX, '\n');
        if (!p)
          break;
        if (strncasecmp (buf, "dll major:", 10) == 0)
          {
            char c = '\0';
            /* assumes 'p-(buf+11)' <= 7 */
            memcpy (dll_major, buf + 11, p - (buf + 11));
            c = dll_major[1];
            dll_major[1] = '\0';
            verInfo->dll_major = atoi (dll_major);
            dll_major[1] = c;
            verInfo->dll_minor = atoi (dll_major + 1);
            continue;
          }
        if (strncasecmp (buf, "dll minor:", 10) == 0)
          {
            verInfo->dll_micro = atoi (buf + 11);
            continue;
          }
        if (strncasecmp (buf, "api major:", 10) == 0)
          {
            verInfo->api_major = atoi (buf + 11);
            continue;
          }
        if (strncasecmp (buf, "api minor:", 10) == 0)
          {
            verInfo->api_minor = atoi (buf + 11);
            continue;
          }
      }
}

int
putclip (FILE * in, flags_struct flags, FILE * f, char *name)
{
  HANDLE hData;                 /* handle to clip data */
  void *clipbuf = NULL;         /* pointer to clip data */
  char chunk[CLIPCHUNK + 1];
  char *buf = NULL;
  char *convbuf = NULL;
  char *buf_old = NULL;
  char *ptr;
  int len = 0;
  int convlen = 0;
  int numread;
  int cygNewFormat = 0;

  cygwin_version_data verInfo;
  getCygVer(&verInfo);
  if ((verInfo.dll_major > 1) ||
      ((verInfo.dll_major == 1) && (verInfo.dll_minor > 7)) ||
      ((verInfo.dll_major == 1) && (verInfo.dll_minor == 7) &&
       (verInfo.dll_micro >= 13)))
    cygNewFormat = 1;

  /* Since we have to copy to the clipboard all in one step,
     read entire stream into memory. However, let's try to 
     do this using the minimum amount of memory, but keep 
     allocating more if needed.
   */
  do
    {
      numread = fread (chunk, sizeof (char), CLIPCHUNK, in);
      if (numread > 0)
        {
          if (buf)
            {
              if ((buf_old = malloc (len * sizeof (char))) == NULL)
                {
                  fprintf (stderr, "%s: memory allocation error\n", name);
                  return (PUTCLIP_ERR);
                }
              memcpy (buf_old, buf, len);
              free (buf);
            }
          if ((buf = malloc ((len + numread) * sizeof (char))) == NULL)
            {
              fprintf (stderr, "%s: memory allocation error\n", name);
              return (PUTCLIP_ERR);
            }
          if (buf_old)
            {
              memcpy (buf, buf_old, len);
              free (buf_old);
            }
          ptr = &(buf[len]);
          memcpy (ptr, chunk, numread);
          len += numread;
        }
    }
  while (!feof (in) && !ferror (in));

  /* format the string according to flags */
  if (buf)
    {
      const char *CYGWIN_NATIVE = "CYGWIN_NATIVE_CLIPBOARD";
      UINT cygnativeformat;

      if (!OpenClipboard (0))
        {
          fprintf (stderr, "Unable to open the clipboard\n");
#if DEBUGGING
          DWORD err = GetLastError ();
          /* look up error code displayed here in w32api/winerror.h */
          fprintf (stderr, "OpenClipboard returns %ld\n", err);
#endif
          return (PUTCLIP_ERR);
        }
      cygnativeformat = RegisterClipboardFormat (CYGWIN_NATIVE);
      CloseClipboard ();

      // if flags.endl_mode == ENDLMODE_NOCONV 
      convbuf = buf;
      convlen = len;

      if (flags.endl_mode == ENDLMODE_UNIX)
        {
          /* remove all instances of '\r' */
          int i, cnt;
          char *newStr = NULL;
          char *prev;
          char *curr;
          char *pos;

          /* allocate memory for a new string */
          if ((newStr = (char *) malloc (convlen * sizeof (char))) == NULL)
            {
              fprintf (stderr, "%s: memory allocation error\n", name);
              return (PUTCLIP_ERR);
            }

          cnt = 0;              /* # of occurrences of \r\n */
          pos = newStr;         /* the current position in the new string */
          prev = buf;
          curr = buf;
          for (i = 1; i < convlen; i++)
            {
              ++curr;
              if ((*prev == '\r') && *curr == '\n')
                {
                  cnt++;
                }
              else
                {
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
          char *prev;
          char *curr;
          char *pos;

          /* count the instances [!\r]\n */
          cnt = 0;
          prev = buf;
          curr = buf;
          if (buf[0] == '\n')
            {
              cnt++;
            }
          for (i = 1; i < convlen; i++)
            {
              ++curr;
              if ((*curr == '\n') && (*prev != '\r'))
                {
                  cnt++;
                }
              prev = curr;
            }

          /* allocate memory for a new string */
          if ((newStr =
               (char *) malloc ((convlen + cnt + 1) * sizeof (char))) == NULL)
            {
              fprintf (stderr, "%s: memory allocation error\n", name);
              return (PUTCLIP_ERR);
            }

          /* do the search and replace */
          pos = newStr;
          prev = buf;
          curr = buf;
          if (buf[0] == '\n')
            {
              *pos++ = '\r';
              *pos++ = '\n';
            }
          else
            {
              *pos++ = buf[0];
            }
          for (i = 1; i < convlen; i++)
            {
              ++curr;
              if ((*curr == '\n') && (*prev != '\r'))
                {
                  *pos++ = '\r';
                  *pos++ = '\n';
                }
              else
                {
                  *pos++ = *curr;
                }
              prev = curr;
            }
          convbuf = newStr;
          convlen += cnt;
        }

      if (buf != convbuf)
        free (buf);
      /* Now, the only buffer left to be freed is convbuf */

      /* Allocate memory and copy the string to it */
      /* cygwin native format */
      if (!OpenClipboard (0))
        {
          fprintf (stderr, "Unable to open the clipboard\n");
#if DEBUGGING
          DWORD err = GetLastError ();
          /* look up error code displayed here in w32api/winerror.h */
          fprintf (stderr, "OpenClipboard returns %ld\n", err);
#endif
          if (convbuf)
            free (convbuf);
          return (PUTCLIP_ERR);
        }
      if (cygNewFormat)
	hData = GlobalAlloc (GMEM_MOVEABLE, convlen + sizeof (cygcb_t));
      else
	hData = GlobalAlloc (GMEM_MOVEABLE, convlen + sizeof (size_t));

      if (!hData)
        {
          fprintf (f, "Couldn't allocate global buffer for write.\n");
          if (convbuf)
            free (convbuf);
          return (PUTCLIP_ERR);
        }
      if (!(clipbuf = (void *) GlobalLock (hData)))
        {
          fprintf (f, "Couldn't lock global buffer.\n");
          free (convbuf);
          return (PUTCLIP_ERR);
        }

      if (cygNewFormat)
        {
          cygcb_t *clipbufX = (cygcb_t *) clipbuf;
          clipbufX->len = convlen;
          clock_gettime (CLOCK_REALTIME, &clipbufX->timestamp);
          memcpy (clipbufX->data, convbuf, convlen);
        }
      else
        {
          *(size_t *) (clipbuf) = convlen;
          memcpy (clipbuf + sizeof (size_t), convbuf, convlen);
        }
      GlobalUnlock (hData);
      EmptyClipboard ();
      if (!SetClipboardData (cygnativeformat, hData))
        {
          fprintf (f,
                   "Couldn't write native format to the clipboard %04x %x\n",
                   cygnativeformat, hData);
#if DEBUGGING
          DWORD err = GetLastError ();
          /* look up error code displayed here in w32api/winerror.h */
          fprintf (stderr, "SetClipboardData returns %ld\n", err);
#endif
          free (convbuf);
          return (PUTCLIP_ERR);
        }
      CloseClipboard ();
#if 0 // Per MSDN, don't GlobalFree a handle successfully transferred to system
      if (GlobalFree (hData))
        {
          fprintf (f,
                   "Couldn't free global buffer after write to clipboard.\n");
          free (convbuf);
          return (PUTCLIP_ERR);
        }
#endif
      hData = NULL;
      clipbuf = NULL;

      /* CF_UNICODETEXT format */
      if (!OpenClipboard (0))
        {
          fprintf (f, "Unable to open the clipboard\n");
#if DEBUGGING
          DWORD err = GetLastError ();
          /* look up error code displayed here in w32api/winerror.h */
          fprintf (stderr, "OpenClipboard returns %ld\n", err);
#endif
          free (convbuf);
          return (PUTCLIP_ERR);
        }
      if (!(hData = GlobalAlloc (GMEM_MOVEABLE, sizeof (WCHAR) * convlen + 2)))
        {
          fprintf (f, "Couldn't allocate global buffer for write.\n");
          free (convbuf);
          return (PUTCLIP_ERR);
        }
      if (!(clipbuf = (void *) GlobalLock (hData)))
        {
          fprintf (f, "Couldn't lock global buffer.\n");
          free (convbuf);
          return (PUTCLIP_ERR);
        }

      int res = MultiByteToWideChar (CP_UTF8, 0, convbuf, convlen,
                                     (LPWSTR) clipbuf, convlen + 1);
      if (res == 0)
        {
#if DEBUGGING
          DWORD err = GetLastError ();
          /* look up error code displayed here in w32api/winerror.h */
          fprintf (stderr, "MultiByteToWideChar returns %ld\n", err);
#endif
        }
      else /* res != 0 */
        {
          *((WCHAR *) clipbuf + res) = 0; /* NULL-terminate just in case */
#if DEBUGGING
          fprintf (stderr, "len %d, convlen %d, res %d\n", len, convlen, res);
#endif
        }

      GlobalUnlock (hData);
      if (!SetClipboardData (CF_UNICODETEXT, hData))
        {
          fprintf (f, "Couldn't write CF_UNICODETEXT format to the clipboard.\n");
#if DEBUGGING
          DWORD err = GetLastError ();
          /* look up error code displayed here in w32api/winerror.h */
          fprintf (stderr, "SetClipboardData returns %ld\n", err);
#endif
          free (convbuf);
          return (PUTCLIP_ERR);
        }
      CloseClipboard ();
#if 0 // Per MSDN, don't GlobalFree a handle successfully transferred to system
      if (GlobalFree (hData))
        {
          fprintf (f,
                   "Couldn't free global buffer after write to clipboard.\n");
          free (convbuf);
          return (PUTCLIP_ERR);
        }
#endif
      hData = NULL;
      clipbuf = NULL;

      /* CF_TEXT format */
      if (!OpenClipboard (0))
        {
          fprintf (f, "Unable to open the clipboard\n");
#if DEBUGGING
          DWORD err = GetLastError ();
          /* look up error code displayed here in w32api/winerror.h */
          fprintf (stderr, "OpenClipboard returns %ld\n", err);
#endif
          free (convbuf);
          return (PUTCLIP_ERR);
        }
      if (!(hData = GlobalAlloc (GMEM_MOVEABLE, convlen + 2)))
        {
          fprintf (f, "Couldn't allocate global buffer for write.\n");
          free (convbuf);
          return (PUTCLIP_ERR);
        }
      if (!(clipbuf = (void *) GlobalLock (hData)))
        {
          fprintf (f, "Couldn't lock global buffer.\n");
          free (convbuf);
          return (PUTCLIP_ERR);
        }

      memcpy (clipbuf, convbuf, convlen);
      *((char *)clipbuf + convlen) = '\0';
      *((char *)clipbuf + convlen + 1) = '\0';

      GlobalUnlock (hData);
      if (!SetClipboardData (CF_TEXT, hData))
        {
          fprintf (f, "Couldn't write CF_TEXT format to the clipboard.\n");
#if DEBUGGING
          DWORD err = GetLastError ();
          /* look up error code displayed here in w32api/winerror.h */
          fprintf (stderr, "SetClipboardData returns %ld\n", err);
#endif
          free (convbuf);
          return (PUTCLIP_ERR);
        }
      CloseClipboard ();
#if 0 // Per MSDN, don't GlobalFree a handle successfully transferred to system
      if (GlobalFree (hData))
        {
          fprintf (f,
                   "Couldn't free global buffer after write to clipboard.\n");
          free (convbuf);
          return (PUTCLIP_ERR);
        }
#endif
      hData = NULL;
      clipbuf = NULL;

      free (convbuf);
    }
  else                          /* if (buf) */
    {
      OpenClipboard (0);
      EmptyClipboard ();
      CloseClipboard ();
    }
  return (PUTCLIP_OK);
}
