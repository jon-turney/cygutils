/**
 * conv.exe '\n' convertor 0.1.1
 *   based on hd2c 0.5.12 by Peter Hanecak (made 17.1.2001)
 *     Copyright 1997,.. by Peter Hanecak <hanecak@megaloman.com>.
 *     All rights reserved.
 *   based on Unix2Dos 0.9.0 by Peter Hanecak (made 19.2.1997)
 *     Copyright 1997,.. by Peter Hanecak <hanecak@megaloman.com>.
 *     All rights reserved.
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

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <limits.h>
#include <popt.h>

static const char versionID[] = 
    "0.1.2";
/* for future CVS */
static const char revID[] =
    "$Id$";
static const char copyrightID[] =
    "Copyright (c) 2001,...\nCharles S. Wilson. All rights reserved.\nLicensed under GPL v2.0\n";

#define UNIX2DOS_NAME_S		"u2d"
#define UNIX2DOS_NAME_L		"unix2dos"
#define DOS2UNIX_NAME_S		"d2u"
#define DOS2UNIX_NAME_L		"dos2unix"

#define PARAM_CT_AUTO		"auto"
#define PARAM_CT_UNIX2DOS_S	"u2d"
#define PARAM_CT_UNIX2DOS_L	"unix2dos"
#define PARAM_CT_DOS2UNIX_S	"d2u"
#define PARAM_CT_DOS2UNIX_L	"dos2unix"

#define PARAM_CT_AUTO_SHORT	'A'
#define PARAM_CT_UNIX2DOS_SHORT	'D'
#define PARAM_CT_DOS2UNIX_SHORT	'U'

#define CT_AUTO         0
#define CT_UNIX2DOS	1
#define CT_DOS2UNIX	2

static void printTopDescription(FILE * f, char * name);
static void printBottomDescription(FILE * f, char * name);
static const char * getVersion(void);
static void usage(poptContext optCon, FILE * f, char * name);
static void help(poptContext optCon, FILE * f, char * name);
static void version(poptContext optCon, FILE * f, char * name);
static void license(poptContext optCon, FILE * f, char * name);
static int basename(char* p, const char* s);
static int convert(const char *fn, int ConvType, char * progname);

int main(int argc, const char ** argv) {
  poptContext optCon;
  const char ** rest;
  int ConvType = CT_AUTO;
  int rc;
  int ec = 0;
  int xargc = 0;
  char * progname;
  int progtype;

  struct poptOption generalOptionsTable[] = {
    { PARAM_CT_AUTO, PARAM_CT_AUTO_SHORT, \
        POPT_ARG_VAL, NULL, CT_AUTO, \
	"Output format will be the opposite of the autodetected source format", NULL },
    { PARAM_CT_UNIX2DOS_S, PARAM_CT_UNIX2DOS_SHORT, \
        POPT_ARG_VAL, NULL, CT_UNIX2DOS, \
	"Output will be in DOS format", NULL },
    { PARAM_CT_UNIX2DOS_L, NULL, \
        POPT_ARG_VAL, NULL, CT_UNIX2DOS, \
	"Output will be in DOS format", NULL },
    { PARAM_CT_DOS2UNIX_S, PARAM_CT_DOS2UNIX_SHORT, \
        POPT_ARG_VAL, NULL, CT_DOS2UNIX, \
	"Output will be in UNIX format", NULL },
    { PARAM_CT_DOS2UNIX_L, NULL, \
        POPT_ARG_VAL, NULL, CT_DOS2UNIX, \
	"Output will be in UNIX format", NULL },
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
        "Main options (not all may apply)", NULL },
    { NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0, \
        "Help options", NULL },
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };

  generalOptionsTable[0].arg = &ConvType;
  generalOptionsTable[1].arg = &ConvType;
  generalOptionsTable[2].arg = &ConvType;
  generalOptionsTable[3].arg = &ConvType;
  generalOptionsTable[4].arg = &ConvType;

  if( (progname = strdup(argv[0])) == NULL ) {
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(1);
  }
  basename(progname, argv[0]);
  if( strcasecmp(progname, UNIX2DOS_NAME_S) == 0 ) {
    ConvType = CT_UNIX2DOS;
    progtype = CT_UNIX2DOS;
  }
  else if ( strcasecmp(progname, UNIX2DOS_NAME_L) == 0 ) {
    ConvType = CT_UNIX2DOS;
    progtype = CT_UNIX2DOS;
  }
  else if ( strcasecmp(progname, DOS2UNIX_NAME_S) == 0 ) {
    ConvType = CT_DOS2UNIX;
    progtype = CT_DOS2UNIX;
  }
  else if ( strcasecmp(progname, DOS2UNIX_NAME_L) == 0 ) {
    ConvType = CT_DOS2UNIX;
    progtype = CT_DOS2UNIX;
  }
  else
    progtype = CT_AUTO;
  
  optCon = poptGetContext(NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp(optCon, "[OPTION...] [input file list...]");

  while ((rc = poptGetNextOpt(optCon)) > 0) {
    switch (rc) {
      case '?':  help(optCon, stderr, progname);
                 goto exit;
      case 'u':  usage(optCon, stderr, progname);
                 goto exit;
      case 'v':  version(optCon, stderr, progname);
                 goto exit;
      case 'l':  license(optCon, stderr, progname);
                 goto exit;
    }
  }
  if (rc < -1 ) {
    fprintf(stderr, "%s: bad argument %s: %s\n",
      progname, poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
      poptStrerror(rc));
    ec = 2;
    goto exit;
  }
  rest = poptGetArgs(optCon);

  // check that there is no conflict between program's name and options
  if (progtype == CT_UNIX2DOS) {
    if (ConvType != CT_UNIX2DOS) {
      fprintf(stderr, "%s: cannot accept any conversion type argument other\n" \
                      "  than --%s (--%s, -%c) when the program is called with this name\n", \
                      progname, PARAM_CT_UNIX2DOS_S, PARAM_CT_UNIX2DOS_L, \
							 PARAM_CT_UNIX2DOS_SHORT);
      ec = 3;
      goto exit;
    }
  }
  if (progtype == CT_DOS2UNIX) {
    if (ConvType != CT_DOS2UNIX) {
      fprintf(stderr, "%s: cannot accept any conversion type argument other\n" \
                      "  than --%s (--%s, -%c) when this program is called with this name\n", \
                      progname, PARAM_CT_DOS2UNIX_S, PARAM_CT_DOS2UNIX_L, \
							 PARAM_CT_DOS2UNIX_SHORT);
      ec = 3;
      goto exit;
    }
  }
  if (rest) 
    while (*rest) {
      if ((ec = convert(*rest++, ConvType, progname)) < 0)
        break;
    }
  else 
    ec = convert(NULL, ConvType, progname);

exit:
  poptFreeContext(optCon);
  free(progname);
  return ec;
}
/* int basename(char* p, const char* s)
 * 
 * strip leading path names and a final ".exe" if they
 * exist.  Place the result in buffer p. Return the
 * length of p, or -1 if error.
 */
static int basename(char* p, const char* s)
{
   char* start;
   char* end;
   char* s1;
   s1 = strdup(s);
   // first, replace all \  with /
   while (start = strchr(s1, '\\')) 
     *start = '/';
   // then, locate the final /
   start = strrchr(s1, '/');
   if (!start)
     start = s1;
   else 
     start++;   // if s ends with /, then this points to '\0'
   end = &s1[strlen(s1)];  // this points to '\0'

   // the following assumes single byte char's
   if (( ((int) (end - start)) > 4 ) && // long enough to have .exe extension
       // second part not evaluated (short circuit) if string fragment too short
       (strcasecmp(end-4,".exe") == 0)) // end -4 > start, so we're okaya
   {
      end -= 4;
      *end = '\0';
   }
   strncpy(p, start, ((int) (end - start)) + 1);
   free(s1);
   return strlen(p);
}

static const char * getVersion()
{
  return versionID;
}

static void printTopDescription(FILE * f, char * name)
{
  if( ( strcasecmp(name, UNIX2DOS_NAME_S) == 0 ) ||
      ( strcasecmp(name, UNIX2DOS_NAME_L) == 0 ) ) {
    fprintf(f, "%s version %s\n", name, getVersion());
    fprintf(f, "  converts the line endings of text files from\n");
    fprintf(f, "  UNIX style (0x0a) to DOS style (0x0d 0x0a)\n\n");
  }
  else if ( ( strcasecmp(name, DOS2UNIX_NAME_S) == 0 ) ||
            ( strcasecmp(name, DOS2UNIX_NAME_L) == 0 ) ) {
    fprintf(f, "%s version %s\n", name, getVersion());
    fprintf(f, "  converts the line endings of text files from\n");
    fprintf(f, "  DOS style (0x0d 0x0a) to UNIX style (0x0a)\n\n");
  }
  else {
    fprintf(f, "%s version %s\n", name, getVersion());
    fprintf(f, "  converts the line endings of text files to/from\n");
    fprintf(f, "  DOS style (0x0d 0x0a) and UNIX style (0x0a)\n");
    fprintf(f, "  When no conversion options are specified, the input format\n");
    fprintf(f, "  will be automatically detected and converted to the opposite\n");
    fprintf(f, "  format on output\n\n");
  }
}

static void printBottomDescription(FILE * f, char * name)
{
  fprintf(f, "\n");
  fprintf(f, "Other arguments\n");
  fprintf(f, "  [input file list...]       for each file listed, convert in place.\n");
  fprintf(f, "                             If none specified, then use stdin/stdout\n");
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

// if fn is NULL then input is stdin and output is stdout
static int convert(const char *fn, int ConvType, char * progname) {
  char c;
  char *tempFn;
  FILE *in = stdin, *out = stdout;
  char buf[PATH_MAX * 2];
  if (fn != NULL) {
    fprintf(stderr, "%s: ", fn);
    if ((in = fopen(fn, "rb")) == NULL) {
      snprintf(buf, PATH_MAX*2 - 1, "\n%s processing %s", progname, fn);
      perror(buf);
      return -1;
    }
    tempFn = tmpnam(NULL);
    if ((out = fopen(tempFn, "wb")) == NULL) {
      snprintf(buf, PATH_MAX*2 - 1, "\n%s processing %s", progname, fn);
      perror(buf);
      return -2;
    }
  }
  else {
    setmode(0, O_BINARY);
    setmode(1, O_BINARY);
  }

  while ((c = fgetc(in)) != EOF) {
    if (c == '\r') {
      if ((ConvType == CT_UNIX2DOS) && (fn != NULL)) {
        // file is alredy in DOS format so it is not necessery to touch it
        if (fclose(in) < 0) {
          perror(progname);
          return -2;
        }
        if (fclose(out) < 0) {
          perror(progname);
          return -2;
        }
        if (remove(tempFn) < 0) {
          perror(progname);
          return -2;
        }
	goto convert_ret;
      }
      if (ConvType == CT_AUTO)
        ConvType = CT_DOS2UNIX;
      break;
    }
    if (c == '\n') {
      if ((ConvType == CT_DOS2UNIX) && (fn != NULL)) {
        // file is alredy in UNIX format so it is not necessery to touch it
        if (fclose(in) < 0) {
	  perror(progname);
          return -2;
        }
        if (fclose(out) < 0) {
          perror(progname);
          return -2;
        }
        if (remove(tempFn) < 0) {
          perror(progname);
          return -2;
        }
        goto convert_ret;
      }
      if (ConvType == CT_AUTO)
        ConvType = CT_UNIX2DOS;
      if (ConvType == CT_UNIX2DOS)
        fputc('\r', out);
      fputc('\n', out);
      break;
    }
    fputc(c, out);
  }
  if (c != EOF)
    while ((c = fgetc(in)) != EOF) {
      if (c == '\r')
        continue;
      if (c == '\n') {
        if (ConvType == CT_UNIX2DOS)
          fputc('\r', out);
	fputc('\n', out);
        continue;
      }
      fputc(c, out);
    }

  if (fn != NULL) {
    if (fclose(in) < 0) {
      perror(progname);
      return -2;
    }
    if (fclose(out) < 0) {
      perror(progname);
      return -2;
    }
    if ((in = fopen(tempFn, "rb")) == NULL) {
      perror(progname);
      return -1;
    }
    if ((out = fopen(fn, "wb")) == NULL) {
      perror(fn);
      return -2;
    }

    while ((c = fgetc(in)) != EOF)
      fputc(c, out);

    if (fclose(in) < 0) {
      perror(progname);
      return -2;
    }
    if (fclose(out) < 0) {
      perror(progname);
      return -2;
    }
    if (remove(tempFn) < 0) {
      perror(progname);
      return -2;
    }
  }
convert_ret:
  if (fn != NULL)
    fprintf(stderr, "done.\n");
  return 0;
}
