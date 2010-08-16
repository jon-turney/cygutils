/**
 * ascii.c ASCII table display
 *
 * Copyright 2001,2002,2009 by Charles Wilson
 * All rights reserved.
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

static const char versionID[] = PACKAGE_VERSION;
static const char revID[] =
  "$Id$";
static const char copyrightID[] =
  "Copyright (c) 2010\nCharles S. Wilson. All rights reserved.\nLicensed under GPLv3+\n";

static void printTopDescription (FILE * f, char *name);
static const char *getVersion (void);
static void usage (FILE * f, char *name);
static void help (FILE * f, char *name);
static void version (FILE * f, char *name);
static void license (FILE * f, char *name);
static void ascii (FILE * f, int opt_extended);

static char *program_name;
static poptContext optCon;

int
main (int argc, const char **argv)
{
  const char **rest;
  int rc;
  int ec = 0;
  int opt_extended = 0;

  struct poptOption generalOptionsTable[] = {
    {"extended", 'e', POPT_ARG_NONE, NULL, 'e',
     "Include \"hi-ascii\" characters (128..255).", NULL},
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
        case 'e':
          opt_extended=1;
          break;
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
  if (rc < -1)
    {
      fprintf (stderr, "%s: bad argument %s: %s\n",
               program_name, poptBadOption (optCon, POPT_BADOPTION_NOALIAS),
               poptStrerror (rc));
      ec = 2;
      goto exit;
    }
  rest = poptGetArgs (optCon);

  if (rest == NULL)
    ascii (stdout, opt_extended);
  else
    {
      fprintf (stderr, "Extra args ignored: ");
      while (*rest)
        fprintf (stderr, "%s ", *rest++);
      fprintf (stderr, "\n");
      ascii (stdout, opt_extended);
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
  fprintf (f,
           "  Prints nicely formatted table of the ascii character set\n\n");
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

static void
ascii (FILE * f, int opt_extended)
{
  char str1[64][80];
  int i, j;
  int c;

  for (i = 0; i < 64; i++)
    str1[i][0] = '\0';

  for (i = 0; i < 128; i++)
    {
      j = i % 32;
      sprintf (str1[j], "%s%03d  %0#4x  ", str1[j], i, i);
      if (i < 32)
        sprintf (str1[j], "%s^%c\t", str1[j], i + 64);
      else
        sprintf (str1[j], "%s%c\t", str1[j], i);
    }
  if (opt_extended)
    {
      for (i = 128; i < 256; i++)
        {
          j = ((i - 128) % 32) + 32;
          sprintf (str1[j], "%s%03d  %0#4x  ", str1[j], i, i);
          sprintf (str1[j], "%s%c\t", str1[j], i);
        }
    }
  for (i = 0; i < 32; i++)
    fprintf (f, "%s\n", str1[i]);

  if (opt_extended)
    {
      fprintf (f, "\n");
      for (i = 32; i < 64; i++)
        fprintf (f, "%s\n", str1[i]);
    }
}

