#include <paths.h>

#ifndef PATH_UTMP
#ifdef _PATH_UTMP
#define PATH_UTMP _PATH_UTMP
#endif
#endif

#ifndef PATH_UTMP
#ifdef UTMP_FILE
#define PATH_UTMP UTMP_FILE
#endif
#endif

#ifndef PATH_UTMP
/* Best default for systems old enough not to have a symbol */
#define PATH_UTMP "/etc/utmp"
#endif
