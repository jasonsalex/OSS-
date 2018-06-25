{
  'include_dirs': [
    ".",
    "include",
	"lib",
	"include/curl",
	"include/openssl",
	"include/qiniu"
  ],
  
  'sources': [  
	"include/qiniu/base.h",
    "include/qiniu/cdn.h",
    "include/qiniu/conf.h",
    "include/qiniu/emu_posix.h",
    "include/qiniu/fop.h",
    "include/qiniu/http.h",
    "include/qiniu/io.h",
    "include/qiniu/macro.h",
    "include/qiniu/qetag.h",
    "include/qiniu/reader.h",
    "include/qiniu/resumable_io.h",
    "include/qiniu/rs.h",
    "include/qiniu/rsf.h",
    "include/qiniu/tm.h"
  ]
}
