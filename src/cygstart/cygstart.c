/*
 * cygstart - Let Windows start a program, or open a file or URL
 *
 * (c) 2002 Michael Schaap <cygwin_start@mscha.org>
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
#include "config.h"
#endif
#include "common.h"

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "cygstart"
#define AUTHORS "Michael Schaap"

/* Predefined actions */
#define ACTION_OPEN "open"
#define ACTION_EXPLORE "explore"
#define ACTION_EDIT "edit"
#define ACTION_FIND "find"
#define ACTION_PRINT "print"

/* MSDN reference URL */
#define MSDN_URL "http://msdn.microsoft.com/library/en-us/shellcc/platform/" \
                 "Shell/reference/functions/shellexecute.asp"

static const char versionID[] = "1.0";
/* for future CVS */
static const char revID[] =
	"$Id$";
static const char copyrightID[] =
	"Copyright (c) 2002,...\n"
        "Michael Schaap. All rights reserved.\n"
        "Licensed under GPL v2.0\n";

/* The name this program was run with. */
static char *program_name;

static int cygStart(const char *aPath, const char *action, const char *args,
                    const char *workDir, int show);
static int winStart(const char *aPath, const char *action, const char *args,
                    const char *workDir, int show);
static char *startError(int err);
static const char *getVersion(void);
static void printTopDescription(FILE *f, char *name);
static void printBottomDescription(FILE *f, char *name);
static void usage(poptContext optCon, FILE *f, char *name);
static void help(poptContext optCon, FILE *f, char *name);
static void version(poptContext optCon, FILE *f, char *name);
static void license(poptContext optCon, FILE *f, char *name);

int main(int argc, const char **argv)
{
    poptContext optCon;
    const char *arg;
    const char **rest;
    int rc;
    int ret;
    char *action = NULL;
    char *file = NULL;
    char *args = NULL;
    char *workDir = NULL;
    int show = SW_SHOWNORMAL;

    /* Action options */
    struct poptOption actionOptionsTable[] = {
        { "action",  'a',  POPT_ARG_STRING, NULL, 'a', \
          "Use specified action instead of default", NULL},
        { "open",  'o',  POPT_ARG_NONE, NULL, 'o', \
          "Short for: --action open", NULL},
        { "explore",  'x',  POPT_ARG_NONE, NULL, 'x', \
          "Short for: --action explore", NULL},
        { "edit",  'e',  POPT_ARG_NONE, NULL, 'e', \
          "Short for: --action edit", NULL},
        { "find",  'f',  POPT_ARG_NONE, NULL, 'f', \
          "Short for: --action find", NULL},
        { "print",  'p',  POPT_ARG_NONE, NULL, 'p', \
          "Short for: --action print", NULL},
        { NULL, '\0', 0, NULL, 0, NULL, NULL }
    };

    /* Directory options */
    struct poptOption directoryOptionsTable[] = {
        { "directory",  'd',  POPT_ARG_STRING, NULL, 'd', \
          "Set working directory", NULL},
        { NULL, '\0', 0, NULL, 0, NULL, NULL }
    };

    /* SHow options */
    struct poptOption showOptionsTable[] = {
        { "hide",  '\0',  POPT_ARG_NONE, NULL, 'H', \
          "Hides the window and activates another window", NULL},
        { "maximize",  '\0',  POPT_ARG_NONE, NULL, 'M', \
          "Maximizes the specified window", NULL},
        { "minimize",  '\0',  POPT_ARG_NONE, NULL, 'N', \
          "Minimizes the specified window and activates the next top-level "
          "window in the z-order", NULL},
        { "restore",  '\0',  POPT_ARG_NONE, NULL, 'R', \
          "Activates and displays the window. If the window is minimized or "
          "maximized, Windows restores it to its original size and position. "
          "An application should specify this flag when restoring a minimized "
          "window", NULL},
        { "show",  '\0',  POPT_ARG_NONE, NULL, 'S', \
          "Activates the window and displays it in its current size and "
          "position", NULL},
        { "showmaximized",  '\0',  POPT_ARG_NONE, NULL, 'X', \
          "Activates the window and displays it as a maximized window", NULL},
        { "showminimized",  '\0',  POPT_ARG_NONE, NULL, 'Y', \
          "Activates the window and displays it as a minimized window", NULL},
        { "showminnoactive",  '\0',  POPT_ARG_NONE, NULL, 'Z', \
          "Displays the window as a minimized window. The active window "
          "remains active", NULL},
        { "showna",  '\0',  POPT_ARG_NONE, NULL, 'A', \
          "Displays the window in its current state. The active window "
          "remains active", NULL},
        { "shownoactivate",  '\0',  POPT_ARG_NONE, NULL, 'V', \
          "Displays a window in its most recent size and position. The "
          "active window remains active", NULL},
        { "shownormal",  '\0',  POPT_ARG_NONE, NULL, 'O', \
          "Activates and displays a window. If the window is minimized or "
          "maximized, Windows restores it to its original size and position. "
          "An application should specify this flag when displaying the window "
          "for the first time", NULL},
        { NULL, '\0', 0, NULL, 0, NULL, NULL }
    };

    /* Help options */
    struct poptOption helpOptionsTable[] = {
        { "help",  '?',  POPT_ARG_NONE, NULL, '?', \
          "Show this help message", NULL},
        { "usage", '\0', POPT_ARG_NONE, NULL, 'u', \
          "Display brief usage message", NULL},
        { "version", '\0', POPT_ARG_NONE, NULL, 'v', \
          "Display version information", NULL},
        { "license", '\0', POPT_ARG_NONE, NULL, 'l', \
          "Display licensing information", NULL},
        { "reference", '\0', POPT_ARG_NONE, NULL, 'r', \
          "Open MSDN reference for ShellExecute", NULL},
        { NULL, '\0', 0, NULL, 0, NULL, NULL }
    };

    struct poptOption opt[] = {
        { NULL, '\0', POPT_ARG_INCLUDE_TABLE, actionOptionsTable, 0, \
          "Action options", NULL },
        { NULL, '\0', POPT_ARG_INCLUDE_TABLE, directoryOptionsTable, 0, \
          "Directory options", NULL },
        { NULL, '\0', POPT_ARG_INCLUDE_TABLE, showOptionsTable, 0, \
          "Show options", NULL },
        { NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0, \
          "Help options", NULL },
        { NULL, '\0', 0, NULL, 0, NULL, NULL }
    };

    if ((program_name = strdup(argv[0])) == NULL ) {
        fprintf(stderr, "%s: memory allocation error\n", argv[0]);
        exit(1);
    }

    /* Parse options */
    optCon = poptGetContext(NULL, argc, argv, opt, 0);
    poptSetOtherOptionHelp(optCon, "[OPTION]... FILE [ARGUMENTS]");
    while ((rc = poptGetNextOpt(optCon)) > 0) {
        switch (rc) {
            /* Help options */
            case '?':
                help(optCon, stdout, program_name);
                poptFreeContext(optCon);
                free(program_name);
                if (action)
                    free(action);
                if (workDir)
                    free(workDir);
                return(0);
            case 'u':
                usage(optCon, stdout, program_name);
                poptFreeContext(optCon);
                free(program_name);
                if (action)
                    free(action);
                if (workDir)
                    free(workDir);
                return(0);
            case 'v':
                version(optCon, stdout, program_name);
                poptFreeContext(optCon);
                free(program_name);
                if (action)
                    free(action);
                if (workDir)
                    free(workDir);
                return(0);
            case 'l':
                license(optCon, stdout, program_name);
                poptFreeContext(optCon);
                free(program_name);
                if (action)
                    free(action);
                if (workDir)
                    free(workDir);
                return(0);
            case 'r':
                cygStart(MSDN_URL, NULL, NULL, NULL, SW_NORMAL);
                poptFreeContext(optCon);
                free(program_name);
                if (action)
                    free(action);
                if (workDir)
                    free(workDir);
                return(0);

            /* Action options */
            case 'a':
                if (arg = poptGetOptArg(optCon)) {
                    if ((action = strdup(arg)) == NULL ) {
                        fprintf(stderr, "%s: memory allocation error\n",
                                                                    argv[0]);
                        exit(1);
                    }
                }
                break;
            case 'o':
                if ((action = strdup(ACTION_OPEN)) == NULL ) {
                    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
                    exit(1);
                }
                break;
            case 'x':
                if ((action = strdup(ACTION_EXPLORE)) == NULL ) {
                    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
                    exit(1);
                }
                break;
            case 'e':
                if ((action = strdup(ACTION_EDIT)) == NULL ) {
                    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
                    exit(1);
                }
                break;
            case 'f':
                if ((action = strdup(ACTION_FIND)) == NULL ) {
                    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
                    exit(1);
                }
                break;
            case 'p':
                if ((action = strdup(ACTION_PRINT)) == NULL ) {
                    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
                    exit(1);
                }
                break;

            /* Directory options */
            case 'd':
                if (arg = poptGetOptArg(optCon)) {
                    if ((workDir = strdup(arg)) == NULL ) {
                        fprintf(stderr, "%s: memory allocation error\n",
                                                                    argv[0]);
                        exit(1);
                    }
                }
                break;

            /* Show options */
            case 'H':
                show = SW_HIDE;
                break;
            case 'M':
                show = SW_MAXIMIZE;
                break;
            case 'N':
                show = SW_MINIMIZE;
                break;
            case 'R':
                show = SW_RESTORE;
                break;
            case 'S':
                show = SW_SHOW;
                break;
            case 'X':
                show = SW_SHOWMAXIMIZED;
                break;
            case 'Y':
                show = SW_SHOWMINIMIZED;
                break;
            case 'Z':
                show = SW_SHOWMINNOACTIVE;
                break;
            case 'A':
                show = SW_SHOWNA;
                break;
            case 'V':
                show = SW_SHOWNOACTIVATE;
                break;
            case 'O':
                show = SW_SHOWNORMAL;
                break;
        }
    }
    if (rc < -1 ) {
        fprintf(stderr, "%s: bad argument %s: %s\n",
                program_name, poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
                poptStrerror(rc));
        poptFreeContext(optCon);
        free(program_name);
        if (action)
            free(action);
        if (workDir)
            free(workDir);
        return(2);
    }
    rest = poptGetArgs(optCon);

    /* Determine file (or program, or URL) to start */
    if (rest && *rest) {
        if ((file = strdup(*rest)) == NULL ) {
            fprintf(stderr, "%s: memory allocation error\n", argv[0]);
            exit(1);
        }
        rest++;
    } else {
        usage(optCon, stdout, program_name);
        return(2);
    }

    /* Retrieve any arguments */
    if (rest && *rest) {
        if ((args = (char *) malloc(MAX_PATH+1)) == NULL) {
            fprintf(stderr, "%s: memory allocation error\n", argv[0]);
            exit(1);
        }
        strncpy(args, *rest, MAX_PATH);
        while (rest++ && *rest) {
            strncat(args, " ", MAX_PATH-strlen(args));
            strncat(args, *rest, MAX_PATH-strlen(args));
        }
    }

    /* Start it! */
    ret = cygStart(file, action, args, workDir, show);

    poptFreeContext(optCon);
    free(program_name);
    if (action)
        free(action);
    if (args)
        free(workDir);
    if (workDir)
        free(workDir);
    if (file)
        free(file);

    return (ret ? 0 : 1);
}

/* Start a program, or open a file or URL, using Cygwin POSIX paths */
static int cygStart(const char *aPath, const char *action, const char *args,
                    const char *workDir, int show)
{
    char winPath[MAX_PATH+1];
    char winDir[MAX_PATH+1];

    /* Convert file path from POSIX to Windows, unless it looks like a URL */
    if (!strstr(aPath, "://")) {
        cygwin_conv_to_win32_path(aPath, winPath);
    } else {
        strncpy(winPath, aPath, MAX_PATH);
    }

    /* Convert working directory, if any, from POSIX to Windows */
    if (workDir) {
        cygwin_conv_to_win32_path(workDir, winDir);
        return winStart(winPath, action, args, winDir, show);
    } else {
        return winStart(winPath, action, args, NULL, show);
    }
}

/* Start a program, or open a file or URL, using Windows paths */
static int winStart(const char *aPath, const char *action, const char *args,
                    const char *workDir, int show)
{
    int ret;

    ret = (int) ShellExecute(NULL, action, aPath, args, workDir, show);
    if (ret >= 32) {
        return TRUE;
    } else {
        printf("Unable to start '%s': %s\n", aPath, startError(ret));
        return FALSE;
    }
}

/* Return an error message, given a ShellExecute return code */
static char *startError(int err)
{
    switch (err) {
        case 0:
            return "The operating system is out of memory or resources.";
        case ERROR_FILE_NOT_FOUND:
            return "The specified file was not found.";
        case ERROR_PATH_NOT_FOUND:
            return "The specified path was not found.";
        case ERROR_BAD_FORMAT:
            return "The .exe file is invalid (non-Win32 .exe or error in "
                   ".exe image).";
        case SE_ERR_ACCESSDENIED:
            return "The operating system denied access to the specified file.";
        case SE_ERR_ASSOCINCOMPLETE:
            return "The file name association is incomplete or invalid.";
        case SE_ERR_DDEBUSY:
            return "The DDE transaction could not be completed because "
                   "other DDE transactions were being processed.";
        case SE_ERR_DDEFAIL:
            return "The DDE transaction failed.";
        case SE_ERR_DDETIMEOUT:
            return "The DDE transaction could not be completed because the "
                   "request timed out.";
        case SE_ERR_DLLNOTFOUND:
            return "The specified dynamic-link library was not found.";
        case SE_ERR_NOASSOC:
            return "There is no application associated with the given file "
                   "name extension.";
        case SE_ERR_OOM:
            return "There was not enough memory to complete the operation.";
        case SE_ERR_SHARE:
            return "A sharing violation occurred.";
        default:
            return "An unknown error occurred.";
    }
}

static const char *getVersion()
{
    return versionID;
}

static void printTopDescription(FILE *f, char *name)
{
    fprintf(f, "%s version %s, by %s\n", name, getVersion(), AUTHORS);
    fprintf(f, "\nLet Windows start a program or open a file or URL.\n\n");
}

static void printBottomDescription(FILE *f, char *name)
{
    fprintf(f, "\n");
    fprintf(f, "With thanks to MSDN: <%s>\n\n", MSDN_URL);
    fprintf(f, "Please report any bugs to <cygwin_start@mscha.org>.\n");
}

static printLicense(FILE *f, char *name)
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

static void usage(poptContext optCon, FILE *f, char *name)
{
  poptPrintUsage(optCon, f, 0);
}

static void help(poptContext optCon, FILE *f, char *name)
{
  printTopDescription(f, name);
  poptPrintHelp(optCon, f, 0);
  printBottomDescription(f, name);
}

static void version(poptContext optCon, FILE *f, char *name)
{
  printTopDescription(f, name);
}

static void license(poptContext optCon, FILE *f, char *name)
{
  printTopDescription(f, name);
  printLicense(f, name);
}  
