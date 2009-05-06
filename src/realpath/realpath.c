/* realpath - return the canonicalized absolute pathname
 * Copyright (C) 2001 Steven Barker
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

/* Modified 2002-01-26 Charles Wilson:
 *   -- use popt for option handling
 *   -- remove gettext dependency
 */

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include "common.h"

/* The official name of this program (e.g., no `g' prefix).  */
#define PROGRAM_NAME "realpath"
#define AUTHORS "Steven Barker"

static const char versionID[] = PACKAGE_VERSION;
static const char revID[] =
	"$Id$";
static const char copyrightID[] =
	"Copyright (c) 2001,...\nSteven Barker. All rights reserved.\nLicensed under GPL v2.0\n";

/* The name this program was run with. */
static char * program_name;

static void printTopDescription(FILE * f, char * name);
static void printBottomDescription(FILE * f, char * name);
static const char * getVersion(void);
static void usage(poptContext optCon, FILE * f, char * name);
static void help(poptContext optCon, FILE * f, char * name);
static void version(poptContext optCon, FILE * f, char * name);
static void license(poptContext optCon, FILE * f, char * name);

int main(int argc, const char ** argv)
{
  poptContext optCon;
  const char ** rest;
  int rc;
  int ec = 0;
  char *input;
  char *result;

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
    { NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0, \
        "Help options", NULL },
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };

  if( (program_name = strdup(argv[0])) == NULL ) {
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(1);
  }
  if( (result = malloc(PATH_MAX)) == NULL ) {
    fprintf(stderr, "%s: memory allocation error\n", program_name);
    free(program_name);
	 exit(1);
  }
   
  optCon = poptGetContext(NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp(optCon, "[OPTIONS...] PATH");

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
    }
  }
  if (rc < -1 ) {
    fprintf(stderr, "%s: bad argument %s: %s\n",
      program_name, poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
      poptStrerror(rc));
    ec = 2;
    goto exit;
  }
  rest = poptGetArgs(optCon);

  if (rest == NULL) {
    fprintf(stderr, "%s: not enough arguments\n",program_name);
    usage(optCon, stderr, program_name);
  }
  else {
    if( (input = strdup(*rest++)) == NULL ) {
      fprintf(stderr, "%s: memory allocation error\n", program_name);
      ec = 1;
      goto exit;
    }
    if (*rest != NULL) { /* should only have ONE member of **rest  */
	   fprintf(stderr, "%s: too many arguments\n", program_name);
      usage(optCon, stderr, program_name);
      free(input);
    }
    else {
      if(! realpath (input, result)) {
        ec = errno;
        perror(program_name);
        free(input);
        goto exit;
      }
      printf ("%s\n", result);
      free(input);
    }
  }

exit:
  poptFreeContext(optCon);
  free(program_name);
  free(result);
  return(ec);
}

static const char * getVersion()
{
  return versionID;
}

static void printTopDescription(FILE * f, char * name)
{
  fprintf(f, "%s is part of cygutils version %s\n", name, getVersion());
  fprintf(f, "%s was originally authored by %s\n", name, AUTHORS);
  fprintf(f, "  Print PATH with all references to `.', `..', and symlinks resolved.\n\n");
}
static void printBottomDescription(FILE * f, char * name)
{
/* fprintf(f, "\n");
 * fprintf(f, "Report bugs to <bug-sh-utils@gnu.org>.\n");
 */
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

