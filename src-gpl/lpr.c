/* 
 * lpr for cygwin/windows
 * 
 * written by Rick Rankin, published 2000-Jul-10
 * http://www.cygwin.com/ml/cygwin/2000-07/msg00320.html
 */

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>

char *        programName     = "lpr";
char          deviceName[256] = "prn";
HANDLE        deviceHandle;
unsigned char buffer[1024];
int           bufferIndex;

int error(int sys, const char *control, ...)
{
  va_list ap;

  fprintf(stderr, "%s: ", programName);
    
  va_start(ap, control);
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
    fprintf(stderr, "%s: %s\n", programName, (char *) msgBuf);
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
