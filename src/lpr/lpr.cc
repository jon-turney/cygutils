/* 
 * lpr for cygwin/windows
 *
 * Copyright (C) 2000-2003 Rick Rankin
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

#include <fstream>
#include <string>

#include <sys/cygwin.h>

#include "src/lpr/Printer.hh"

using namespace std;

string progName;

string getProgramName(const string & str)
{
  string::size_type lastSlashPos = str.find_last_of("\\/");
  if (lastSlashPos == string::npos)
    lastSlashPos = 0;
  else
    ++lastSlashPos;
  string::size_type dotPos = str.find_last_of(".");
  if (dotPos == string::npos || dotPos < lastSlashPos)
    dotPos = str.length();
  return str.substr(lastSlashPos, dotPos - lastSlashPos);
}

const char * usageMessage =
" [-h] [-D] [-d device] [-P device]\n"
"\n"
"where:\n"
"\n"
"  -h        does nothing. Accepted for compatibility.\n"
"  -d device spools to the specified device.\n"
"  -D        enable debugging output.\n"
"  -P device spools to the specified device.\n"
"\n"
"Notes:\n"
"\n"
"-d and -P are aliases of each other and perform the same function.\n"
"Device names may take the form of DOS devices (e.g., lpt1:) if the printer\n"
"is connected locally. Network printers can be accessed using the form\n"
"'\\\\server\\printer_name'. Forward slashes can be used as well, e.g.,\n"
"'//server/printer_name'.\n"
"\n"
"The environment variable PRINTER can be used to set the default printer\n"
"device.\n";

void usage(const string & msg = "")
{
  if (msg != "")
    cerr << progName << ": " << msg << endl << endl;

  cerr << "Usage: " << progName << usageMessage;
  exit(1);
}

int main(int argc, char *argv[])
{
  progName = getProgramName(argv[0]);
  string printerName = "";

  // Is the printer specified in the environment?
  const char *p = getenv("PRINTER");
  if (p != 0)
    printerName = p;

  bool debugFlag = false;
  bool rawFlag = false;

  int optionChar;
  while ((optionChar = getopt(argc, argv, ":Dd:hlP:")) != EOF)
  {
    switch (optionChar)
    {
    case 'h':
      // accept for compatibility
      break;
    case 'D':
      debugFlag = true;
      break;
    case 'd':
    case 'P':
      printerName = optarg;
      break;
    case 'l':
      rawFlag = true;
      break;
    default:
      usage(string("unknown option: -") + char(optionChar));
    }
  }

  // Can't proceed without a printer name.
  if (printerName == "")
  {
    cerr << progName << ": no printer specified" << endl;
    return 1;
  }
  
  char winPrinter[MAX_PATH];
  cygwin_conv_to_win32_path(printerName.c_str(), winPrinter);
  if (debugFlag)
    cout << "Windows printer name = '" << winPrinter << "'" << endl;
  
  try
  {
    Printer pr(winPrinter, debugFlag);
    pr.setRawFlag(rawFlag);
    
    if (optind >= argc)
      pr.print(cin, "stdin");
    else
    {
      for (int ii = optind; ii < argc; ++ii)
      {
        ifstream in (argv[ii]);
        if (!in)
          cerr << progName << ": can't open '" << argv[ii] << "' for input."
               << endl;
        else
          pr.print(in, argv[ii]);
      }
    }
    pr.close();
  }
  catch (const PrinterException & ex)
  {
    cerr << progName << ": printer error: " << ex << endl;
    return 1;
  }

  return 0;
}
