/* 
 * readshortcut for cygwin/windows
 *
 * Copyright (C) 2003 Rob Siklos
 * http://www.cygwin.com/ml/cygwin/2003-08/msg00640.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation in version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
#include <olectlid.h>

#define PATH_UNIX 0
#define PATH_WIN  1

#define ERR_NONE 0
#define ERR_USER 1
#define ERR_SYS  2
#define ERR_WIN  3

#define SLGP_RAWPATH 0
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

} optvals;

void cygwin_conv_to_full_win32_path (const char *path, char *win32_path);
void cygwin_conv_to_full_posix_path (const char *path, char *posix_path);

static void printTopDescription (FILE * f, char *name);
static void printBottomDescription (FILE * f, char *name);
static const char *getVersion (void);
static void usage (poptContext optCon, FILE * f, char *name);
static void help (poptContext optCon, FILE * f, char *name);
static void version (poptContext optCon, FILE * f, char *name);
static void license (poptContext optCon, FILE * f, char *name);

void formatPath (char *strPath, int format);
int readshortcut (optvals * opts, poptContext optContext);

static char *program_name;

int
main (int argc, const char **argv)
{
  poptContext optContext;
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

  /* set the pOpt context and help line */
  optContext = poptGetContext (NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp (optContext, "[OPTION]* SHORTCUT");

  while ((rc = poptGetNextOpt (optContext)) > 0)
    {
      switch (rc)
        {
        case '?':
          help (optContext, stdout, program_name);
          goto exit;
        case 'u':
          usage (optContext, stdout, program_name);
          goto exit;
        case 'v':
          version (optContext, stdout, program_name);
          goto exit;
        case 'l':
          license (optContext, stdout, program_name);
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
  loose_args = poptGetArgs (optContext);

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
          usage (optContext, stderr, program_name);
          result = ERR_USER;
        }
      else
        {
      /************** Main Program ***********/
          result = readshortcut (&opts, optContext);
        }
    }
  else
    {
      fprintf (stderr, "%s: SHORTCUT not specified\n", program_name);
      usage (optContext, stderr, program_name);
      result = ERR_USER;
    }

exit:
  poptFreeContext (optContext);
  free (program_name);
  free (opts.target_fname);
  return result;
}

int
readshortcut (optvals * opts, poptContext optContext)
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

  /* if there's no colon in the path, it's POSIX and we should convert to win32 */
  if (strchr (opts->target_fname, ':') == NULL)
    {
      char *strTmpPath = (char *) malloc (MAX_PATH);
      if (strTmpPath == NULL)
        {
          fprintf (stderr, "%s: memory allocation error\n", program_name);
          return (ERR_SYS);
        }
      cygwin_conv_to_full_win32_path (opts->target_fname, strTmpPath);
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
              shell_link->lpVtbl->GetPath (shell_link, strPath, MAX_PATH,
                                           NULL, SLGP_RAWPATH);
              if (opts->show_field_names)
                {
                  printf ("Target: ");
                }
              formatPath (strPath, opts->pathType);
              printf ("%s\n", strPath);
            }
          if (opts->show_all || opts->show_working_dir)
            {
              shell_link->lpVtbl->GetWorkingDirectory (shell_link, strPath,
                                                       MAX_PATH);
              if (opts->show_field_names)
                {
                  printf ("Working Directory: ");
                }
              formatPath (strPath, opts->pathType);
              printf ("%s\n", strPath);
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
                  if (opts->show_field_names)
                    {
                      printf ("Icon Library: ");
                    }
                  formatPath (strPath, opts->pathType);
                  printf ("%s\n", strPath);
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

/* change the path to the proper format */
void
formatPath (char *strPath, int format)
{
  if (format == PATH_WIN)
    {
      return;
    }                           /* windows is the default */
  else
    {
      // convert to posix path
      char strTmp[MAX_PATH];
      strcpy (strTmp, strPath);
      cygwin_conv_to_full_posix_path (strTmp, strPath);
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
           "This program is free software; you can redistribute it and/or\n");
  fprintf (f,
           "modify it under the terms of the GNU General Public License\n");
  fprintf (f,
           "as published by the Free Software Foundation; either version 2\n");
  fprintf (f, "of the License, or (at your option) any later version.\n");
  fprintf (f, "\n");
  fprintf (f,
           "This program is distributed in the hope that it will be useful,\n");
  fprintf (f,
           "but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
  fprintf (f,
           "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
  fprintf (f, "GNU General Public License for more details.\n");
  fprintf (f, "\n");
  fprintf (f,
           "You should have received a copy of the GNU General Public License\n");
  fprintf (f,
           "along with this program; if not, write to the Free Software\n");
  fprintf (f,
           "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n");
  fprintf (f, "\n");
  fprintf (f, "See the COPYING file for license information.\n");
}

static void
usage (poptContext optCon, FILE * f, char *name)
{
  poptPrintUsage (optCon, f, 0);
}

static void
help (poptContext optCon, FILE * f, char *name)
{
  printTopDescription (f, name);
  poptPrintHelp (optCon, f, 0);
  printBottomDescription (f, name);
}

static void
version (poptContext optCon, FILE * f, char *name)
{
  printTopDescription (f, name);
}

static void
license (poptContext optCon, FILE * f, char *name)
{
  printTopDescription (f, name);
  printLicense (f, name);
}
