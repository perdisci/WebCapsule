/*
 * WebCapsule.cpp
 *
 *  Created on: Dec 13, 2014
 *      Author: roberto.perdisci 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/OwnPtr.h"
#include "core/inspector/WebCapsule.h"

#if OS(ANDROID)
#include <android/log.h>
#endif


namespace WebCore {

// static
WebCapsule::LogLevel WebCapsule::s_globalLogLevel = WebCapsule::DebugLogLevel; // Default logging level

// static
void WebCapsule::setGlobalLogLevel(WebCapsule::LogLevel level) {
	s_globalLogLevel = level;
}

// static
void WebCapsule::log(WebCapsule::LogLevel level, const char* format, ...) {

	if(level > s_globalLogLevel) // pring only if level <= m_globalLogLevel
		return;

        va_list args;
        va_start(args, format);
        print_with_prefix(WEBCAPSULE_DEFAULT_LOG_PREFIX_STR, format, args);
        va_end(args);
}


// static
void WebCapsule::print_with_prefix(const char* prefix, const char* format, va_list args)
{
    #define WEBCAPSULE_END_OF_LINE "\n"

    size_t prefixLength = strlen(prefix);
    size_t formatLength = strlen(format);
    size_t eolLength = strlen(WEBCAPSULE_END_OF_LINE);

    OwnPtr<char[]> formatWithPrefix = adoptArrayPtr(new char[prefixLength + formatLength + eolLength + 1]);
    memcpy(formatWithPrefix.get(), prefix, prefixLength);
    memcpy(formatWithPrefix.get() + prefixLength, format, formatLength);
    memcpy(formatWithPrefix.get() + prefixLength + formatLength, WEBCAPSULE_END_OF_LINE, eolLength);
    formatWithPrefix[prefixLength + formatLength + eolLength] = 0;

    #if OS(ANDROID)
    	__android_log_vprint(ANDROID_LOG_WARN, "WebKit", formatWithPrefix.get(), args);
    #endif
    vfprintf(stderr, formatWithPrefix.get(), args);
}

} // namespace WebCapsule

