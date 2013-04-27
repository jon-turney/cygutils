/* 
 * readshortcut for cygwin/windows
 *
 * Copyright (C) 2003 Rob Siklos
 * http://www.cygwin.com/ml/cygwin/2003-08/msg00640.html
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



 /* http://www.rpm.org/dark_corners/popt/
  * http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/programmersguide/shell_int/shell_int_programming/shortcuts/shortcut.asp
  */

/* how to compile a standalone version:
 *
 * - comment the config.h and common.h includes
 * - uncomment the stdio and popt.h includes
 * - run "gcc readshortcut.c -o readshortcut.exe -lpopt -luuid -lole32" 
 *
 */


#if HAVE_CONFIG_H
# include "config.h"
#endif
#include "common.h"

/* moved to common.h */
/*
#include <stdio.h>
#include <popt.h>
*/

#include <shlobj.h>
#include <olectl.h>
#include <sys/cygwin.h>

#define PATH_UNIX 0
#define PATH_WIN  1

#define ERR_NONE 0
#define ERR_USER 1
#define ERR_SYS  2
#define ERR_WIN  3

#define SW_SHOWMINIMIZED_SEVEN 7

#define BUFF_SIZE 1024

static const char versionID[] = PACKAGE_VERSION;

typedef struct optvals_s
{
  char *target_fname;

  int show_field_names;
  int pathType;

  int show_target;
  int show_working_dir;
  int show_args;
  int show_showCmd;
  int show_icon;
  int show_icon_offset;
  int show_desc;
  int show_all;
  int show_raw;

} optvals;

static void printTopDescription (FILE * f, char *name);
static void printBottomDescription (FILE * f, char *name);
static const char *getVersion (void);
static void usage (FILE * f, char *name);
static void help (FILE * f, char *name);
static void version (FILE * f, char *name);
static void license (FILE * f, char *name);

char * formatPath (char *strPath, int format);
int readshortcut (optvals * opts);

static char *program_name;
static poptContext optCon;

int
main (int argc, const char **argv)
{
  const char **loose_args;
  int rc;
  int result = ERR_NONE;
  optvals opts;
  const char *tmp_str;

  struct poptOption infoOptionsTable[] = {
    {"help", 'h', POPT_ARG_NONE, NULL, '?', "This message", NULL},
    {"usage", '\0', POPT_ARG_NONE, NULL, 'u', "Program usage", NULL},
    {"version", 'v', POPT_ARG_NONE, NULL, 'v', "Version information", NULL},
    {"license", '\0', POPT_ARG_NONE, NULL, 'l', "License information", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

  struct poptOption outputOptionsTable[] = {
    {"fields", 'f', POPT_ARG_VAL, &(opts.show_field_names), 1,
     "Show field names", NULL},
    {"unix", 'u', POPT_ARG_VAL, &(opts.pathType), PATH_UNIX,
     "Use Unix path format for display (default)", NULL},
    {"windows", 'w', POPT_ARG_VAL, &(opts.pathType), PATH_WIN,
     "Use Windows path format for display ", NULL},
    {"target", 't', POPT_ARG_VAL, &(opts.show_target), 1,
     "Display shortcut target", NULL},
    {"working", 'g', POPT_ARG_VAL, &(opts.show_working_dir), 1,
     "Display shortcut working directory", NULL},
    {"args", 'r', POPT_ARG_VAL, &(opts.show_args), 1,
     "Display shortcut arguments", NULL},
    {"showcmd", 's', POPT_ARG_VAL, &(opts.show_showCmd), 1,
     "Display shortcut \"show\" command value", NULL},
    {"icon", 'i', POPT_ARG_VAL, &(opts.show_icon), 1,
     "Display icon library location", NULL},
    {"offset", 'j', POPT_ARG_VAL, &(opts.show_icon_offset), 1,
     "Display icon library offset", NULL},
    {"desc", 'd', POPT_ARG_VAL, &(opts.show_desc), 1,
     "Display shortcut description", NULL},
    {"all", 'a', POPT_ARG_VAL, &(opts.show_all), 1, "Display all information",
     NULL},
    {"raw", 'R', POPT_ARG_VAL, &(opts.show_raw), 1,
     "Show paths in raw mode (that is, without expanding embedded environment variables)", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

  struct poptOption opt[] = {
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, outputOptionsTable, 0,
     "Output options", NULL},
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, infoOptionsTable, 0,
     "Information options (display a message and exit)", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

  /* get the program name */
  tmp_str = strrchr (argv[0], '/');
  if (tmp_str == NULL)
    {
      tmp_str = strrchr (argv[0], '\\');
    }
  if (tmp_str == NULL)
    {
      tmp_str = argv[0];
    }
  else
    {
      tmp_str++;
    }
  if ((program_name = strdup (tmp_str)) == NULL)
    {
      fprintf (stderr, "%s: memory allocation error\n", argv[0]);
      exit (ERR_SYS);
    }

  /* set default values for options */
  opts.target_fname = NULL;

  opts.show_field_names = 0;
  opts.pathType = PATH_UNIX;

  opts.show_target = 0;
  opts.show_working_dir = 0;
  opts.show_args = 0;
  opts.show_showCmd = 0;
  opts.show_icon = 0;
  opts.show_icon_offset = 0;
  opts.show_desc = 0;
  opts.show_all = 0;
  opts.show_raw = 0;

  /* set the pOpt context and help line */
  optCon = poptGetContext (NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp (optCon, "[OPTION]* SHORTCUT");

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
        }
    }

  // set show_target by default
  if (!
      (opts.show_all + opts.show_target + opts.show_working_dir +
       opts.show_args + opts.show_showCmd + opts.show_icon +
       opts.show_icon_offset + opts.show_desc))
    {
      opts.show_target = 1;
    }

  /* get the remaining arguments */
  loose_args = poptGetArgs (optCon);

  if (loose_args && *loose_args)
    {
      if ((opts.target_fname = strdup (*loose_args)) == NULL)
        {
          fprintf (stderr, "%s: memory allocation error\n", program_name);
          result = ERR_SYS;
          goto exit;
        }
      loose_args++;
      if (loose_args && *loose_args)
        {
          fprintf (stderr, "%s: Too many arguments: ", program_name);
          while (*loose_args)
            {
              fprintf (stderr, "%s ", *loose_args++);
            }
          fprintf (stderr, "\n");
          usage (stderr, program_name);
          result = ERR_USER;
        }
      else
        {
      /************** Main Program ***********/
          result = readshortcut (&opts);
        }
    }
  else
    {
      fprintf (stderr, "%s: SHORTCUT not specified\n", program_name);
      usage (stderr, program_name);
      result = ERR_USER;
    }

exit:
  poptFreeContext (optCon);
  free (program_name);
  free (opts.target_fname);
  return result;
}

static int startsWithDOSEnvVar(const char* string, size_t n)
{
  if (n > 2 && string[0] == '%')
  {
    const char *p = strchr(&string[1], '%');
    if (p && (p - string) > 1)
      return 1;
  }
  return 0;
}

int
readshortcut (optvals * opts)
{
  HRESULT hres;
  IShellLink *shell_link;
  IPersistFile *persist_file;
  char strPath[MAX_PATH];
  char strBuff[BUFF_SIZE];
  int iBuff;

  int result = ERR_NONE;        /* the value to return on exit */

  /*  Add suffix to link name if necessary */
  if (strlen (opts->target_fname) > 4)
    {
      int tmp = strlen (opts->target_fname) - 4;
      if (strncmp (opts->target_fname + tmp, ".lnk", 4) != 0)
        {
          opts->target_fname =
            (char *) realloc (opts->target_fname,
                              strlen (opts->target_fname) + 1 + 4);
          if (opts->target_fname == NULL)
            {
              fprintf (stderr, "%s: memory allocation error\n", program_name);
              return (ERR_SYS);
            }
          strcat (opts->target_fname, ".lnk");
        }
    }
  else
    {
      opts->target_fname =
        (char *) realloc (opts->target_fname,
                          strlen (opts->target_fname) + 1 + 4);
      if (opts->target_fname == NULL)
        {
          fprintf (stderr, "%s: memory allocation error\n", program_name);
          return (ERR_SYS);
        }
      strcat (opts->target_fname, ".lnk");
    }

  /* if none of the following are true:
   *     there's a colon in the path or
   *     the path starts with two backslashes or
   *     the path starts with a DOS environment variable [*]
   * then it's POSIX and we should convert to win32
   * [*] which itself might have a colon, or start with two backslashes.
   *     we don't know, so assume it probably does.
   */
  if (! ((strchr (opts->target_fname, ':') != NULL) ||
         (strlen (opts->target_fname) > 2 && opts->target_fname[0] == '\\' && opts->target_fname[1] == '\\') ||
	 startsWithDOSEnvVar (opts->target_fname, strlen (opts->target_fname))))
    {
      char *strTmpPath = (char *)cygwin_create_path(CCP_POSIX_TO_WIN_A,
                                                    opts->target_fname);
      if (strTmpPath == NULL)
        {
          fprintf (stderr, "%s: error converting target posix path to win32 (%s)\n",
                   program_name, strerror(errno));
          return (ERR_SYS);
        }
      free (opts->target_fname);
      opts->target_fname = strTmpPath;
    }

  hres = OleInitialize (NULL);
  if (hres != S_FALSE && hres != S_OK)
    {
      fprintf (stderr, "%s: Could not initialize OLE interface\n",
               program_name);
      return (ERR_WIN);
    }

  /* Get a pointer to the IShellLink interface. */
  hres =
    CoCreateInstance (&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                      &IID_IShellLink, (void **) &shell_link);

  if (!SUCCEEDED (hres))
    {
      fprintf (stderr, "%s: CoCreateInstance failed\n", program_name);
      return (ERR_WIN);
    }

  /* Get a pointer to the IPersistFile interface. */
  hres =
    shell_link->lpVtbl->QueryInterface (shell_link, &IID_IPersistFile,
                                        (void **) &persist_file);

  if (SUCCEEDED (hres))
    {
      WCHAR wsz[MAX_PATH];

      /* Ensure that the string is Unicode. */
      MultiByteToWideChar (CP_ACP, 0, (LPCSTR) opts->target_fname, -1, wsz,
                           MAX_PATH);

      /* Load the shortcut.  */
      hres = persist_file->lpVtbl->Load (persist_file, wsz, STGM_READ);

      if (SUCCEEDED (hres))
        {
          /* read stuff from the link object and print it to the screen */
          if (opts->show_all || opts->show_target)
            {
              char *s;
              shell_link->lpVtbl->GetPath (shell_link, strPath, MAX_PATH,
                                           NULL, (opts->show_raw ? SLGP_RAWPATH : 0));
              if (opts->show_field_names)
                {
                  printf ("Target: ");
                }
              s = formatPath (strPath, opts->pathType);
              printf ("%s\n", (s ? s : strPath));
              if (s) free(s);
            }
          if (opts->show_all || opts->show_working_dir)
            {
              char *s;
              shell_link->lpVtbl->GetWorkingDirectory (shell_link, strPath,
                                                       MAX_PATH);
              if (opts->show_field_names)
                {
                  printf ("Working Directory: ");
                }
              s = formatPath (strPath, opts->pathType);
              printf ("%s\n", (s ? s : strPath));
              if (s) free(s);
            }
          if (opts->show_all || opts->show_args)
            {
              shell_link->lpVtbl->GetArguments (shell_link, strBuff,
                                                BUFF_SIZE);
              if (opts->show_field_names)
                {
                  printf ("Arguments: ");
                }
              printf ("%s\n", strBuff);
            }
          if (opts->show_all || opts->show_showCmd)
            {
              shell_link->lpVtbl->GetShowCmd (shell_link, &iBuff);
              if (opts->show_field_names)
                {
                  printf ("Show Command: ");
                }

              switch (iBuff)
                {
                case SW_SHOWNORMAL:
                  printf ("Normal\n");
                  break;
                case SW_SHOWMINIMIZED:
                case SW_SHOWMINIMIZED_SEVEN:
                  printf ("Minimized\n");
                  break;
                case SW_SHOWMAXIMIZED:
                  printf ("Maximized\n");
                  break;
                }
            }
          if (opts->show_all || opts->show_icon || opts->show_icon_offset)
            {
              shell_link->lpVtbl->GetIconLocation (shell_link, strPath,
                                                   MAX_PATH, &iBuff);
              if (opts->show_all || opts->show_icon)
                {
                  char *s;
                  if (opts->show_field_names)
                    {
                      printf ("Icon Library: ");
                    }
                  s = formatPath (strPath, opts->pathType);
                  printf ("%s\n", (s ? s : strPath));
                  if (s) free(s);
                }
              if (opts->show_all || opts->show_icon_offset)
                {
                  if (opts->show_field_names)
                    {
                      printf ("Icon Library Offset: ");
                    }
                  printf ("%d\n", iBuff);
                }
            }
          if (opts->show_all || opts->show_desc)
            {
              shell_link->lpVtbl->GetDescription (shell_link, strBuff,
                                                  BUFF_SIZE);
              if (opts->show_field_names)
                {
                  printf ("Description: ");
                }
              printf ("%s\n", strBuff);
            }
        }
      else
        {
          fprintf (stderr, "%s: Load failed on %s\n", program_name,
                   opts->target_fname);
          result = ERR_WIN;
        }

      /* Release the pointer to the IPersistFile interface. */
      persist_file->lpVtbl->Release (persist_file);
    }
  else
    {
      fprintf (stderr, "%s: QueryInterface failed\n", program_name);
      result = ERR_WIN;
    }

  /* Release the pointer to the IShellLink interface. */
  shell_link->lpVtbl->Release (shell_link);

  return (result);
}

static char* xstrndup(const char* string, size_t n)
{
  char *s = strndup (string, n);
  if (!s)
    fprintf (stderr, "%s: memory allocation error\n", program_name);
  return s;
}


/* change the path to the proper format */
char *
formatPath (char *strPath, int format)
{
  if (format == PATH_WIN)
    {
      char *s = strndup (strPath, MAX_PATH);
      if (!s)
        fprintf (stderr, "%s: memory allocation error\n", program_name);
      return s;
    }                           /* windows is the default */
  else
    {
      /*
       * convert to posix path -- but assume that, if the path starts
       * with a windows environment variable such as %SystemPath% (or
       * even %foo%) that it MIGHT already be an absolute path, and
       * cheat a bit: use relative path conversion.
       */
      cygwin_conv_path_t convType = CCP_WIN_A_TO_POSIX;
      if (startsWithDOSEnvVar(strPath, strlen (strPath)))
	convType |= CCP_RELATIVE;

      char *s = (char *) cygwin_create_path(convType, strPath);
      if (!s)
        {
          fprintf (stderr, "%s: error converting win32 path to posix (%s)\n",
                   program_name, strerror(errno));
        }
      return s;
    }
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
  fprintf (f,
           "  Reads and outputs data from a Windows shortcut (.lnk) file.\n\n");
}

static void
printBottomDescription (FILE * f, char *name)
{
  fprintf (f,
           "\nNOTE: The SHORTCUT argument may be in Windows or Unix format.\n");
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
