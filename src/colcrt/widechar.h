/*
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
#ifndef WIDECHAR_H
#define WIDECHAR_H
/* Declarations for wide characters */
/* This file must be included last because the redefinition of wchar_t may
   cause conflicts when system include files were included after it. */
#if HAVE_CONFIG_H
#  include "config.h"  /* for ENABLE_WIDECHAR */
#endif
/* don't include "common.h" here */

#ifdef ENABLE_WIDECHAR

# include <wchar.h>
# include <wctype.h>
#if 0 /* for testing on platforms without built-in wide character support */
#  include <libutf8.h>
#endif

#if 1
/* explicit prototypes, since sometimes <wchar.h> does not give them */
extern int wcwidth (wint_t c);
extern int wcswidth (const wchar_t *s, size_t n);
extern size_t wcslen (const wchar_t *s);
extern wchar_t *wcsdup (const wchar_t *s);
#endif

#else

# include <ctype.h>
  /* Fallback for types */
# define wchar_t char
# define wint_t int
# define WEOF EOF
  /* Fallback for input operations */
# define fgetwc fgetc
# define getwc getc
# define getwchar getchar
# define fgetws fgets
  /* Fallback for output operations */
# define fputwc fputc
# define putwc putc
# define putwchar putchar
# define fputws fputs
  /* Fallback for character classification */
# define iswgraph isgraph
# define iswprint isprint
# define iswspace isspace
  /* Fallback for string functions */
# define wcschr strchr
# define wcsdup strdup
# define wcslen strlen

# define wcwidth(c) 1

#endif
#endif
