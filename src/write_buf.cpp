/*
   The MIT License (MIT)

   Copyright (c) 2014 Antonio SJ Musumeci <trapexit@spawn.link>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#if WRITE_BUF

#include <fuse.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "config.hpp"
#include "fileinfo.hpp"
#include "fs_movefile.hpp"
#include "policy.hpp"
#include "rwlock.hpp"
#include "ugid.hpp"
#include "write.hpp"

using std::string;
using std::vector;
using namespace mergerfs;

static
int
_write_buf(const int    fd,
           fuse_bufvec &src,
           const off_t  offset)
{
  size_t      size = fuse_buf_size(&src);
  fuse_bufvec dst  = FUSE_BUFVEC_INIT(size);
  const fuse_buf_copy_flags cpflags =
    (fuse_buf_copy_flags)(FUSE_BUF_SPLICE_MOVE|FUSE_BUF_SPLICE_NONBLOCK);

  dst.buf->flags = (fuse_buf_flags)(FUSE_BUF_IS_FD|FUSE_BUF_FD_SEEK);
  dst.buf->fd    = fd;
  dst.buf->pos   = offset;

  return fuse_buf_copy(&dst,&src,cpflags);
}

namespace mergerfs
{
  namespace fuse
  {
    int
    write_buf(const char     *fusepath,
              fuse_bufvec    *src,
              off_t           offset,
              fuse_file_info *ffi)
    {
      int rv;
      FileInfo *fi = reinterpret_cast<FileInfo*>(ffi->fh);

      rv = _write_buf(fi->fd,*src,offset);
      if(rv == -ENOSPC)
       {
          const fuse_context *fc     = fuse_get_context();
          const Config       &config = Config::get(fc);

          if(config.moveonenospc)
            {
              size_t extra;
              const ugid::Set         ugid(0,0);
              const rwlock::ReadGuard readlock(&config.srcmountslock);

              extra = fuse_buf_size(src);
              rv = fs::movefile(config.srcmounts,fusepath,extra,fi->fd);
              if(rv == -1)
                return -ENOSPC;

              rv = _write_buf(fi->fd,*src,offset);
            }
        }

      return rv;
    }
  }
}

#endif
