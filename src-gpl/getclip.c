/**
 * getclip.exe   Copy Windows clipboard to stdout  
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
static int getclip(FILE * out, flags_struct flags, FILE * f, char * name);

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
        "Output text will have DOS line endings.", NULL},
    { "unix", 'u', POPT_ARG_NONE, NULL, 'U', \
        "Output text will have UNIX line endings.", NULL},
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
    ec |= putclip(stdout, flags, stderr, program_name);
  else {
    fprintf(stderr, "Extra args ignored: ");
    while (*rest) 
      fprintf(stderr, "%s ", *rest++);
    fprintf(stderr, "\n");
    ec |= putclip(stdout, flags, stderr, program_name);
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
  fprintf(f, "  Copy the Windows Clipboard to stdout\n\n");
}

static void printBottomDescription(FILE * f, char * name)
{
  fprintf(f, "\nWARNING: --dos and --unix are not yet implemented.  Pipe output\n");
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

int putclip(FILE * out, flags_struct flags, FILE * f, char * name)
{
  HGLOBAL hglb;
  LPSTR lpstr;

  OpenClipboard(0);
  hglb = GetClipboardData(CF_TEXT);
  lpstr = GlobalLock(hglb);

  fprintf(out, "%s",lpstr);
  GlobalUnlock(hglb);
  CloseClipboard();
  return(0);
}
