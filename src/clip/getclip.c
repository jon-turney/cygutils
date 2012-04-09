/**
 * getclip.exe   Copy Windows clipboard to stdout  
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
#include <sys/cygwin.h>

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

#define ENDLMODE_UNIX 0
#define ENDLMODE_DOS 1
#define ENDLMODE_NOCONV 2

static const char versionID[] = PACKAGE_VERSION;
static const char revID[] =
  "$Id$";
static const char copyrightID[] =
  "Copyright (c) 2009\nCharles S. Wilson. All rights reserved.\nLicensed under GPLv3+\n";

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
static int getclip (FILE * out, flags_struct flags, FILE * f, char *name);

static char *program_name;
static poptContext optCon;

int
main (int argc, const char **argv)
{
  const char **rest;
  int rc;
  int ec = 0;
  flags_struct flags = { ENDLMODE_NOCONV, FALSE, FALSE, FALSE };

  struct poptOption generalOptionsTable[] = {
    {"dos", 'd', POPT_ARG_NONE, NULL, 'd',
     "Output text will have DOS line endings.", NULL},
    {"unix", 'u', POPT_ARG_NONE, NULL, 'U',
     "Output text will have UNIX line endings.", NULL},
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
      ec = 2;
      goto exit;
    }
  if (flags.dosflag && flags.unixflag)
    {
      fprintf (stderr,
               "%s: can't specify both --unix and --dos (-u and -d)\n",
               program_name);
      ec = 2;
      goto exit;
    }
  if (flags.dosflag && flags.noconvflag)
    {
      fprintf (stderr,
               "%s: can't specify both --dos  and --no-conv (-d and -n)\n",
               program_name);
      ec = 2;
      goto exit;
    }
  if (flags.unixflag && flags.noconvflag)
    {
      fprintf (stderr,
               "%s: can't specify both --unix  and --no-conv (-u and -n)\n",
               program_name);
      ec = 2;
      goto exit;
    }
  if (flags.unixflag && (sizeof (char) != 1))
    {
      fprintf (stderr,
               "%s: the --unix flag will not work on wide-character systems\n",
               program_name);
      ec = 2;
      goto exit;
    }
  rest = poptGetArgs (optCon);
  if (rest == NULL)
    ec |= getclip (stdout, flags, stderr, program_name);
  else
    {
      fprintf (stderr, "Extra args ignored: ");
      while (*rest)
        fprintf (stderr, "%s ", *rest++);
      fprintf (stderr, "\n");
      ec |= getclip (stdout, flags, stderr, program_name);
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
  fprintf (f, "  Copy the Windows Clipboard to stdout\n\n");
}

static void
printBottomDescription (FILE * f, char *name)
{
  fprintf (f,
           "\nNOTE: by default, no line ending conversion is performed.\n");
}

static
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
getclip (FILE * out, flags_struct flags, FILE * f, char *name)
{
  HGLOBAL hglb;
  char *buf = NULL;
  size_t len = 0;
  char *newStr = NULL;
  int origMode;
  int cygNewFormat = 0;

  cygwin_version_data verInfo;
  getCygVer(&verInfo);
  if ((verInfo.dll_major > 1) ||
      ((verInfo.dll_major == 1) && (verInfo.dll_minor > 7)) ||
      ((verInfo.dll_major == 1) && (verInfo.dll_minor == 7) &&
       (verInfo.dll_micro >= 13)))
    cygNewFormat = 1;

  origMode = setmode (fileno (out), O_BINARY);

  if (flags.endl_mode == ENDLMODE_NOCONV)
    {
      const char *CYGWIN_NATIVE = "CYGWIN_NATIVE_CLIPBOARD";
      UINT cygnativeformat;
      UINT formatlist[2];
      UINT format;

      OpenClipboard (NULL);
      cygnativeformat = RegisterClipboardFormat (CYGWIN_NATIVE);

      formatlist[0] = cygnativeformat;
      formatlist[1] = CF_TEXT;

      if ((format = GetPriorityClipboardFormat (formatlist, 2)) <= 0)
        {
          CloseClipboard ();
          return (0);
        }

      hglb = GetClipboardData (format);

      if (format == cygnativeformat)
        {
	  if (cygNewFormat)
	    {
              cygcb_t *clipbuf = (cygcb_t *) GlobalLock (hglb);
              if (clipbuf)
                {
                  fwrite (clipbuf->data, sizeof (unsigned char), clipbuf->len, out);
                  GlobalUnlock (hglb);
                }
	    }
	  else
	    {
              unsigned char *nativebuf = (unsigned char *) GlobalLock (hglb);
              if (nativebuf)
                {
                  size_t buflen = (*(size_t *) nativebuf);
                  buf = nativebuf + sizeof (size_t);
                  len = buflen;

                  fwrite (buf, sizeof (unsigned char), len, out);
                  GlobalUnlock (hglb);
                }
	    }
        }
      else
        {
          LPSTR lpstr = (LPSTR) GlobalLock (hglb);
          if (lpstr)
            {
              size_t lplen = strlen (lpstr);
              buf = lpstr;
              len = lplen;

              fwrite (buf, sizeof (unsigned char), len, out);
              GlobalUnlock (hglb);
            }
        }
      CloseClipboard ();
    }

  if (flags.endl_mode == ENDLMODE_UNIX)
    {
      /* remove the '\r' of any '\r\n' pair */
      int i, cnt;
      LPSTR lpstr;
      int lplen;
      char *prev;
      char *curr;
      char *pos;

      OpenClipboard (0);
      hglb = GetClipboardData (CF_TEXT);
      lpstr = GlobalLock (hglb);
      lplen = strlen (lpstr);

      buf = lpstr;
      len = lplen;

      /* get some memory for the new string */
      if ((newStr = (char *) malloc ((len + 1) * sizeof (char))) == NULL)
        {
          fprintf (f, "%s: memory allocation error\n", name);
          return 1;
        }

      cnt = 0;                  /* how many '\r' have been dropped */
      pos = newStr;             /* the current position in the new string */
      prev = buf;
      curr = buf;
      for (i = 1; i < len; i++)
        {
          ++curr;
          if ((*prev == '\r') && (*curr == '\n'))
            {
              ++cnt;
            }
          else
            {
              *pos++ = *prev;
            }
          prev = curr;
        }
      /* always copy the last char -- it can't be the '\r' of a '\r\n' pair */
      *pos++ = *prev;
      *pos = '\0';
      len -= cnt;

      /* not len+1 because we don't want to write the final '\0' */
      fwrite (newStr, sizeof (char), len, out);
      free (newStr);

      GlobalUnlock (hglb);
      CloseClipboard ();
    }

  if (flags.endl_mode == ENDLMODE_DOS)
    {
      /* change all instances of [!\r]\n to \r\n */
      int i, cnt;
      LPSTR lpstr;
      int lplen;
      char *newStr = NULL;
      char *prev;
      char *curr;
      char *pos;

      OpenClipboard (0);
      hglb = GetClipboardData (CF_TEXT);
      lpstr = GlobalLock (hglb);
      lplen = strlen (lpstr);

      buf = lpstr;
      len = lplen;

      /* count the instances [!\r]\n */
      cnt = 0;
      prev = buf;
      curr = buf;
      if (buf[0] == '\n')
        {
          cnt++;
        }
      for (i = 1; i < len; i++)
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
           (char *) malloc ((len + cnt + 1) * sizeof (char))) == NULL)
        {
          fprintf (stderr, "%s: memory allocation error\n", name);
          return 1;
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
      for (i = 1; i < len; i++)
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
      len += cnt;
      *pos = '\0';

      /* not len+1 because we don't want to write the final '\0' */
      fwrite (newStr, sizeof (char), len, out);
      free (newStr);

      GlobalUnlock (hglb);
      CloseClipboard ();
    }

  /* reset the mode of the output channel (in case it's stdout or stderr) */
  setmode (fileno (out), origMode);
  return (0);
}
