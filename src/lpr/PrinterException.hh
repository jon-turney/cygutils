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

#if !defined(PrinterException_hh)
#define PrinterException_hh

#include <iostream>
#include <string>

using namespace std;

class PrinterException
{
public:
  PrinterException()
    : m_msg("")
  {
  }
  
  PrinterException(const string & msg)
    : m_msg(msg)
  {
  }

  PrinterException(const PrinterException & rhs)
    : m_msg(rhs.m_msg)
  {
  }

  ~PrinterException()
  {
  }

  PrinterException & operator = (const PrinterException & rhs)
  {
    if (this != &rhs)
      m_msg = rhs.m_msg;
    return *this;
  }
  
  const string & getMessage() const
  {
    return m_msg;
  }

private:
  string m_msg;
};

inline ostream & operator << (ostream & stream, const PrinterException & pe)
{
  stream << pe.getMessage();
  return stream;
}

#endif /* PrinterException_hh */
