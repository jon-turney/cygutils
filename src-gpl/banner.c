/*
banner - make posters
Copyright (C) 1999-2001 Joerg Schaible

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation in version 2 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "common.h"

static const char versionID[] = 
    "1.0";
/* for future CVS */
static const char revID[] =
    "$Id$";
static const char copyrightID[] =
    "Copyright (c) 1999-2001\nJoerg Schaible. All rights reserved.\nLicensed under GPL v2.0\n";

static void printTopDescription(FILE * f, char * name);
static void printBottomDescription(FILE * f, char * name);
static const char * getVersion(void);
static void usage(poptContext optCon, FILE * f, char * name);
static void help(poptContext optCon, FILE * f, char * name);
static void version(poptContext optCon, FILE * f, char * name);
static void license(poptContext optCon, FILE * f, char * name);

static char * program_name;

static char *c = "X";
static int w = 80;
static TEXTMETRIC tm;
static HFONT hFont;
static HBITMAP hBmp;
static HBITMAP hBmpOrg;
static HDC hMem;
static RECT rt;
static BITMAP bm;
static BITMAPINFO bmi;
static unsigned char *buffer;

void initialize()
{
	HDC hScreen;
	hFont = GetStockObject( ANSI_FIXED_FONT );
	hScreen = GetDC( 0 );
	hMem = CreateCompatibleDC( hScreen );
	ReleaseDC( 0, hScreen );

	SelectObject( hMem, hFont );
	GetTextMetrics( hMem, &tm );

	hBmp = CreateBitmap( w, tm.tmHeight, 1, 1, 0 );
	GetObject( hBmp, sizeof( bm ), &bm );
	memset( &bmi.bmiHeader, 0, sizeof( BITMAPINFOHEADER ));
	bmi.bmiHeader.biSize 	   = sizeof( BITMAPINFOHEADER );
	bmi.bmiHeader.biWidth	   = bm.bmWidth;
	bmi.bmiHeader.biHeight	   = bm.bmHeight;
	bmi.bmiHeader.biPlanes	   = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount    = 1;
	bmi.bmiHeader.biSizeImage   =
		  (((( unsigned int )bm.bmWidth + 31 ) & ~31 ) >> 3 )
		* bm.bmHeight;
	bmi.bmiHeader.biClrUsed	   = 2;
	buffer = ( unsigned char * )malloc( bmi.bmiHeader.biSizeImage );

	hBmpOrg = SelectObject( hMem, hBmp );
	SelectObject( hMem, GetStockObject( WHITE_BRUSH ));
	SelectObject( hMem, GetStockObject( WHITE_PEN ));

	SetBkColor( hMem, RGB( 0, 0, 0 ));
	SetTextColor( hMem, RGB( 255, 255, 255 ));
	SetBkMode( hMem, OPAQUE );

	rt.top = 0;
	rt.left = 0;
	rt.right = w;
	rt.bottom = tm.tmHeight;
}

void deinitialize()
{
	SelectObject( hMem, hBmpOrg );
	DeleteObject( hBmp );
	DeleteDC( hMem );
	free( buffer );
}

int print_line( const char *line )
{
	int x, y;
	int len = strlen( line );
	if( len > ( w / tm.tmMaxCharWidth ))
		len = ( w / tm.tmMaxCharWidth );

	Rectangle( hMem, 0, 0, rt.right, rt.bottom );
	DrawText( hMem, line, len, &rt,
			  DT_NOCLIP | DT_NOPREFIX | DT_SINGLELINE );

	SelectObject( hMem, hBmpOrg );
	GetDIBits( hMem, hBmp, 0, bmi.bmiHeader.biHeight, buffer,
			   ( LPBITMAPINFO )&bmi.bmiHeader, DIB_RGB_COLORS );
	SelectObject( hMem, hBmp );

	for( y = tm.tmHeight; y--; )
	{
		unsigned char *line = buffer + y * ( bmi.bmiHeader.biSizeImage / bmi.bmiHeader.biHeight );

		for( x = 0; x < len * tm.tmMaxCharWidth; ++x )
		{
			unsigned char byte = *( line + ( x / 8 ));
			putchar(( byte & ( 1 << (7 - (( int )x % 8 )))) ? *c : ' ' );
		}

		putchar( '\n' );
	}
	return 0;
}

int main(int argc, const char ** argv)
{
  poptContext optCon;
  const char ** rest;
  int rc;
  int ec = 0;
  int i;

  struct poptOption generalOptionsTable[] = {
    { "char",  'c',  POPT_ARG_STRING, &c, 'c', \
        "use character <X>", "X"},
    { "width", 'w', POPT_ARG_INT, &w, 'w', \
        "set display width to <80> ", "80"},
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
        "General options", NULL },
    { NULL, '\0', POPT_ARG_INCLUDE_TABLE, helpOptionsTable, 0, \
        "Help options", NULL },
    { NULL, '\0', 0, NULL, 0, NULL, NULL }
  };

  if( (program_name = strdup(argv[0])) == NULL ) {
    fprintf(stderr, "%s: memory allocation error\n", argv[0]);
    exit(1);
  }
  optCon = poptGetContext(NULL, argc, argv, opt, 0);
  poptSetOtherOptionHelp(optCon, "A string to print...");

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
      case 'c': /* no additional action needed */ 
                 break;
      case 'w': /* no additional action needed */ 
                 break;
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

  if (rest == NULL) {
    fprintf(stderr, "%s: not enough arguments\n",program_name);
    usage(optCon, stderr, program_name);
  }
  else {
  	initialize();
    while (*rest && !ec) {
      ec |= print_line( *rest );
		rest++;
	 }
    deinitialize();
  }

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
  fprintf(f, "  Prints a string enlarged as a banner on the screen.\n\n");
}

static void printBottomDescription(FILE * f, char * name)
{
  fprintf(f, "\nThis version works the same way as System V'S banner does:\n");
  fprintf(f, "The banner text is displayed horizontally.\n");
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

