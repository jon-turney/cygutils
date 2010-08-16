/*
 * lpr for cygwin/windows
 *
 * Copyright (C) 2000-2003 Rick Rankin
 * http://www.cygwin.com/ml/cygwin/2000-07/msg00320.html
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

class Printer
{
public:
  Printer(const std::string &name, bool debugFlag) throw(PrinterException);
  ~Printer();

  void close();
  void endDoc() throw(PrinterException);
  void endPage() throw(PrinterException);
  void flush() throw(PrinterException);
  bool getRawFlag() const;
  void print(istream & in, const std::string & docName) throw(PrinterException);
  void put(unsigned char ch) throw(PrinterException);
  void setRawFlag(bool flag);
  void startDoc(const std::string & docName) throw(PrinterException);
  void startPage() throw(PrinterException);
  void write(unsigned char *buf, unsigned int size) throw(PrinterException);

private:

  typedef std::vector<PRINTER_INFO_5> PrinterList;

  // Don't want copy construction or assignment.
  Printer(const Printer & rhs);
  Printer & operator = (const Printer & rhs);

  // Enumerate the existing printers.
  PrinterList enumPrinters(DWORD flags, LPTSTR name) throw(PrinterException);

  // Map a port name (lpt1, etc) to a device name.
  void mapPortName() throw(PrinterException);

  // Open the printer if it's not already open.
  void open() throw(PrinterException);

  std::string     m_devName;
  bool            m_debugFlag;
  HANDLE          m_devHandle;
  DEVMODE *       m_devMode;
  unsigned char * m_buffer;
  unsigned int    m_bufferSize;
  unsigned int    m_bufferIndex;
  bool            m_rawFlag;
};

#endif /* Printer_hh */
