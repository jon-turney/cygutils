/**
 * conv.exe '\n' convertor 0.1.3
 *   based on hd2c 0.5.12 by Peter Hanecak (made 17.1.2001)
 *     Copyright 1997,.. by Peter Hanecak <hanecak@megaloman.com>.
 *     All rights reserved.
 *   based on Unix2Dos 0.9.0 by Peter Hanecak (made 19.2.1997)
 *     Copyright 1997,.. by Peter Hanecak <hanecak@megaloman.com>.
 *     All rights reserved.
 * Copyright 2001,.. by Charles Wilson <cwilson@ece.gatech.edu>.
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

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include "common.h"

static const char versionID[] = 
    "0.1.3";
/* for future CVS */
static const char revID[] =
    "$Id$";
static const char copyrightID[] =
    "Copyright (c) 2005\nCharles S. Wilson. All rights reserved.\nLicensed under GPL v2.0\n";

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

#define SM_SAFE         0
#define SM_FORCE        1

typedef struct Opt_ {
  char * progname;
  int    ConvType;
  int    SafeMode;
} Opt;

static void printTopDescription(FILE * f, char * name);
static void printBottomDescription(FILE * f, char * name);
static const char * getVersion(void);
static void usage(poptContext optCon, FILE * f, char * name);
static void help(poptContext optCon, FILE * f, char * name);
static void version(poptContext optCon, FILE * f, char * name);
static void license(poptContext optCon, FILE * f, char * name);
static int basename(char* p, const char* s);
static int convert(const char *fn, Opt opt);
static int exitOnZero(const char *fn, Opt opts, 
                      FILE* in, FILE* out, char* tempFn);

int main(int argc, const char ** argv) {
  poptContext optCon;
  const char ** rest;
  Opt opts = {
    NULL,
    CT_AUTO,
    SM_SAFE
  };
  int safeFlag = 0;
  int forceFlag = 0;
  int u2dFlag = 0;
  int d2uFlag = 0;
  int autoFlag = 0;
  int convFlags = 0;
  int rc;
  int ec = 0;
  int xargc = 0;
  int progtype;

  struct poptOption generalOptionsTable[] = {
    { PARAM_CT_AUTO, PARAM_CT_AUTO_SHORT, \
        POPT_ARG_NONE, &autoFlag, CT_AUTO, \
	"Output format will be the opposite of the autodetected source format", NULL },
    { PARAM_CT_UNIX2DOS_S, PARAM_CT_UNIX2DOS_SHORT, \
        POPT_ARG_NONE, &u2dFlag, CT_UNIX2DOS, \
	"Output will be in DOS format", NULL },
    { PARAM_CT_UNIX2DOS_L, '\0', \
        POPT_ARG_NONE, &u2dFlag, CT_UNIX2DOS, \
	"Output will be in DOS format", NULL },
    { PARAM_CT_DOS2UNIX_S, PARAM_CT_DOS2UNIX_SHORT, \
        POPT_ARG_NONE, &d2uFlag, CT_DOS2UNIX, \
	"Output will be in UNIX format", NULL },
    { PARAM_CT_DOS2UNIX_L, '\0', \
        POPT_ARG_NONE, &d2uFlag, CT_DOS2UNIX, \
	"Output will be in UNIX format", NULL },
    { "force", '\0', POPT_ARG_NONE, &forceFlag, 'f',
	"Ignore binary file detection", NULL },
    { "safe", '\0', POPT_ARG_NONE, &safeFlag, 's',
	"Do not modify binary files", NULL },
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

  if( (opts.progname = strdup(argv[0])) == NULL ) {
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(1);
  }
  basename(opts.progname, argv[0]);
  // set defaults based on program name
  if( strcasecmp(opts.progname, UNIX2DOS_NAME_S) == 0 ) {
    opts.ConvType = CT_UNIX2DOS;
    progtype = CT_UNIX2DOS;
  }
  else if ( strcasecmp(opts.progname, UNIX2DOS_NAME_L) == 0 ) {
    opts.ConvType = CT_UNIX2DOS;
    progtype = CT_UNIX2DOS;
  }
  else if ( strcasecmp(opts.progname, DOS2UNIX_NAME_S) == 0 ) {
    opts.ConvType = CT_DOS2UNIX;
    progtype = CT_DOS2UNIX;
  }
  else if ( strcasecmp(opts.progname, DOS2UNIX_NAME_L) == 0 ) {
    opts.ConvType = CT_DOS2UNIX;
    progtype = CT_DOS2UNIX;
  }
  else
    progtype = CT_AUTO;
  
  optCon = poptGetContext(NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp(optCon, "[OPTION...] [input file list...]");

  while ((rc = poptGetNextOpt(optCon)) > 0) {
    switch (rc) {
      case '?':  help(optCon, stderr, opts.progname);
                 goto exit;
      case 'u':  usage(optCon, stderr, opts.progname);
                 goto exit;
      case 'v':  version(optCon, stderr, opts.progname);
                 goto exit;
      case 'l':  license(optCon, stderr, opts.progname);
                 goto exit;
    }
  }
  if (rc < -1 ) {
    fprintf(stderr, "%s: bad argument %s: %s\n",
      opts.progname, poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
      poptStrerror(rc));
    ec = 2;
    goto exit;
  }
  rest = poptGetArgs(optCon);

  convFlags = (u2dFlag << 2) | (d2uFlag << 1) | autoFlag;
  if ((convFlags == 7) || (convFlags == 6) || (convFlags == 5) || (convFlags == 3))
  {
    fprintf(stderr,
            "%s: Only one congruent set of options allowed:\n"
            "(--%s, --%s, -%c), (--%s, --%s, -%c), (--%s, -%c)\n",
            opts.progname,
            PARAM_CT_UNIX2DOS_S, PARAM_CT_UNIX2DOS_L, PARAM_CT_UNIX2DOS_SHORT,
            PARAM_CT_DOS2UNIX_S, PARAM_CT_DOS2UNIX_L, PARAM_CT_DOS2UNIX_SHORT,
            PARAM_CT_AUTO, PARAM_CT_AUTO_SHORT);
    ec = 3;
    goto exit;
  }
  if (d2uFlag) opts.ConvType = CT_DOS2UNIX;
  if (u2dFlag) opts.ConvType = CT_UNIX2DOS;
  if (autoFlag) opts.ConvType = CT_AUTO;

  // check that there is no conflict between program's name and options
  if (progtype == CT_UNIX2DOS) {
    if (opts.ConvType != CT_UNIX2DOS) {
      fprintf(stderr, 
              "%s: cannot accept any conversion type argument other\n" \
              "  than --%s (--%s, -%c) when the program is called with this name\n", \
              opts.progname, PARAM_CT_UNIX2DOS_S, PARAM_CT_UNIX2DOS_L, \
              PARAM_CT_UNIX2DOS_SHORT);
      ec = 3;
      goto exit;
    }
  }
  if (progtype == CT_DOS2UNIX) {
    if (opts.ConvType != CT_DOS2UNIX) {
      fprintf(stderr,
              "%s: cannot accept any conversion type argument other\n" \
              "  than --%s (--%s, -%c) when this program is called with this name\n", \
              opts.progname, PARAM_CT_DOS2UNIX_S, PARAM_CT_DOS2UNIX_L, \
              PARAM_CT_DOS2UNIX_SHORT);
      ec = 3;
      goto exit;
    }
  }
  if ((safeFlag == 1) && (forceFlag == 1))
  {
    fprintf(stderr,
            "%s: Warning, both --force and --safe specified.  Using --safe.\n",
            opts.progname);
    opts.SafeMode = SM_SAFE;
  }
  else 
  { 
    if (safeFlag == 1) opts.SafeMode = SM_SAFE;
    if (forceFlag == 1) opts.SafeMode = SM_FORCE;
  }

  if (rest) 
    while (*rest) {
      if ((ec = convert(*rest++, opts)) < 0)
        break;
    }
  else 
    ec = convert(NULL, opts);

exit:
  poptFreeContext(optCon);
  free(opts.progname);
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
  
//  0 : continue
//  1 : error, skip to next file
// <0 : error, exit immediately
static int exitOnZero(const char *fn, Opt opts,
                      FILE* in, FILE* out, char* tempFn)
{
  char buf[PATH_MAX * 2];

  // We may be dealing with a binary file 
  // if not using stdin, and not --safe mode, warn and bail
  if ((fn != NULL) && (opts.SafeMode == SM_SAFE)) {
    snprintf(buf, PATH_MAX*2 - 1, 
             "\n%s: skipping binary file %s...", opts.progname, fn);
    buf[PATH_MAX*2 - 1] = '\0';
    fputs(buf, stderr);

    if (fclose(in) < 0) {
      perror(opts.progname);
      return -2;
    }
    if (fclose(out) < 0) {
      perror(opts.progname);
      return -2;
    }
    if (remove(tempFn) < 0) {
      perror(opts.progname);
      return -2;
    }
    return 1;
  }
  return 0;
}

// if fn is NULL then input is stdin and output is stdout
static int convert(const char *fn, Opt opts) {
  int c;
  int fileChanged = 0;
  char *tempFn;
  FILE *in = stdin, *out = stdout;
  char buf[PATH_MAX * 2];

  if (fn != NULL) {
    int inFileDesc = 0;
    fprintf(stderr, "%s: ", fn);
    if ((in = fopen(fn, "rb")) == NULL) {
      snprintf(buf, PATH_MAX*2 - 1, "\n%s processing %s", opts.progname, fn);
      perror(buf);
      return -1;
    }
    tempFn = tmpnam(NULL);
    if ((out = fopen(tempFn, "wb")) == NULL) {
      snprintf(buf, PATH_MAX*2 - 1, "\n%s processing %s", opts.progname, fn);
      perror(buf);
      return -2;
    }
  }
  else {
    setmode(0, O_BINARY);
    setmode(1, O_BINARY);
  }

  while ((c = fgetc(in)) != EOF) {

    if (c == '\0') {
      int a = exitOnZero(fn, opts, in, out, tempFn);
      if (a < 0) return a;
      if (a > 0) goto convert_ret;
      // otherwise, write out the '\0' and continue
      fputc('\0', out);
      continue;
    }

    if (c == '\r') {
      if (opts.ConvType == CT_AUTO) {
        opts.ConvType = CT_DOS2UNIX;
      }
      if (opts.ConvType == CT_DOS2UNIX) {
        // check next char
        int c2;
        // eat all extra '\r'
        while ((c2 = fgetc(in)) == '\r') {
          fileChanged = 1;
        }
        if (c2 == EOF) {
          // file ended on a '\r'.  Finish the line and quit loop.
          fputc('\n', out);
          fileChanged = 1;
          c = c2;
          break;
        } else if (c2 == '\0') {
          int a = exitOnZero(fn, opts, in, out, tempFn);
          if (a < 0) return a;
          if (a > 0) goto convert_ret;
          // otherwise, finish the line, promote c2, and fall off bottom of loop
          fputc('\n', out);
          fileChanged = 1;
          c = c2;
        } else if (c2 == '\n') {
          // '\r' followed by '\n'.  Promote c2, and fall off 
          fileChanged = 1;
          c = c2;
        } else {
          // '\r' followed by something else. Write out '\n', promote c2, and fall off
          fputc('\n', out);
          fileChanged = 1;
          c = c2;
        }
        fputc(c, out);
        continue;
      } // CT_DOS2UNIX

      if (opts.ConvType == CT_UNIX2DOS) {
        // check next char
        int c2;
        // eat all extra '\r'
        while ((c2 = fgetc(in)) == '\r') {
          fileChanged = 1;
        }
        if (c2 == EOF) {
          // file ended on a '\r'.  Finish the line and quit loop.
          fputc('\r', out);
          fputc('\n', out);
          fileChanged = 1;
          c = c2;
          break;
        } else if (c2 == '\0') {
          int a = exitOnZero(fn, opts, in, out, tempFn);
          if (a < 0) return a;
          if (a > 0) goto convert_ret;
          // otherwise, finish the line, promote c2, and fall off bottom of loop
          fputc('\r', out);
          fputc('\n', out);
          fileChanged = 1;
          c = c2;
        } else if (c2 == '\n') {
          // '\r' followed by '\n'.  Write out '\r', promote c2, and fall off 
          fputc('\r', out);
          c = c2;
        } else {
          // '\r' followed by something else. Write out '\r\n', promote c2, and fall off
          fputc('\r', out);
          fputc('\n', out);
          fileChanged = 1;
          c = c2;
        }
        fputc(c, out);
        continue;
      } // CT_UNIX2DOS
    }   // '\r'

    if (c == '\n') {
      // guaranteed that this is NOT preceeded by a '\r'
      if (opts.ConvType == CT_AUTO)
        opts.ConvType = CT_UNIX2DOS;
      if (opts.ConvType == CT_UNIX2DOS) {
        fileChanged = 1;
        fputc('\r', out);
      }
      fputc('\n', out);
      continue;
    } // '\n'

    // c not '\r', '\n', or '\0'
    fputc(c, out);
  }


  if (fn != NULL) {
    if (fclose(in) < 0) {
      perror(opts.progname);
      return -2;
    }
    if (fclose(out) < 0) {
      perror(opts.progname);
      return -2;
    }
    if (fileChanged != 0)
    {
      if ((in = fopen(tempFn, "rb")) == NULL) {
        perror(opts.progname);
        return -1;
      }
      if ((out = fopen(fn, "wb")) == NULL) {
        perror(fn);
        return -2;
      }

      while ((c = fgetc(in)) != EOF)
        fputc(c, out);

      if (fclose(in) < 0) {
        perror(opts.progname);
        return -2;
      }
      if (fclose(out) < 0) {
        perror(opts.progname);
        return -2;
      }
    }

    if (remove(tempFn) < 0) {
      perror(opts.progname);
      return -2;
    }
  }

convert_ret:
  if (fn != NULL)
    fprintf(stderr, "done.\n");
  return 0;
}


