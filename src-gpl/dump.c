/**
 * dump.exe   HEXDUMP utility
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

#include "config.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <popt.h>

static const char versionID[] = 
    "0.1.1";
/* for future CVS */
static const char revID[] =
    "$Id$";
static const char copyrightID[] =
    "Copyright (c) 2001,...\nCharles S. Wilson. All rights reserved.\nLicensed under GPL v2.0\n";

static void printTopDescription(FILE * f, char * name);
static void printBottomDescription(FILE * f, char * name);
static const char * getVersion(void);
static void usage(poptContext optCon, FILE * f, char * name);
static void help(poptContext optCon, FILE * f, char * name);
static void version(poptContext optCon, FILE * f, char * name);
static void license(poptContext optCon, FILE * f, char * name);
static void puthex(long n, int digits, int pos);
static void dumpfile(FILE *f);

static char * program_name;
static char line[80];
static long address;

int main(int argc, const char ** argv)
{
  poptContext optCon;
  const char ** rest;
  int rc;
  int ec = 0;

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
  optCon = poptGetContext(NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp(optCon, "[OPTION...] [files...]");
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

  if (rest == NULL)
    dumpfile(stdin);
  else {
    while (*rest) {
      FILE *f = fopen(*rest, "rb");
      printf("%s:\n",*rest);
      if (f) {
        dumpfile(f);
        fclose(f);
      }
      else
        printf("*** Can't open %s!!\n", *rest);
      rest++;
    }
  }
/*
  if (argc < 2) dumpfile(stdin);
  else {
    while (--argc > 0) {
      FILE *f = fopen(*++argv, "rb");
      printf("%s:\n",*argv);
      if (f) {
        dumpfile(f);
        fclose(f);
      }
      else printf("*** Can't open %s!!\n", *argv);
    }
  }
*/
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
  fprintf(f, "  Prints a hexdump of stdin or specified files to stdout\n\n");
}

static void printBottomDescription(FILE * f, char * name)
{
  fprintf(f, "\n");
  fprintf(f, "Other arguments\n");
  fprintf(f, "  [files...]      dump each file specified; if none, use stdin\n");
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

static void puthex(long n, int digits, int pos) {
  if (digits > 1) puthex(n/16,digits-1,pos);
  line[pos+digits-1] = "0123456789abcdef"[n%16];
}

static void dumpfile(FILE *f) {
  int c,i;
  address = 0;
  c=getc(f);
  while (1) {
    for (i=0;i<50;i++) line[i]=' ';
    for (;i<80;i++) line[i]=0;
    puthex(address,8,0);
    if (c == EOF) return;
    for (i=0;i<16;i++) {
      puthex(c & 0xff,2, 10 + i*2 + i/2);
      line[50+i] = '.';
      if (isprint(c & 0x7f)) line[50+i] = c & 0x7f;
      if ((c=getc(f)) == EOF) break;
    }
    if ((address % 256) == 0) {
#if defined(WIN32) && !defined(__CYGWIN__)
      while (kbhit())
        getch();
      getch();
#endif
      puts("");
      puts("  Addr     0 1  2 3  4 5  6 7  8 9  A B  C D  E F 0 2 4 6 8 A C E ");
      puts("--------  ---- ---- ---- ---- ---- ---- ---- ---- ----------------");
    }
    puts(line);
    address += 16;
  }
}
