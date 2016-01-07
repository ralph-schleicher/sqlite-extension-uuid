/* sqlite3ext_uuid.c --- functions for universally unique identifiers.

   Copyright (C) 2015 Ralph Schleicher

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in
        the documentation and/or other materials provided with the
        distribution.

      * Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
   FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.  */

#undef USE_UTIL_LINUX
#undef USE_COCREATEGUID

#ifdef __linux__
#define USE_UTIL_LINUX 1
#else /* not __linux__ */
#ifdef _WIN32
#define USE_COCREATEGUID 1
#else /* not _WIN32 */
#endif /* not _WIN32 */
#endif /* not __linux__ */

#include "sqlite3ext.h"
SQLITE_EXTENSION_INIT1;

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if USE_UTIL_LINUX
#include <uuid/uuid.h>
#else /* not USE_UTIL_LINUX */
#if USE_COCREATEGUID
#include <windows.h>
#include <objbase.h>
#else /* not USE_COCREATEGUID */
#endif /* not USE_COCREATEGUID */
#endif /* not USE_UTIL_LINUX */

/* Utility function to convert a sequence of 16 octets
   into an UUID with 36 characters.  */
#define hi(octet) (((octet) & 0xF0U) >> 4)
#define lo(octet) (((octet) & 0x0FU) >> 0)

static char *
text_from_octets (char *text, unsigned char const *octet)
{
  static char const hex[16] =
    {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    };

  char *p = text;

  *p++ = hex[hi (octet[0])];
  *p++ = hex[lo (octet[0])];
  *p++ = hex[hi (octet[1])];
  *p++ = hex[lo (octet[1])];
  *p++ = hex[hi (octet[2])];
  *p++ = hex[lo (octet[2])];
  *p++ = hex[hi (octet[3])];
  *p++ = hex[lo (octet[3])];
  *p++ = '-';
  *p++ = hex[hi (octet[4])];
  *p++ = hex[lo (octet[4])];
  *p++ = hex[hi (octet[5])];
  *p++ = hex[lo (octet[5])];
  *p++ = '-';
  *p++ = hex[hi (octet[6])];
  *p++ = hex[lo (octet[6])];
  *p++ = hex[hi (octet[7])];
  *p++ = hex[lo (octet[7])];
  *p++ = '-';
  *p++ = hex[hi (octet[8])];
  *p++ = hex[lo (octet[8])];
  *p++ = hex[hi (octet[9])];
  *p++ = hex[lo (octet[9])];
  *p++ = '-';
  *p++ = hex[hi (octet[10])];
  *p++ = hex[lo (octet[10])];
  *p++ = hex[hi (octet[11])];
  *p++ = hex[lo (octet[11])];
  *p++ = hex[hi (octet[12])];
  *p++ = hex[lo (octet[12])];
  *p++ = hex[hi (octet[13])];
  *p++ = hex[lo (octet[13])];
  *p++ = hex[hi (octet[14])];
  *p++ = hex[lo (octet[14])];
  *p++ = hex[hi (octet[15])];
  *p++ = hex[lo (octet[15])];

  return text;
}

/* Utility function to convert an UUID with 36 characters
   into a sequence of 16 octets.  */
static unsigned char
hexval (int c)
{
  switch (c)
    {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'A': return 10;
    case 'B': return 11;
    case 'C': return 12;
    case 'D': return 13;
    case 'E': return 14;
    case 'F': return 15;
    case 'a': return 10;
    case 'b': return 11;
    case 'c': return 12;
    case 'd': return 13;
    case 'e': return 14;
    case 'f': return 15;
    }

  return UCHAR_MAX;
}

#define get_byte(k)							\
do									\
  {									\
    unsigned char hi, lo;						\
									\
    hi = hexval (*text);						\
    if (hi == UCHAR_MAX)						\
      return NULL;							\
									\
    ++text;								\
									\
    lo = hexval (*text);						\
    if (lo == UCHAR_MAX)						\
      return NULL;							\
									\
    ++text;								\
									\
    octet[k] = (hi << 4) + lo;						\
  }									\
while (0)

static void *
octets_from_text (unsigned char *octet, char const *text)
{
  get_byte (0);
  get_byte (1);
  get_byte (2);
  get_byte (3);

  if (*text++ != '-')
    return NULL;

  get_byte (4);
  get_byte (5);

  if (*text++ != '-')
    return NULL;

  get_byte (6);
  get_byte (7);

  if (*text++ != '-')
    return NULL;

  get_byte (8);
  get_byte (9);

  if (*text++ != '-')
    return NULL;

  get_byte (10);
  get_byte (11);
  get_byte (12);
  get_byte (13);
  get_byte (14);
  get_byte (15);

  return octet;
}

/* Create a random UUID as a sequence of 16 octets.  */
static void *
make_random_uuid (void *buffer)
{
#if USE_UTIL_LINUX

  uuid_t uuid;

  uuid_generate_random (uuid);
  memcpy (buffer, uuid, 16);

#else /* not USE_UTIL_LINUX */
#if USE_COCREATEGUID

  GUID uuid;
  char *from;
  char *to = buffer;

  if (CoCreateGuid (&uuid) != S_OK)
    return NULL;

  /* The first 8 hexadecimal digits of the GUID.  */
  from = (char *) &uuid.Data1;

  to[0] = from[3];
  to[1] = from[2];
  to[2] = from[1];
  to[3] = from[0];

  /* The first group of 4 hexadecimal digits.  */
  from = (char *) &uuid.Data2;

  to[4] = from[1];
  to[5] = from[0];

  /* The second group of 4 hexadecimal digits.  */
  from = (char *) &uuid.Data3;

  to[6] = from[1];
  to[7] = from[0];

  /* The first 2 bytes contain the third group of 4 hexadecimal digits.
     The remaining 6 bytes contain the final 12 hexadecimal digits.  */
  memcpy (to + 8, uuid.Data4, 8);

#else /* not USE_COCREATEGUID */

  sqlite3_randomness (16, buffer);

#endif /* not USE_COCREATEGUID */
#endif /* not USE_UTIL_LINUX */

  return buffer;
}

/* Signal an error in a SQL function and return back to the caller.

   If second argument MESSAGE is a null pointer, signal a memory
   error.  */
#define set_error_and_return(context,message)				\
do									\
  {									\
    char *s;								\
									\
    s = (message);							\
    if (s != NULL)							\
      sqlite3_result_error ((context), s, strlen (s));			\
    else								\
      sqlite3_result_error_nomem (context);				\
									\
    return;								\
  }									\
while (0)

/* Generate a random UUID.

   Return value is a BLOB with 16 bytes.  */
static void
sql_uuidgen (sqlite3_context *context, int val_count, sqlite3_value **val_vec)
{
  unsigned char octets[16];

  if (make_random_uuid (octets) == NULL)
    set_error_and_return (context, "failed to generate a random UUID");

  /* Return value.  */
  sqlite3_result_blob (context, octets, 16, SQLITE_TRANSIENT);
}

/* Convert an UUID from a BLOB into a string.  */
static void
sql_uuid (sqlite3_context *context, int val_count, sqlite3_value **val_vec)
{
  sqlite3_value *arg;
  void const *octets;
  char *text;

  /* Check argument.  */
  arg = val_vec[0];

  if (sqlite3_value_bytes (arg) != 16)
    set_error_and_return (context, "argument has to be a 16 byte object");

  /* Do the conversion.  */
  octets = sqlite3_value_blob (arg);
  if (octets == NULL)
    set_error_and_return (context, NULL);

  text = sqlite3_malloc (36);
  if (text == NULL)
    set_error_and_return (context, NULL);

  text_from_octets (text, octets);

  /* Return value.  */
  sqlite3_result_text (context, text, 36, sqlite3_free);
}

/* Convert an UUID from a string into a BLOB.  */
static void
sql_xuuid (sqlite3_context *context, int val_count, sqlite3_value **val_vec)
{
  sqlite3_value *arg;
  unsigned char octets[16];
  void const *text;

  /* Check argument.  */
  arg = val_vec[0];

  if (sqlite3_value_bytes (arg) != 36)
    set_error_and_return (context, "argument has to be a 36 byte object");

  /* Do the conversion.  */
  text = sqlite3_value_blob (arg);
  if (text == NULL)
    set_error_and_return (context, NULL);

  if (octets_from_text (octets, text) == NULL)
    set_error_and_return (context, "UUID parse error");

  /* Return value.  */
  sqlite3_result_blob (context, octets, 16, SQLITE_TRANSIENT);
}

/* Entry point.  */
int
sqlite3ext_uuid (sqlite3 *database, char **error, sqlite3_api_routines const *api)
{
  SQLITE_EXTENSION_INIT2 (api);

  sqlite3_create_function (database, "uuidgen", 0,
			   SQLITE_UTF8, NULL,
			   sql_uuidgen, NULL, NULL);
  sqlite3_create_function (database, "uuid", 1,
			   SQLITE_UTF8, NULL,
			   sql_uuid, NULL, NULL);
  sqlite3_create_function (database, "xuuid", 1,
			   SQLITE_UTF8, NULL,
			   sql_xuuid, NULL, NULL);

  return SQLITE_OK;
}
