/* zip.c -- compress files to the gzip or pkzip format

   Copyright (C) 1997-1999, 2006-2007, 2009-2019 Free Software Foundation, Inc.
   Copyright (C) 1992-1993 Jean-loup Gailly

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#include <stdio.h>
#include <config.h>
#include <ctype.h>
#include <assert.h>
#include "tailor.h"
#include "gzip.h"
#include "zlib.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/errno.h>
#define CHUNK 16384

off_t header_bytes;   /* number of bytes in gzip header */

/* Speed options for the general purpose bit flag.  */
enum { SLOW = 2, FAST = 4 };

/* Deflate using zlib
 */
off_t
deflateGZIP (int pack_level)
{
  // source is input file descriptor, dest is input file descriptor
  int ret, flush;
  unsigned have;
  z_stream strm;
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];
  int source = ifd;
  int dest = ofd;

  /* allocate deflate state */
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  ret = deflateInit2 (&strm,
                      pack_level,
                      Z_DEFLATED,     // set this for deflation to work
                      MAX_WBITS + 16, // max window bits + 16 for gzip encoding
                      8,              // memlevel default
                      Z_DEFAULT_STRATEGY);
  if (ret != Z_OK)
    return ret;

  errno = 0;

  /* compress until end of file */
  do
    {
      strm.avail_in = read (source, in, CHUNK);
      if (errno != 0)
        {
          (void)deflateEnd (&strm);
          return Z_ERRNO;
        }

        if (rsync == true) {
            flush = (strm.avail_in != CHUNK) ? Z_FINISH : Z_FULL_FLUSH;
        } else {
            flush = (strm.avail_in != CHUNK) ? Z_FINISH : Z_NO_FLUSH;
        }
        strm.next_in = in;

        /* run deflate() on input until output buffer not full, finish
           compression if all of source has been read in */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);    /* no bad return value */
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            have = CHUNK - strm.avail_out;
            if (write(dest, out, have) != have || errno != 0) {
                (void)deflateEnd(&strm);
                return Z_ERRNO;
            }
        }
      while (strm.avail_out == 0);
      assert (strm.avail_in == 0);     /* all input will be used */

      /* done when last data in file processed */
    }
  while (flush != Z_FINISH);
  assert (ret == Z_STREAM_END);        /* stream will be complete */

  /* clean up and return */
  (void)deflateEnd (&strm);
  return Z_OK;
}

/* ===========================================================================
 * Deflate in to out.
 * IN assertions: the input and output buffers are cleared.
 *   The variables time_stamp and save_orig_name are initialized.
 */
int
zip (int in, int out)
{
    /* uch  flags = 0;         /1* general purpose bit flags *1/ */
    /* ush  attr = 0;          /1* ascii/binary flag *1/ */
    /* ush  deflate_flags = 0; /1* pkzip -es, -en or -ex equivalent *1/ */
    /* ulg  stamp; */

  ifd = in;
  ofd = out;

    /* outcnt = 0; */

    /* Write the header to the gzip file. See algorithm.doc for the format */

  method = DEFLATED;
    /* put_byte(GZIP_MAGIC[0]); /1* magic header *1/ */
    /* put_byte(GZIP_MAGIC[1]); */
    /* put_byte(DEFLATED);      /1* compression method *1/ */

    /* if (save_orig_name) { */
    /*     flags |= ORIG_NAME; */
    /* } */
    /* put_byte(flags);         /1* general flags *1/ */
    /* if (time_stamp.tv_nsec < 0) */
    /*   stamp = 0; */
    /* else if (0 < time_stamp.tv_sec && time_stamp.tv_sec <= 0xffffffff) */
    /*   stamp = time_stamp.tv_sec; */
    /* else */
    /*   { */
    /*     /1* It's intended that timestamp 0 generates this warning, */
    /*        since gzip format reserves 0 for something else.  *1/ */
    /*     warning ("file timestamp out of range for gzip format"); */
    /*     stamp = 0; */
    /*   } */
    /* put_long (stamp); */

    /* /1* Write deflated file to zip file *1/ */
    /* updcrc (NULL, 0); */

    /* bi_init(out); */
    /* ct_init(&attr, &method); */
    /* if (level == 1) */
    /*   deflate_flags |= FAST; */
    /* else if (level == 9) */
    /*   deflate_flags |= SLOW; */

    /* put_byte((uch)deflate_flags); /1* extra flags *1/ */
    /* put_byte(OS_CODE);            /1* OS identifier *1/ */

    /* if (save_orig_name) { */
    /*     char *p = gzip_base_name (ifname); /1* Don't save the directory part. *1/ */
    /*     do { */
    /*         put_byte (*p); */
    /*     } while (*p++); */
    /* } */
    /* header_bytes = (off_t)outcnt; */

#ifdef IBM_Z_DFLTCC
  dfltcc_deflate (level);
#else
  deflateGZIP (level);
#endif

#ifndef NO_SIZE_CHECK
  /* Check input size
   * (but not on MSDOS -- diet in TSR mode reports an incorrect file size)
   */
    /* if (ifile_size != -1L && bytes_in != ifile_size) { */
    /*     fprintf(stderr, "%s: %s: file size changed while zipping\n", */
    /*             program_name, ifname); */
    /* } */
#endif

    /* Write the crc and uncompressed size */
    /* put_long (getcrc ()); */
    /* put_long((ulg)bytes_in); */
    /* header_bytes += 2*4; */

    /* flush_outbuf(); */
  return OK;
}


/* ===========================================================================
 * Read a new buffer from the current input file, perform end-of-line
 * translation, and update the crc and input file size.
 * IN assertion: size >= 2 (for end-of-line translation)
 */
int
file_read (char *buf, unsigned size)
{
  unsigned len;

  Assert (insize == 0, "inbuf not empty");

  len = read_buffer (ifd, buf, size);
  if (len == 0)
    return (int)len;
  if (len == (unsigned)-1)
    read_error();

  updcrc ((uch *) buf, len);
  bytes_in += (off_t)len;
  return (int)len;
}
