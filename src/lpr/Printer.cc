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

#include "src/lpr/Printer.hh"
#include "src/lpr/Win32Utils.hh"
using namespace std;

inline LPTSTR tstr(const string & str)
{
  return const_cast<char*>(str.c_str());
}

Printer::Printer(const string &name, bool debugFlag) throw(PrinterException)
    : m_devName(name),
    m_debugFlag(debugFlag),
    m_devHandle(INVALID_HANDLE_VALUE),
    m_devMode(0),
    m_buffer(0),
    m_bufferSize(4096),
    m_bufferIndex(0),
    m_rawFlag(false)
{
  m_buffer = new unsigned char[m_bufferSize];
  mapPortName();

  open();

  // Initialize the DEVMODE structure
  // Get the amount of space needed for the buffer.
  DWORD needed = DocumentProperties(NULL, m_devHandle, tstr(m_devName),
                                    NULL, NULL, 0);

  m_devMode = reinterpret_cast<DEVMODE *>(new unsigned char[needed]);

  // Get the default DEVMODE for the printer.
  DWORD ret = DocumentProperties(NULL, m_devHandle, tstr(m_devName),
                                 m_devMode, NULL, DM_OUT_BUFFER);
  if (ret != IDOK)
    throw PrinterException("error getting the default device mode: " +
                           Win32Utils::getErrorMessage());

  // Close the printer, because we'll need to reopen it later, passing it
  // the DEVMODE structure with any optional changes by the user.
  close();
}

Printer::~Printer()
{
  close();
  if (m_buffer != 0)
    delete [] m_buffer;
  if (m_devMode != 0)
    delete m_devMode;
}

void Printer::close()
{
  if (m_devHandle != INVALID_HANDLE_VALUE)
    {
      ClosePrinter(m_devHandle);
      m_devHandle = INVALID_HANDLE_VALUE;
    }
}

void Printer::endDoc() throw(PrinterException)
{
  if (!EndDocPrinter(m_devHandle))
    throw PrinterException("EndDocPrinter error: " +
                           Win32Utils::getErrorMessage());
}

void Printer::endPage() throw(PrinterException)
{
  cout << "Printer::endPage()" << endl;
  if (!EndPagePrinter(m_devHandle))
    throw PrinterException("EndPagePrinter error: " +
                           Win32Utils::getErrorMessage());
}

void Printer::flush(void) throw(PrinterException)
{
  if (m_bufferIndex > 0)
    {
      write(m_buffer, m_bufferIndex);
      m_bufferIndex = 0;
    }
}

bool Printer::getRawFlag() const
{
  return m_rawFlag;
}

void Printer::print(istream & in, const string & docName) throw(PrinterException)
{
  startDoc(docName);

  char ch;
  char lastCh = '\0';
  while (in.get(ch))
    {
      if (!m_rawFlag && ch == '\n' && lastCh != '\r')
        put('\r');
      put(ch);
      lastCh = ch;
    }
  flush();
  endDoc();
}

void Printer::put(unsigned char ch) throw(PrinterException)
{
  if (m_bufferIndex == m_bufferSize)
    flush();
  m_buffer[m_bufferIndex++] = ch;
}

void Printer::setRawFlag(bool flag)
{
  m_rawFlag = flag;
}

void Printer::startDoc(const string & docName) throw(PrinterException)
{
  DOC_INFO_1    di1;

  open();

  di1.pDocName = tstr(docName);
  di1.pOutputFile = 0;
  di1.pDatatype = "RAW";

  if (StartDocPrinter(m_devHandle, 1, (LPBYTE) &di1) == 0)
    throw PrinterException("StartDocPrinter error: " +
                           Win32Utils::getErrorMessage());
}

void Printer::startPage() throw(PrinterException)
{
  cout << "Printer::startPage()" << endl;
  if (!StartPagePrinter(m_devHandle))
    throw PrinterException("StartPagePrinter error: " +
                           Win32Utils::getErrorMessage());
}

void Printer::write(unsigned char *buf, unsigned int size) throw(PrinterException)
{
  DWORD written;
  if (!WritePrinter(m_devHandle, buf, size, &written))
    throw PrinterException("error writing to device '" + m_devName + "': " +
                           Win32Utils::getErrorMessage());
}

//***************************************************************************
//
// Private operations
//
//***************************************************************************

Printer::PrinterList Printer::enumPrinters(DWORD flags, LPTSTR name) throw(PrinterException)
{
  DWORD numPrinters;
  DWORD bytesNeeded;
  EnumPrinters(flags, name, 5, 0, 0, &bytesNeeded, &numPrinters);
  auto_ptr<BYTE> enumBuffer(new BYTE[bytesNeeded]);
  if (!EnumPrinters(flags, name, 5, enumBuffer.get(), bytesNeeded,
                    &bytesNeeded, &numPrinters))
    throw PrinterException("unable to enumerate printers: " +
                           Win32Utils::getErrorMessage());

  PRINTER_INFO_5 *pi = reinterpret_cast<PRINTER_INFO_5 *>(enumBuffer.get());

  PrinterList list;
  for (unsigned int ii = 0; ii < numPrinters; ++ii)
    {
      if (m_debugFlag)
        cerr << "Printer name: '" << pi->pPrinterName << "', "
             << "Port name: '" << pi->pPortName << "'" << endl;
      list.push_back(*pi++);
    }

  return list;
}

void Printer::mapPortName() throw(PrinterException)
{

  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&osvi);
  bool isWindows9x = (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
                      (osvi.dwMajorVersion > 4 ||
                       (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion > 0)));

  bool isWindows2K_NT4 = (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT &&
                          osvi.dwMajorVersion >= 4);

  if (m_debugFlag)
    cout << "isWindows9x = " << isWindows9x << ", "
         << "isWindows2K_NT4 = " << isWindows2K_NT4 << endl;


  if (isWindows9x)
    {
      PrinterList list = enumPrinters(PRINTER_ENUM_NAME, "");
      for (PrinterList::iterator it = list.begin(); it != list.end(); ++it)
        {
          if (stricmp(m_devName.c_str(), it->pPortName) == 0)
            {
              if (m_debugFlag)
                cout << "Mapped '" << m_devName << "' to '" << it->pPrinterName
                     << "'" << endl;
              m_devName = it->pPrinterName;
              return;
            }
        }
    }

  else if (isWindows2K_NT4)
    {
      PrinterList list = enumPrinters(PRINTER_ENUM_LOCAL, 0);
      for (PrinterList::iterator it = list.begin(); it != list.end(); ++it)
        {
          if (stricmp(m_devName.c_str(), it->pPortName) == 0)
            {
              if (m_debugFlag)
                cout << "Mapped '" << m_devName << "' to '" << it->pPrinterName
                     << "'" << endl;
              m_devName = it->pPrinterName;
              return;
            }
        }

      list = enumPrinters(PRINTER_ENUM_CONNECTIONS, 0);
      for (PrinterList::iterator it = list.begin(); it != list.end(); ++it)
        {
          if (stricmp(m_devName.c_str(), it->pPortName) == 0)
            {
              if (m_debugFlag)
                cout << "Mapped '" << m_devName << "' to '" << it->pPrinterName
                     << "'" << endl;
              m_devName = it->pPrinterName;
              return;
            }
        }
    }
}

void Printer::open() throw(PrinterException)
{
  PRINTER_DEFAULTS prDef;

  prDef.pDatatype = "RAW";
  prDef.pDevMode = m_devMode;
  prDef.DesiredAccess = PRINTER_ACCESS_USE;

  if (m_devHandle == INVALID_HANDLE_VALUE &&
      !OpenPrinter(tstr(m_devName), &m_devHandle, &prDef))
    throw PrinterException("can't open '" + m_devName + "' for writing: " +
                           Win32Utils::getErrorMessage());
}
