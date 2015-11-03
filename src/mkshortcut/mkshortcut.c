/* mkshortcut.c -- create a Windows shortcut
 *
 * Copyright (c) 2002 Joshua Daniel Franklin
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
 *
 * Exit values
 *   1: user error (syntax error)
 *   2: system error (out of memory, etc.)
 *   3: windows error (interface failed)
 *
 * Compile with: gcc -o prog.exe mkshortcut.c -lpopt -lole32 -luuid
 *  (You'd need to uncomment the moved to common.h lines.)
 *
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include "common.h"

#define NOCOMATTRIBUTE

#include <shlobj.h>
#include <olectl.h>
/* moved to common.h */
/*
#include <stdio.h>
#include <popt.h>
*/
#include <sys/cygwin.h>

static const char versionID[] = PACKAGE_VERSION;
static const char revID[] =
  "$Id$";
static const char copyrightID[] =
  "Copyright (c) 2002\nJoshua Daniel Franklin. All rights reserved.\nLicensed under GPL v2.0\n";

typedef struct optvals_s
{
  int icon_flag;
  int unix_flag;
  int windows_flag;
  int allusers_flag;
  int desktop_flag;
  int smprograms_flag;
  int show_flag;
  int offset;
  char *name_arg;
  char *desc_arg;
  char *dir_name_arg;
  char *argument_arg;
  char *target_arg;
  char *icon_name_arg;
} optvals;

static int mkshortcut (optvals opts);
static void printTopDescription (FILE * f, char *name);
static void printBottomDescription (FILE * f, char *name);
static const char *getVersion ();
static void usage (FILE * f, char *name);
static void help (FILE * f, char *name);
static void version (FILE * f, char *name);
static void license (FILE * f, char *name);

static char *program_name;
static poptContext optCon;

int
main (int argc, const char **argv)
{
  const char **rest;
  int rc;
  int ec = 0;
  optvals opts;

  const char *tmp_str;
  int icon_offset_flag;
  const char *arg;

  struct poptOption helpOptionsTable[] = {
    {"help", 'h', POPT_ARG_NONE, NULL, '?',
     "Show this help message", NULL},
    {"usage", '\0', POPT_ARG_NONE, NULL, 'u',
     "Display brief usage message", NULL},
    {"version", 'v', POPT_ARG_NONE, NULL, 'v',
     "Display version information", NULL},
    {"license", '\0', POPT_ARG_NONE, NULL, 'l',
     "Display licensing information", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

  struct poptOption generalOptionsTable[] = {
    {"arguments", 'a', POPT_ARG_STRING, NULL, 'a',
     "Use arguments ARGS", "ARGS"},
    {"desc", 'd', POPT_ARG_STRING, NULL, 'd',
     "Text for description/tooltip (defaults to POSIX path of TARGET)",
     "DESC"},
    {"icon", 'i', POPT_ARG_STRING, NULL, 'i',
     "Icon file for link to use", "ICONFILE"},
    {"iconoffset", 'j', POPT_ARG_INT, &(opts.offset), 'j',
     "Offset of icon in icon file (default is 0)", NULL},
    {"name", 'n', POPT_ARG_STRING, NULL, 'n',
     "Name for link (defaults to TARGET)", "NAME"},
    {"show", 's', POPT_ARG_STRING, NULL, 's',
     "Window to show: normal, minimized, maximized", "norm|min|max"},
    {"workingdir", 'w', POPT_ARG_STRING, NULL, 'w',
     "Set working directory (defaults to directory path of TARGET)", "PATH"},
    {"allusers", 'A', POPT_ARG_VAL, &(opts.allusers_flag), 1,
     "Use 'All Users' instead of current user for -D,-P", NULL},
    {"desktop", 'D', POPT_ARG_VAL, &(opts.desktop_flag), 1,
     "Create link relative to 'Desktop' directory", NULL},
    {"smprograms", 'P', POPT_ARG_VAL, &(opts.smprograms_flag), 1,
     "Create link relative to Start Menu 'Programs' directory", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

  struct poptOption opt[] = {
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, generalOptionsTable, 0,
     "General options", NULL},
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0,
     "Help options", NULL},
    {NULL, '\0', 0, NULL, 0, NULL, NULL}
  };

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
      exit (2);
    }

  icon_offset_flag = 0;

  opts.offset = 0;
  opts.icon_flag = 0;
  opts.unix_flag = 0;
  opts.windows_flag = 0;
  opts.allusers_flag = 0;
  opts.desktop_flag = 0;
  opts.smprograms_flag = 0;
  opts.show_flag = SW_SHOWNORMAL;
  opts.target_arg = NULL;
  opts.argument_arg = NULL;
  opts.name_arg = NULL;
  opts.desc_arg = NULL;
  opts.dir_name_arg = NULL;
  opts.icon_name_arg = NULL;

  /* Parse options */
  optCon = poptGetContext (NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp (optCon, "[OPTION]* TARGET");
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
          if (arg = poptGetOptArg (optCon))
            {
              if ((opts.desc_arg = strdup (arg)) == NULL)
                {
                  fprintf (stderr, "%s: memory allocation error\n",
                           program_name);
                  ec = 2;
                  goto exit;
                }
            }
          break;
        case 'i':
          opts.icon_flag = 1;
          if (arg = poptGetOptArg (optCon))
            {
              opts.icon_name_arg = (char *) cygwin_create_path(
                  CCP_POSIX_TO_WIN_A, arg);
              if (opts.icon_name_arg == NULL)
                {
                  fprintf (stderr, "%s: error converting posix path to win32 (%s)\n",
                           program_name, strerror(errno));
                  ec = 2;
                  goto exit;
                }
            }
          break;
        case 'j':
          icon_offset_flag = 1;
          break;
        case 'n':
          if (arg = poptGetOptArg (optCon))
            {
              if ((opts.name_arg = strdup (arg)) == NULL)
                {
                  fprintf (stderr, "%s: memory allocation error\n",
                           program_name);
                  ec = 2;
                  goto exit;
                }
            }
          break;
        case 's':
          if (arg = poptGetOptArg (optCon))
            {
              if (strcmp (arg, "min") == 0)
                {
                  opts.show_flag = SW_SHOWMINNOACTIVE;
                }
              else if (strcmp (arg, "max") == 0)
                {
                  opts.show_flag = SW_SHOWMAXIMIZED;
                }
              else if (strcmp (arg, "norm") == 0)
                {
                  opts.show_flag = SW_SHOWNORMAL;
                }
              else
                {
                  fprintf (stderr, "%s: %s not valid for show window\n",
                           program_name, arg);
                  ec = 2;
                  goto exit;
                }
            }
          break;
        case 'w':
          if (arg = poptGetOptArg (optCon))
            {
              if ((opts.dir_name_arg = strdup (arg)) == NULL)
                {
                  fprintf (stderr, "%s: memory allocation error\n",
                           program_name);
                  ec = 2;
                  goto exit;
                }
            }
          break;
        case 'a':
          if (arg = poptGetOptArg (optCon))
            {
              if ((opts.argument_arg = strdup (arg)) == NULL)
                {
                  fprintf (stderr, "%s: memory allocation error\n",
                           program_name);
                  ec = 2;
                  goto exit;
                }
            }
          break;
          // case 'A' 
          // case 'D'
          // case 'P' all handled by popt itself
        }
    }

  if (icon_offset_flag & !opts.icon_flag)
    {
      fprintf (stderr,
               "%s: --iconoffset|-j only valid in conjuction with --icon|-i\n",
               program_name);
      usage (stderr, program_name);
      ec = 1;
      goto exit;
    }

  if (opts.smprograms_flag && opts.desktop_flag)
    {
      fprintf (stderr,
               "%s: --smprograms|-P not valid in conjuction with --desktop|-D\n",
               program_name);
      usage (stderr, program_name);
      ec = 1;
      goto exit;
    }

  if (rc < -1)
    {
      fprintf (stderr, "%s: bad argument %s: %s\n",
               program_name, poptBadOption (optCon, POPT_BADOPTION_NOALIAS),
               poptStrerror (rc));
      ec = 1;
      goto exit;
    }

  rest = poptGetArgs (optCon);

  if (rest && *rest)
    {
      if ((opts.target_arg = strdup (*rest)) == NULL)
        {
          fprintf (stderr, "%s: memory allocation error\n", program_name);
          ec = 2;
          goto exit;
        }
      rest++;
      if (rest && *rest)
        {
          fprintf (stderr, "%s: Too many arguments: ", program_name);
          while (*rest)
            fprintf (stderr, "%s ", *rest++);
          fprintf (stderr, "\n");
          usage (stderr, program_name);
          ec = 1;
        }
      else
        {
          // THE MEAT GOES HERE
          ec = mkshortcut (opts);
        }
    }
  else
    {
      fprintf (stderr, "%s: TARGET not specified\n", program_name);
      usage (stderr, program_name);
      ec = 1;
    }

exit:
  poptFreeContext (optCon);
  if (opts.target_arg)
    {
      free (opts.target_arg);
    }
  if (opts.name_arg)
    {
      free (opts.name_arg);
    }
  if (opts.desc_arg)
    {
      free (opts.desc_arg);
    }
  if (opts.dir_name_arg)
    {
      free (opts.dir_name_arg);
    }
  if (opts.argument_arg)
    {
      free (opts.argument_arg);
    }
  if (opts.icon_name_arg)
    {
      free (opts.icon_name_arg);
    }
  free (program_name);
  return (ec);
}

static char *
xstrndup (const char *string, size_t n)
{
  char *s = strndup (string, n);
  if (!s)
  {
    fprintf (stderr, "%s: out of memory\n", program_name);
    exit(2);
  }
  return s;
}

static char *
xstrncat (char **dest, const char *add, size_t n)
{
  size_t len = strlen (*dest) + n + 1;
  char *s = (char *) realloc (*dest, len * sizeof (char));
  if (!s)
  {
    fprintf (stderr, "%s: out of memory\n", program_name);
    exit(2);
  }
  *dest = s;
  return strncat (*dest, add, n);
}

int
mkshortcut (optvals opts)
{
  char* link_name = NULL;
  char* exe_name = NULL;
  char* dir_name = NULL;
  char* desc = NULL;
  char *buf_str, *tmp_str, *base_str;
  int tmp;

  /* For OLE interface */
  LPITEMIDLIST id;
  HRESULT hres;
  IShellLink *shell_link;
  IPersistFile *persist_file;
  WCHAR widepath[MAX_PATH];
  char link_path[MAX_PATH];

  /*  If there's a colon in the TARGET, it should be a URL */
  if (strchr (opts.target_arg, ':') != NULL)
    {
      /*  Nope, somebody's trying a W32 path  */
      if (opts.target_arg[1] == ':')
        {
          fprintf (stderr, "%s: all paths must be in POSIX format\n",
                   program_name);
          usage (stderr, program_name);
          return (1);
        }
      exe_name = xstrndup (opts.target_arg, strlen(opts.target_arg));
      dir_name = xstrndup ("", 0);  /* No working dir for URL */
    }
  /* Convert TARGET to win32 path */
  else
    {
      buf_str = xstrndup (opts.target_arg, strlen(opts.target_arg));
      exe_name = (char *) cygwin_create_path (CCP_POSIX_TO_WIN_A,
                                              opts.target_arg);
      if (!exe_name)
      {
        fprintf (stderr, "%s: error converting posix path to win32 (%s)\n",
                 program_name, strerror(errno));
        free (buf_str);
        return 2;
      }

      /*  Get a working dir from 'w' option */
      if (opts.dir_name_arg != NULL)
        {
          if (strchr (opts.dir_name_arg, ':') != NULL)
            {
              free (buf_str);
              free (exe_name);
              fprintf (stderr, "%s: all paths must be in POSIX format\n",
                       program_name);
              usage (stderr, program_name);
              return (1);
            }
          dir_name = (char *) cygwin_create_path (CCP_POSIX_TO_WIN_A,
                                                  opts.dir_name_arg);
          if (!dir_name)
          {
            fprintf (stderr, "%s: error converting posix path to win32 (%s)\n",
                     program_name, strerror(errno));
            free (buf_str);
            free (exe_name);
            return 2;
          }
        }
      /*  Get a working dir from the exepath */
      else
        {
          tmp_str = strrchr (exe_name, '\\');
          tmp = strlen (exe_name) - strlen (tmp_str);
          dir_name = xstrndup (exe_name, tmp);
          dir_name[tmp] = '\0';
        }
      if (buf_str) free (buf_str);
    }

  /*  Generate a name for the link if not given */
  if (opts.name_arg == NULL)
    {
      /*  Strip trailing /'s if any */
      buf_str = xstrndup (opts.target_arg, strlen(opts.target_arg));
      base_str = buf_str;
      tmp_str = buf_str;
      tmp = strlen (buf_str) - 1;
      while (strrchr (buf_str, '/') == (buf_str + tmp))
        {
          buf_str[tmp] = '\0';
          tmp--;
        }
      /*  Get basename */
      while (*buf_str)
        {
          if (*buf_str == '/')
            tmp_str = buf_str + 1;
          buf_str++;
        }
      link_name = xstrndup (tmp_str, strlen (tmp_str));
      free (base_str);
    }
  /*  User specified a name, so check it and convert  */
  else
    {
      if (opts.desktop_flag || opts.smprograms_flag)
        {
          /*  Cannot have absolute path relative to Desktop/SM Programs */
          if (opts.name_arg[0] == '/')
            {
              fprintf (stderr,
                       "%s: absolute pathnames not allowed with -D/-P\n",
                       program_name);
              usage (stderr, program_name);
              return (1);
            }
        }
      /*  Sigh. Another W32 path */
      if (strchr (opts.name_arg, ':') != NULL)
        {
          fprintf (stderr, "%s: all paths must be in POSIX format\n",
                   program_name);
          usage (stderr, program_name);
          return (1);
        }
      link_name = (char *) cygwin_create_path (
          CCP_POSIX_TO_WIN_A | CCP_RELATIVE, opts.name_arg);
      if (!link_name)
      {
        fprintf (stderr, "%s: error converting posix path to win32 (%s)\n",
                 program_name, strerror(errno));
        return 2;
      }
    }

  /*  Add suffix to link name if necessary */
  if (strlen (link_name) > 4)
    {
      tmp = strlen (link_name) - 4;
      if (strncmp (link_name + tmp, ".lnk", 4) != 0)
        xstrncat (&link_name, ".lnk", 4);
    }
  else
    xstrncat (&link_name, ".lnk", 4);

  /*  Prepend relative path if necessary  */
  if (opts.desktop_flag)
    {
      char local_buf[MAX_PATH];
      buf_str = xstrndup (link_name, strlen (link_name));

      if (!opts.allusers_flag)
        SHGetSpecialFolderLocation (NULL, CSIDL_DESKTOPDIRECTORY, &id);
      else
        SHGetSpecialFolderLocation (NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &id);
      SHGetPathFromIDList (id, local_buf);
      /*  Make sure Win95 without "All Users" has output  */
      if (strlen (local_buf) == 0)
        {
          SHGetSpecialFolderLocation (NULL, CSIDL_DESKTOPDIRECTORY, &id);
          SHGetPathFromIDList (id, local_buf);
        }
      free (link_name);
      link_name = xstrndup (local_buf, strlen (local_buf));
      xstrncat (&link_name, "\\", 1);
      xstrncat (&link_name, buf_str, strlen (buf_str));
      free (buf_str);
    }

  if (opts.smprograms_flag)
    {
      char local_buf[MAX_PATH];
      buf_str = xstrndup (link_name, strlen (link_name));

      if (!opts.allusers_flag)
        SHGetSpecialFolderLocation (NULL, CSIDL_PROGRAMS, &id);
      else
        SHGetSpecialFolderLocation (NULL, CSIDL_COMMON_PROGRAMS, &id);
      SHGetPathFromIDList (id, local_buf);
      /*  Make sure Win95 without "All Users" has output  */
      if (strlen (local_buf) == 0)
        {
          SHGetSpecialFolderLocation (NULL, CSIDL_PROGRAMS, &id);
          SHGetPathFromIDList (id, local_buf);
        }
      free (link_name);
      link_name = xstrndup (local_buf, strlen (local_buf));
      xstrncat (&link_name, "\\", 1);
      xstrncat (&link_name, buf_str, strlen (buf_str));
      free (buf_str);
    }

  /* After Windows 7, saving link to relative path fails; work around that */
  hres = GetFullPathName (link_name, sizeof (link_path), link_path, 0);
  if (hres == 0)
    {
      fprintf (stderr, "%s: Could not qualify link name\n", program_name);
      return 2;
    }
  free (link_name);
  link_name = xstrndup (link_path, strlen (link_path));

  /* Setup description text */
  if (opts.desc_arg != NULL)
    {
      desc = xstrndup (opts.desc_arg, strlen (opts.desc_arg));
    }
  else
    {
      /* Put the POSIX path in the "Description", just to be nice */
      desc = (char *) cygwin_create_path (CCP_WIN_A_TO_POSIX, exe_name);
      if (!desc)
      {
        fprintf (stderr, "%s: error converting win32 path to posix (%s)\n",
                 program_name, strerror(errno));
        return 2;
      }
    }

  /*  Beginning of Windows interface */
  hres = OleInitialize (NULL);
  if (hres != S_FALSE && hres != S_OK)
    {
      fprintf (stderr, "%s: Could not initialize OLE interface\n",
               program_name);
      free (link_name);
      free (exe_name);
      free (dir_name);
      free (desc);
      return (3);
    }

  hres =
    CoCreateInstance (&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                      &IID_IShellLink, (void **) &shell_link);
  if (SUCCEEDED (hres))
    {
      hres =
        shell_link->lpVtbl->QueryInterface (shell_link, &IID_IPersistFile,
                                            (void **) &persist_file);
      if (SUCCEEDED (hres))
        {
          shell_link->lpVtbl->SetPath (shell_link, exe_name);
          shell_link->lpVtbl->SetDescription (shell_link, desc);
          shell_link->lpVtbl->SetWorkingDirectory (shell_link, dir_name);
          if (opts.argument_arg)
            shell_link->lpVtbl->SetArguments (shell_link, opts.argument_arg);
          if (opts.icon_flag)
            shell_link->lpVtbl->SetIconLocation (shell_link,
                                                 opts.icon_name_arg,
                                                 opts.offset);
          if (opts.show_flag != SW_SHOWNORMAL)
            shell_link->lpVtbl->SetShowCmd (shell_link, opts.show_flag);


          /*  Make link name Unicode-compliant  */
          hres =
            MultiByteToWideChar (CP_ACP, 0, link_name, -1, widepath,
                                 MAX_PATH);
          if (!SUCCEEDED (hres))
            {
              fprintf (stderr, "%s: Unicode translation failed%d\n",
                       program_name, hres);
              free (link_name);
              free (exe_name);
              free (dir_name);
              free (desc);
              return (3);
            }
          hres = persist_file->lpVtbl->Save (persist_file, widepath, TRUE);
          if (!SUCCEEDED (hres))
            {
              fprintf (stderr,
                       "%s: Saving \"%s\" failed; does the target directory exist?\n",
                       program_name, link_name);
              free (link_name);
              free (exe_name);
              free (dir_name);
              free (desc);
              return (3);
            }
          persist_file->lpVtbl->Release (persist_file);
          shell_link->lpVtbl->Release (shell_link);

          /* If we are creating shortcut for all users, ensure it is readable by all users */
          if (opts.allusers_flag)
            {
              char *posixpath = (char *) cygwin_create_path (
                CCP_WIN_W_TO_POSIX | CCP_ABSOLUTE, widepath);
              if (posixpath && *posixpath)
                {
                  struct stat statbuf;
                  if (stat(posixpath, &statbuf))
                  {
                    fprintf (stderr,
                             "%s: stat \"%s\" failed\n",
                             program_name, posixpath);
                  }
                  else if (chmod(posixpath, statbuf.st_mode|S_IRUSR|S_IRGRP|S_IROTH))
                  {
                    fprintf (stderr,
                             "%s: chmod \"%s\" failed\n",
                             program_name, posixpath);
                  }
                }
              free(posixpath);
            }
          free (link_name);
          free (exe_name);
          free (dir_name);
          free (desc);
          return (0);
        }
      else
        {
          fprintf (stderr, "%s: QueryInterface failed\n", program_name);
          free (link_name);
          free (exe_name);
          free (dir_name);
          free (desc);
          return (3);
        }
    }
  else
    {
      fprintf (stderr, "%s: CoCreateInstance failed\n", program_name);
      free (link_name);
      free (exe_name);
      free (dir_name);
      free (desc);
      return (3);
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
  char s[20];
  fprintf (f, "%s is part of cygutils version %s\n", name, getVersion ());
  fprintf (f, "  create a Windows shortcut\n\n");
}
static void
printBottomDescription (FILE * f, char *name)
{
  fprintf (f,
           "\nNOTE: All filename arguments must be in unix (POSIX) format\n");
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
  fprintf (f, copyrightID);
}

static void
license (FILE * f, char *name)
{
  printTopDescription (f, name);
  printLicense (f, name);
}
