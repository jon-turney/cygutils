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

#include "src/lpr/Win32Utils.hh"
using namespace std;

string Win32Utils::getErrorMessage()
{
  return Win32Utils::getErrorMessage(GetLastError());
}

string Win32Utils::getErrorMessage(DWORD code)
{
  char * lpMsgBuf;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                code,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                (LPTSTR) &lpMsgBuf,
                0,
                NULL);

  string msg = lpMsgBuf;
  LocalFree(lpMsgBuf);

  return msg;
}

