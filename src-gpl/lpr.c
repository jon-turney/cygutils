/* 
 * lpr for cygwin/windows
 *
 * Copyright (C) 2000-2002 Rick Rankin
 * http://www.cygwin.com/ml/cygwin/2000-07/msg00320.html
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

#if HAVE_CONFIG_H
#  include "config.h"
#endif

#include "common.h"
/* moved into common.h */
/*
#include <getopt.h>
*/

static const char versionID[] =
    "0.1.1";
/* for CVS */
static const char revID[] =
    "$Id$";
static const char copyrightID[] =
    "Copyright (c) 2001,...\nRick Rankin. All rights reserved.\nLicensed under GPL v2.0\n";

static char * program_name;
char          deviceName[256] = "prn";
HANDLE        deviceHandle;
unsigned char buffer[1024];
int           bufferIndex;

int error(int sys, const char *control, ...)
{
  va_list ap;

  fprintf(stderr, "%s: ", program_name);
    
  VA_START (ap, control);

  vfprintf(stderr, control, ap);
  va_end(ap);

  fputc('\n', stderr);
  
  if (sys)
  {
    LPVOID msgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, GetLastError(),
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &msgBuf, 0, NULL);
    fprintf(stderr, "%s: %s\n", program_name, (char *) msgBuf);
    /* Free the buffer. */
    LocalFree(msgBuf);
  }

  return 1;
}

int flushBuf(void)
{
  int exitCode = 0;

  if (bufferIndex > 0)
  {
    DWORD written;
    if (!WritePrinter(deviceHandle, buffer, bufferIndex, &written))
      exitCode = error(1, "error writing to %s", deviceName);
    bufferIndex = 0;
  }

  return exitCode;
}

void initBuf(void)
{
  bufferIndex = 0;
}

int putBuf(unsigned char ch)
{
  int exitCode = 0;
  
  if (bufferIndex == sizeof(buffer))
    exitCode = flushBuf();
  buffer[bufferIndex++] = ch;
  
  return exitCode;
}

int lpr(FILE *inFile, LPTSTR docName)
{
  int           exitCode = 0;
  unsigned char ch;
  unsigned char lastCh = '\0';
  DOC_INFO_1    di1;

  di1.pDocName = docName;
  di1.pOutputFile = "";
  di1.pDatatype = "raw";

  if (StartDocPrinter(deviceHandle, 1, (LPBYTE) &di1) == 0)
    exitCode = error(1, "StartDocPrinter error");
  else
  {
    initBuf();
    while (exitCode == 0 && !feof(inFile))
    {
      ch = fgetc(inFile);
      if (ch == '\n' && lastCh != '\r')
        exitCode = putBuf('\r');
      exitCode = putBuf(ch);
      lastCh = ch;
    }
    flushBuf();
    if (!EndDocPrinter(deviceHandle))
      exitCode = error(1, "EndDocPrinter error");
  }
  
  return exitCode;
}

int main(int argc, char *argv[])
{
  int     optionChar;
  char *  envPtr;
  FILE *  inputFile;
  int     i;
  int     exitCode = 0;
  
  if( (program_name = strdup(argv[0])) == NULL ) {
    /* can't use error() here, because error used program_name */
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(1);
  }

  if ((envPtr = getenv("PRINTER")) != NULL)
  {
    strncpy(deviceName, envPtr, sizeof(deviceName));
    deviceName[sizeof(deviceName)-1] = '\0';
  }
    
  while ((optionChar = getopt(argc, argv, "hP:")) != EOF)
  {
    switch (optionChar)
    {
    case 'h':
      /* accept for compatibility */
      break;
    case 'P':
      strncpy(deviceName, optarg, sizeof(deviceName));
      deviceName[sizeof(deviceName)-1] = '\0';
      break;
    default:
      return error(0, "unknown option: '%c'", optionChar);
    }
  }

  if (!OpenPrinter(deviceName, &deviceHandle, NULL))
    exitCode = error(1, "can't open '%s' for writing", deviceName);
  else
  {
    if (optind >= argc)
      exitCode = lpr(stdin, "stdin");
    else
    {
      for (i = optind; i < argc && exitCode == 0; ++i)
      {
        if ((inputFile = fopen(argv[i], "r")) == NULL)
          exitCode = error(1, "can't open '%s' for input",argv[i]);
        else
        {
          exitCode = lpr(inputFile, argv[i]);
          fclose(inputFile);
        }
      }
    }
    if (!ClosePrinter(deviceHandle))
      exitCode = error(1, "error closing printer");
  }

  return exitCode;
}
