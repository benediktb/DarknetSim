#ifndef DEBUG_H_
#define DEBUG_H_

// Put #define USE_DEBUG in _Debug.h -- or not.
#include "./_Debug.h"

#include <sstream>

// Idea by http://rootdirectory.de/wiki/SSTR%28%29
#define S(x) ((std::ostringstream&) (std::ostringstream() << std::dec << x)).str()
#define CS(x) S(x).c_str()

#ifdef USE_DEBUG
#define DEBUG(x) (EV << x)
#else
#define DEBUG(x)
#endif


#endif /* DEBUG_H_ */
