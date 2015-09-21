/*
 * WebCapsule.h
 *
 *  Created on: Dec 13, 2014
 *      Author: roberto.perdisci 
 */

#ifndef WebCapsule_h
#define WebCapsule_h

#include <stdarg.h>

#define WEBCAPSULE_DEFAULT_LOG_PREFIX_STR "-- WEBCAPSULE -- "

namespace WebCore {
		
class WebCapsule {
public:

	enum LogLevel {
		QuietLogLevel,
		InfoLogLevel,
		DebugLogLevel,
		VerboseLogLevel,
		VeryVerboseLogLevel
	};
		
	static LogLevel s_globalLogLevel;

	static void setGlobalLogLevel(LogLevel level);
	static void log(LogLevel level, const char* format, ...);

private:

	static void print_with_prefix(const char* prefix, const char* format, va_list args);

}; // class WebCapsule

} // namespace WebCore

#endif /* WebCapsule_h */

