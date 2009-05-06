/* mkshortcut.c -- create a Windows shortcut
 *
 * Copyright (c) 2002 Joshua Daniel Franklin
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * See the COPYING file for license information.
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
#  include "config.h"
#endif
#include "common.h"

#define NOCOMATTRIBUTE

#include <shlobj.h>
#include <olectlid.h>
/* moved to common.h */
/*
#include <stdio.h>
#include <popt.h>
*/

static const char versionID[] = PACKAGE_VERSION;
static const char revID[] =
  "$Id$";
static const char copyrightID[] =
  "Copyright (c) 2002\nJoshua Daniel Franklin. All rights reserved.\nLicensed under GPL v2.0\n";

typedef struct optvals_s {
  int icon_flag;
  int unix_flag;
  int windows_flag;
  int allusers_flag;
  int desktop_flag;
  int smprograms_flag;
  int show_flag;
  int offset;
  char * name_arg;
  char * desc_arg;
  char * dir_name_arg;
  char * argument_arg;
  char * target_arg;
  char * icon_name_arg;
} optvals;

static int mkshortcut(optvals opts, poptContext optCon);
static void printTopDescription(FILE * f, char * name);
static void printBottomDescription(FILE * f, char * name);
static const char * getVersion();
static void usage(poptContext optCon, FILE * f, char * name);
static void help(poptContext optCon, FILE * f, char * name);
static void version(poptContext optCon, FILE * f, char * name);
static void license(poptContext optCon, FILE * f, char * name);

static char *program_name;

int
main (int argc, const char **argv)
{
  poptContext optCon;
  const char ** rest;
  int rc;
  int ec = 0;
  optvals opts;

  const char *tmp_str;
  int icon_offset_flag;
  char icon_name[MAX_PATH];
  const char * arg;

  struct poptOption helpOptionsTable[] = {
    { "help", 'h',  POPT_ARG_NONE, NULL, '?', \
        "Show this help message", NULL},
    { "usage", '\0', POPT_ARG_NONE, NULL, 'u', \
        "Display brief usage message", NULL},
    { "version", 'v', POPT_ARG_NONE, NULL, 'v', \
        "Display version information", NULL},
    { "license", '\0', POPT_ARG_NONE, NULL, 'l', \
        "Display licensing information", NULL},
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };

  struct poptOption generalOptionsTable[] = {
    { "arguments", 'a',  POPT_ARG_STRING, NULL, 'a', \
        "Use arguments ARGS", "ARGS"},
    { "desc", 'd', POPT_ARG_STRING, NULL, 'd', \
        "Text for description/tooltip (defaults to POSIX path of TARGET)", "DESC"},
    { "icon", 'i', POPT_ARG_STRING, NULL, 'i', \
        "Icon file for link to use", "ICONFILE"},
    { "iconoffset", 'j', POPT_ARG_INT, &(opts.offset), 'j', \
        "Offset of icon in icon file (default is 0)", NULL},
    { "name", 'n', POPT_ARG_STRING, NULL, 'n', \
        "Name for link (defaults to TARGET)", "NAME"},
    { "show", 's', POPT_ARG_STRING, NULL, 's', \
        "Window to show: normal, minimized, maximized", "norm|min|max"},
    { "workingdir", 'w', POPT_ARG_STRING, NULL, 'w', \
        "Set working directory (defaults to directory path of TARGET)", "PATH"},
    { "allusers", 'A', POPT_ARG_VAL, &(opts.allusers_flag), 1, \
        "Use 'All Users' instead of current user for -D,-P", NULL},
    { "desktop", 'D', POPT_ARG_VAL, &(opts.desktop_flag), 1, \
        "Create link relative to 'Desktop' directory", NULL},
    { "smprograms", 'P', POPT_ARG_VAL, &(opts.smprograms_flag), 1, \
        "Create link relative to Start Menu 'Programs' directory", NULL},
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };
  
  struct poptOption opt[] = {
    { NULL, '\0', POPT_ARG_INCLUDE_TABLE, generalOptionsTable, 0, \
        "General options", NULL },
    { NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0, \
        "Help options", NULL },
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };

  tmp_str = strrchr (argv[0], '/');
  if (tmp_str == NULL) {
    tmp_str = strrchr (argv[0], '\\');
  }
  if (tmp_str == NULL) {
    tmp_str = argv[0];
  } else {
    tmp_str++;
  }
  if ((program_name = strdup(tmp_str)) == NULL ) {
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(2);
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
  optCon = poptGetContext(NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp(optCon, "[OPTION]* TARGET");
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
      case 'd':  if (arg = poptGetOptArg(optCon)) {
                   if ((opts.desc_arg = strdup(arg)) == NULL ) {
                     fprintf(stderr, "%s: memory allocation error\n", program_name);
                     ec=2;
                     goto exit;
                   }
                 }
                 break;
      case 'i':  opts.icon_flag = 1;
                 if (arg = poptGetOptArg(optCon)) {
                   cygwin_conv_to_full_win32_path (arg, icon_name);
                   if ((opts.icon_name_arg = strdup(icon_name)) == NULL ) {
                     fprintf(stderr, "%s: memory allocation error\n", program_name);
                     ec=2;
                     goto exit;
                   }
                 }
                 break;
      case 'j':  icon_offset_flag = 1;
                 break;
      case 'n':  if (arg = poptGetOptArg(optCon)) {
                   if ((opts.name_arg = strdup(arg)) == NULL ) {
                     fprintf(stderr, "%s: memory allocation error\n", program_name);
                     ec=2;                                            
                     goto exit;
                   }
                 }
                 break;
      case 's':  if (arg = poptGetOptArg(optCon)) {
                   if (strcmp(arg, "min") == 0) {
                     opts.show_flag = SW_SHOWMINNOACTIVE;
                   } else if (strcmp(arg, "max") == 0) {
                     opts.show_flag = SW_SHOWMAXIMIZED;
                   } else if (strcmp(arg, "norm") == 0) {
                     opts.show_flag = SW_SHOWNORMAL;
                   } else {
                         fprintf(stderr, "%s: %s not valid for show window\n",
                               program_name, arg);
                         ec=2;                                            
                         goto exit;
                   }
                 }
                 break;
      case 'w':  if (arg = poptGetOptArg(optCon)) {
                   if ((opts.dir_name_arg = strdup(arg)) == NULL ) {
                     fprintf(stderr, "%s: memory allocation error\n", program_name);
                     ec=2;                                            
                     goto exit;
                   }
                 }
                 break;
      case 'a':  if (arg = poptGetOptArg(optCon)) {
                   if ((opts.argument_arg = strdup(arg)) == NULL ) {
                     fprintf(stderr, "%s: memory allocation error\n", program_name);
                     ec=2;                                            
                     goto exit;
                   }
                 }
                 break;
      // case 'A' 
      // case 'D'
      // case 'P' all handled by popt itself
    } 
  }
  
  if (icon_offset_flag & !opts.icon_flag) {
    fprintf(stderr, 
      "%s: --iconoffset|-j only valid in conjuction with --icon|-i\n",
      program_name); 
    usage(optCon, stderr, program_name);
    ec=1;
    goto exit;
  }

  if (opts.smprograms_flag && opts.desktop_flag) {
    fprintf(stderr, 
      "%s: --smprograms|-P not valid in conjuction with --desktop|-D\n",
      program_name); 
    usage(optCon, stderr, program_name);
    ec=1;
    goto exit;
  }

  if (rc < -1 ) {
    fprintf(stderr, "%s: bad argument %s: %s\n",
      program_name, poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
      poptStrerror(rc));
    ec = 1;
    goto exit;
  }

  rest = poptGetArgs(optCon);

  if (rest && *rest) {
    if ((opts.target_arg = strdup(*rest)) == NULL) {
      fprintf(stderr, "%s: memory allocation error\n", program_name);
      ec=2;
      goto exit;
    }
    rest++;
    if (rest && *rest) {
      fprintf(stderr, "%s: Too many arguments: ", program_name);
      while (*rest)
        fprintf(stderr, "%s ", *rest++);
      fprintf(stderr, "\n"); 
      usage(optCon, stderr, program_name);
      ec=1;
    } else {
      // THE MEAT GOES HERE
      ec = mkshortcut(opts, optCon);
    }
  } else {
    fprintf(stderr, "%s: TARGET not specified\n", program_name);
    usage(optCon, stderr, program_name);
    ec=1;
  }

exit:
  poptFreeContext(optCon);
  if (opts.target_arg)    { free(opts.target_arg);    }
  if (opts.name_arg)      { free(opts.name_arg);      }
  if (opts.desc_arg)      { free(opts.desc_arg);      }
  if (opts.dir_name_arg)  { free(opts.dir_name_arg);  }
  if (opts.argument_arg)  { free(opts.argument_arg);  } 
  if (opts.icon_name_arg) { free(opts.icon_name_arg); } 
  free(program_name);
  return(ec);
}

int mkshortcut(optvals opts, poptContext optCon)
{
  char link_name[MAX_PATH];
  char exe_name[MAX_PATH];
  char dir_name[MAX_PATH];
  char desc[MAX_PATH];
  char *buf_str, *tmp_str;
  int tmp;

  /* For OLE interface */
  LPITEMIDLIST id;
  HRESULT hres;
  IShellLink *shell_link;
  IPersistFile *persist_file;
  WCHAR widepath[MAX_PATH];

  buf_str = (char *) malloc (PATH_MAX);
  if (buf_str == NULL) {
    fprintf (stderr, "%s: out of memory\n", program_name);
    return(2);
  }

  /*  If there's a colon in the TARGET, it should be a URL */
  if (strchr (opts.target_arg, ':') != NULL)
    {
      /*  Nope, somebody's trying a W32 path  */
      if (opts.target_arg[1] == ':') {
	free (buf_str);
	fprintf(stderr, "%s: all paths must be in POSIX format\n", 
          program_name);
	usage (optCon, stderr, program_name);
        return(1);
      }
      strcpy (exe_name, opts.target_arg);
      dir_name[0] = '\0';	/* No working dir for URL */
    }
  /* Convert TARGET to win32 path */
  else
    {
      strcpy (buf_str, opts.target_arg);
      cygwin_conv_to_full_win32_path (buf_str, exe_name);

      /*  Get a working dir from 'w' option */
      if (opts.dir_name_arg != NULL)
        {
          if (strchr (opts.dir_name_arg, ':') != NULL)
            {
              free (buf_str);
              fprintf(stderr, "%s: all paths must be in POSIX format\n", 
                program_name);
              usage (optCon, stderr, program_name);
              return(1);
            }
          cygwin_conv_to_win32_path (opts.dir_name_arg, dir_name);
        }
      /*  Get a working dir from the exepath */
      else
        {
          tmp_str = strrchr (exe_name, '\\');
          tmp = strlen (exe_name) - strlen (tmp_str);
          strncpy (dir_name, exe_name, tmp);
          dir_name[tmp] = '\0';
        }
    }

  /*  Generate a name for the link if not given */
  if (opts.name_arg == NULL)
    {
      /*  Strip trailing /'s if any */
      strcpy (buf_str, opts.target_arg);
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
      strcpy (link_name, tmp_str);
    } 
  /*  User specified a name, so check it and convert  */
  else
    {
      if (opts.desktop_flag || opts.smprograms_flag)
	{
	  /*  Cannot have absolute path relative to Desktop/SM Programs */
	  if (opts.name_arg[0] == '/') {
	    free (buf_str);
	    fprintf(stderr, "%s: absolute pathnames not allowed with -D/-P\n",
              program_name);
	    usage (optCon, stderr, program_name);
            return(1);
          }
	}
      /*  Sigh. Another W32 path */
      if (strchr (opts.name_arg, ':') != NULL) {
	free (buf_str);
	fprintf(stderr, "%s: all paths must be in POSIX format\n", 
          program_name);
	usage (optCon, stderr, program_name);
        return(1);
      }
      cygwin_conv_to_win32_path (opts.name_arg, link_name);
    }    

  /*  Add suffix to link name if necessary */
  if (strlen (link_name) > 4)
    {
      tmp = strlen (link_name) - 4;
      if (strncmp (link_name + tmp, ".lnk", 4) != 0)
	strcat (link_name, ".lnk");
    }
  else
    strcat (link_name, ".lnk");

  /*  Prepend relative path if necessary  */
  if (opts.desktop_flag)
    {
      strcpy (buf_str, link_name);
      if (!opts.allusers_flag)
	SHGetSpecialFolderLocation (NULL, CSIDL_DESKTOPDIRECTORY, &id);
      else
	SHGetSpecialFolderLocation (NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &id);
      SHGetPathFromIDList (id, link_name);
      /*  Make sure Win95 without "All Users" has output  */
      if (strlen (link_name) == 0)
	{
	  SHGetSpecialFolderLocation (NULL, CSIDL_DESKTOPDIRECTORY, &id);
	  SHGetPathFromIDList (id, link_name);
	}
      strcat (link_name, "\\");
      strcat (link_name, buf_str);
    }

  if (opts.smprograms_flag)
    {
      strcpy (buf_str, link_name);
      if (!opts.allusers_flag)
	SHGetSpecialFolderLocation (NULL, CSIDL_PROGRAMS, &id);
      else
	SHGetSpecialFolderLocation (NULL, CSIDL_COMMON_PROGRAMS, &id);
      SHGetPathFromIDList (id, link_name);
      /*  Make sure Win95 without "All Users" has output  */
      if (strlen (link_name) == 0)
	{
	  SHGetSpecialFolderLocation (NULL, CSIDL_PROGRAMS, &id);
	  SHGetPathFromIDList (id, link_name);
	}
      strcat (link_name, "\\");
      strcat (link_name, buf_str);
    }

  /* Setup description text */
  if (opts.desc_arg != NULL)
    {
      strncpy (desc, opts.desc_arg, MAX_PATH);

      /* There won't be a null terminated if strlen(desc_arg)>MAX_PATH */
      desc[MAX_PATH] = '\0';
    }
  else
    {
      /* Put the POSIX path in the "Description", just to be nice */
      cygwin_conv_to_full_posix_path (exe_name, desc);
    }

  /*  Beginning of Windows interface */
  hres = OleInitialize (NULL);
  if (hres != S_FALSE && hres != S_OK)
    {    
      free (buf_str);
      fprintf (stderr, "%s: Could not initialize OLE interface\n", 
        program_name);
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
	    shell_link->lpVtbl->SetIconLocation (shell_link, opts.icon_name_arg,
						 opts.offset);
          if (opts.show_flag != SW_SHOWNORMAL)
            shell_link->lpVtbl->SetShowCmd(shell_link, opts.show_flag);


	  /*  Make link name Unicode-compliant  */
	  hres =
	    MultiByteToWideChar (CP_ACP, 0, link_name, -1, widepath,
				 MAX_PATH);
	  if (!SUCCEEDED (hres))
	    {
	      free (buf_str);
	      fprintf (stderr, "%s: Unicode translation failed%d\n",
		       program_name, hres);
	      return (3);
	    }
	  hres = persist_file->lpVtbl->Save (persist_file, widepath, TRUE);
	  if (!SUCCEEDED (hres))
	    {
	      free (buf_str);
	      fprintf (stderr,
		"%s: Saving \"%s\" failed; does the target directory exist?\n",
		program_name, link_name);
	      return (3);
	    }
	  persist_file->lpVtbl->Release (persist_file);
	  shell_link->lpVtbl->Release (shell_link);
	}
      else
	{
	  free (buf_str);
	  fprintf (stderr, "%s: QueryInterface failed\n", program_name);
	  return (3);
	}
    }
  else
    {
      free (buf_str);
      fprintf (stderr, "%s: CoCreateInstance failed\n", program_name);
      return (3);
    }
}

static const char * getVersion()
{
  return versionID;
}

static void printTopDescription(FILE * f, char * name)
{
  char s[20];
  fprintf(f, "%s is part of cygutils version %s\n", name, getVersion());
  fprintf(f, "  create a Windows shortcut\n\n");
}
static void printBottomDescription(FILE * f, char * name)
{
  fprintf(f, "\nNOTE: All filename arguments must be in unix (POSIX) format\n");
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
  fprintf(f, copyrightID);
}

static void license(poptContext optCon, FILE * f, char * name)
{
  printTopDescription(f, name);
  printLicense(f, name);
}  

