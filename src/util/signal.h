#ifndef SMSC_UTIL_SIGNAL
#define SMSC_UTIL_SIGNAL

#include <signal.h>

namespace smsc {
namespace util {

/**
 * Sets signal handler for specified signal
 *
 * @param signo   Signal number to catch
 * @param handler pointer to static signal handler function
 * @return true, if signal handler sucessfully installed, false otherwise
 */
bool setSignalHandler(int signo,
											void (*handler)(int));

/**
 * Sets extended signal handler for specified signal
 *
 * @param signo  Signal number to catch
 * @param handler
 *               pointer to static extended signal handler function
 * @return true, if signal handler sucessfully installed, false otherwise
 */
bool setExtendedSignalHandler(int signo,
															void (*handler)(int, siginfo_t*, void*));

}
}
#endif // ifndef SMSC_UTIL_SIGNAL

