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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <vector>

#include "fs_path.hpp"
#include "policy.hpp"

using std::string;
using std::vector;
using std::size_t;

static
int
_ffwp(const vector<string> &basepaths,
      const string         &fusepath,
      vector<string>       &paths)
{
  string fallback;

  for(size_t i = 0, ei = basepaths.size(); i != ei; i++)
    {
      int           rv;
      struct stat   st;
      string        fullpath;
      const string &basepath = basepaths[i];

      fs::path::make(basepath,fusepath,fullpath);

      rv = ::lstat(fullpath.c_str(),&st);
      if(rv == 0)
        {
          paths.push_back(basepath);
          return 0;
        }
      else if(errno == EACCES)
        {
          fallback = basepath;
        }
    }

  if(fallback.empty())
    return (errno=ENOENT,-1);

  paths.push_back(fallback);

  return 0;
}

namespace mergerfs
{
  int
  Policy::Func::ffwp(const Category::Enum::Type  type,
                     const vector<string>       &basepaths,
                     const string               &fusepath,
                     const size_t                minfreespace,
                     vector<string>             &paths)
  {
    return _ffwp(basepaths,fusepath,paths);
  }
}
