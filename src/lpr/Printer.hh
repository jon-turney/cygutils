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

#if !defined(Printer_hh)
#define Printer_hh

#if HAVE_CONFIG_H
#  include "config.h"
#endif
#include "common.h"

#include <string>
#include <memory>
#include <vector>

#include "src/lpr/PrinterException.hh"

using namespace std;

class Printer
{
public:
  Printer(const string &name, bool debugFlag)
    throw(PrinterException);
  ~Printer();

  void close();
  void endDoc()
    throw(PrinterException);
  void endPage()
    throw(PrinterException);
  void flush()
    throw(PrinterException);
  void print(istream & in, const string & docName)
    throw(PrinterException);
  void put(unsigned char ch)
    throw(PrinterException);
  void startDoc(const string & docName)
    throw(PrinterException);
  void startPage()
    throw(PrinterException);
  void write(unsigned char *buf, unsigned int size)
    throw(PrinterException);
  
private:

  typedef vector<PRINTER_INFO_5> PrinterList;
  
  // Don't want copy construction or assignment.
  Printer(const Printer & rhs);
  Printer & operator = (const Printer & rhs);
  
  // Enumerate the existing printers.
  PrinterList enumPrinters(DWORD flags, LPTSTR name)
    throw(PrinterException);
  
  // Map a port name (lpt1, etc) to a device name.
  void mapPortName()
    throw(PrinterException);

  // Open the printer if it's not already open.
  void open()
    throw(PrinterException);
  
  string          m_devName;
  bool            m_debugFlag;
  HANDLE          m_devHandle;
  DEVMODE *       m_devMode;
  unsigned char * m_buffer;
  unsigned int    m_bufferSize;
  unsigned int    m_bufferIndex;
};

#endif /* Printer_hh */
