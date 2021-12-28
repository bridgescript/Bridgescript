#ifndef _VERSION_H_
#define _VERSION_H_

#define _Stringize(n)				#n
#define _MakeString(o,n)			o(n)
#define MakeString(n)				_MakeString( _Stringize, n )

#ifndef VER_FILEVERSION
	#define VER_FILEVERSION		VERSION_MAJOR, VERSION_MINOR, VERSION_3, VERSION_4
#endif

#ifndef VER_FILEVERSION_STR
	#define VER_FILEVERSION_STR	MakeString( VERSION_MAJOR ) "." MakeString( VERSION_MINOR ) "." MakeString (VERSION_3) "." MakeString (VERSION_4)
#endif

#endif //_VERSION_H_
