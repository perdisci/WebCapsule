/*
 * ForensicPlatformWrapper.cpp
 *
 *  Created on: Jul 21, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicPlatformWrapper.h"
#include "core/inspector/InspectorForensicsAgent.h"
#include "core/inspector/ForensicWebClipboard.h"
#include "core/inspector/ForensicWebURLLoader.h"
#include "core/inspector/ForensicReplayDataStore.h"
#include "core/inspector/ForensicReplayDataRecorder.h"
#include "core/inspector/WebCapsule.h"
#include "platform/TraceEvent.h"
#include "wtf/MainThread.h"

namespace WebCore {

ForensicPlatformWrapper::ForensicPlatformWrapper(
		Platform* platform,
		PassOwnPtr<ForensicInspectorStateWrapper> state,
		PassRefPtr<ForensicReplayDataStore> dataStore,
		PassRefPtr<ForensicReplayDataRecorder> dataRecorder)
	: m_platform(platform)
{
	m_dataStore = dataStore;
	m_dataRecorder = dataRecorder;
	m_state = state;
}

ForensicPlatformWrapper::~ForensicPlatformWrapper()
{
	// TODO Auto-generated destructor stub
}


blink::WebCookieJar* ForensicPlatformWrapper::cookieJar()
{
	return m_platform->cookieJar();
}

blink::WebClipboard* ForensicPlatformWrapper::clipboard()
{
	if (m_state->replaying() || m_state->recording()) {
		return new ForensicWebClipboard(m_dataStore, m_dataRecorder, wrappedPlatform(), ForensicInspectorStateWrapper::create(*(m_state.get())), m_platform->clipboard());
	}
	return m_platform->clipboard();
}

blink::WebFileUtilities* ForensicPlatformWrapper::fileUtilities()
{
	return m_platform->fileUtilities();
}

blink::WebMimeRegistry* ForensicPlatformWrapper::mimeRegistry()
{
	return m_platform->mimeRegistry();
}

blink::WebSandboxSupport* ForensicPlatformWrapper::sandboxSupport()
{
	return m_platform->sandboxSupport();
}

blink::WebThemeEngine* ForensicPlatformWrapper::themeEngine()
{
	return m_platform->themeEngine();
}

blink::WebFallbackThemeEngine* ForensicPlatformWrapper::fallbackThemeEngine()
{
	return m_platform->fallbackThemeEngine();
}

blink::WebSpeechSynthesizer* ForensicPlatformWrapper::createSpeechSynthesizer(blink::WebSpeechSynthesizerClient* client)
{
	return m_platform->createSpeechSynthesizer(client);
}


// Audio --------------------------------------------------------------

double ForensicPlatformWrapper::audioHardwareSampleRate()
{
	return m_platform->audioHardwareSampleRate();
}

size_t ForensicPlatformWrapper::audioHardwareBufferSize()
{
	return m_platform->audioHardwareBufferSize();
}

unsigned ForensicPlatformWrapper::audioHardwareOutputChannels()
{
	return m_platform->audioHardwareOutputChannels();
}

blink::WebAudioDevice* ForensicPlatformWrapper::createAudioDevice(
		size_t bufferSize,
		unsigned numberOfInputChannels,
		unsigned numberOfChannels,
		double sampleRate,
		blink::WebAudioDevice::RenderCallback* callback,
		const blink::WebString& deviceId)
{
	return m_platform->createAudioDevice(bufferSize, numberOfInputChannels, numberOfChannels, sampleRate, callback, deviceId);
}


// MIDI ----------------------------------------------------------------

blink::WebMIDIAccessor* ForensicPlatformWrapper::createMIDIAccessor(blink::WebMIDIAccessorClient* client)
{
	return m_platform->createMIDIAccessor(client);
}

// Blob ----------------------------------------------------------------

blink::WebBlobRegistry* ForensicPlatformWrapper::blobRegistry()
{
	return m_platform->blobRegistry();
}

// Database ------------------------------------------------------------

blink::Platform::FileHandle ForensicPlatformWrapper::databaseOpenFile(const blink::WebString& vfsFileName, int desiredFlags)
{
	return m_platform->databaseOpenFile(vfsFileName, desiredFlags);
}

int ForensicPlatformWrapper::databaseDeleteFile(const blink::WebString& vfsFileName, bool syncDir)
{
	return m_platform->databaseDeleteFile(vfsFileName, syncDir);
}

long ForensicPlatformWrapper::databaseGetFileAttributes(const blink::WebString& vfsFileName)
{
	return m_platform->databaseGetFileAttributes(vfsFileName);
}

long long ForensicPlatformWrapper::databaseGetFileSize(const blink::WebString& vfsFileName)
{
	return m_platform->databaseGetFileSize(vfsFileName);
}

long long ForensicPlatformWrapper::databaseGetSpaceAvailableForOrigin(const blink::WebString& originIdentifier)
{
	return m_platform->databaseGetSpaceAvailableForOrigin(originIdentifier);
}

// DOM Storage --------------------------------------------------

blink::WebStorageNamespace* ForensicPlatformWrapper::createLocalStorageNamespace()
{
	return m_platform->createLocalStorageNamespace();
}


// FileSystem ----------------------------------------------------------

blink::WebFileSystem* ForensicPlatformWrapper::fileSystem()
{
	return m_platform->fileSystem();
}


// IDN conversion ------------------------------------------------------

blink::WebString ForensicPlatformWrapper::convertIDNToUnicode(const blink::WebString& host, const blink::WebString& languages)
{

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to convertIDNToUnicode from thread %u, is main thread %s, URL: %s Languages: %s", WTF::currentThread(), isMainThread() ? "true" : "false",  host.latin1().data(), languages.latin1().data());
	return m_platform->convertIDNToUnicode(host, languages);
}


// IndexedDB ----------------------------------------------------------

blink::WebIDBFactory* ForensicPlatformWrapper::idbFactory()
{
	return m_platform->idbFactory();
}


// Gamepad -------------------------------------------------------------

void ForensicPlatformWrapper::sampleGamepads(blink::WebGamepads& into)
{
	m_platform->sampleGamepads(into) ;
}

void ForensicPlatformWrapper::setGamepadListener(blink::WebGamepadListener* listener)
{
	m_platform->setGamepadListener(listener) ;
}


// History -------------------------------------------------------------

unsigned long long ForensicPlatformWrapper::visitedLinkHash(const char* canonicalURL, size_t length)
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::visitedLinkHash : Total");

	unsigned long long retval = 0;
	if(!m_state->replaying())
		retval = m_platform->visitedLinkHash(canonicalURL, length);

	// Notice that the TRACE_EVENTs only matter in recording mode...
	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::visitedLinkHash : Instrumentation");
	String key(canonicalURL, length);
	if(m_state->replaying()){
		WebCapsule::log(WebCapsule::VerboseLogLevel,"ForensicPlatformWrapper: Replaying call made to visitedLinkHash (hash=%d) %s", key.impl()->hash(), key.latin1().data());
	    TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::visitedLinkHash : Instrumentation");
		return m_dataStore->nextVisitedLinkHash(canonicalURL, length);
	}
	else if(m_state->recording()){
		WebCapsule::log(WebCapsule::VerboseLogLevel,"ForensicPlatformWrapper: Recording call made to visitedLinkHash (hash=%d) %s", key.impl()->hash(), key.latin1().data());
		m_dataRecorder->appendVisitedLinkHash(canonicalURL, length, retval);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::visitedLinkHash : Instrumentation");

	return retval;
}

bool ForensicPlatformWrapper::isLinkVisited(unsigned long long linkHash)
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::isLinkVisited : Total");

	bool retval = false;
	if(!m_state->replaying())
		retval = m_platform->isLinkVisited(linkHash);

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::isLinkVisited : Instrumentation");
	WebCapsule::log(WebCapsule::VerboseLogLevel,"ForensicPlatformWrapper: Call made to isLinkVisited.");
	if(m_state->recording()){
		m_dataRecorder->appendIsLinkVisited(linkHash, retval);
	}
	else if(m_state->replaying()){
	    TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::isLinkVisited : Instrumentation");
		return m_dataStore->nextIsLinkVisited(linkHash);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::isLinkVisited : Instrumentation");

	return retval;
}


// Keygen --------------------------------------------------------------

blink::WebString ForensicPlatformWrapper::signedPublicKeyAndChallengeString(unsigned keySizeIndex,
                                                    const blink::WebString& challenge,
                                                    const blink::WebURL& url)
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::signedPublicKeyAndChallengeString : Total");

	blink::WebString retval;
	if(!m_state->replaying())
		retval = m_platform->signedPublicKeyAndChallengeString(keySizeIndex, challenge, url);

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::signedPublicKeyAndChallengeString : Instrumentation");
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to signedPublicKeyAndChallengeString.");
	if(m_state->recording()){
		m_dataRecorder->appendSignedPublicKeyAndChallengeString(keySizeIndex, challenge, url, retval);
	}
	if(m_state->replaying()){
	    TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::signedPublicKeyAndChallengeString : Instrumentation");
		return m_dataStore->nextSignedPublicKeyAndChallengeString(keySizeIndex, challenge, url);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::signedPublicKeyAndChallengeString : Instrumentation");

	return retval;
}

// Memory --------------------------------------------------------------

size_t ForensicPlatformWrapper::memoryUsageMB()
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::memoryUsageMB : Total");

	size_t retval = 0;
	if(!m_state->replaying())
		retval = m_platform->memoryUsageMB();

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::memoryUsageMB : Instrumentation");
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to memoryUsageMB.");
	if(m_state->recording()){
		m_dataRecorder->appendMemoryUsageMB(retval);
	}
	if(m_state->replaying()){
		//return m_dataStore.nextMemoryUsageMB();
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::memoryUsageMB : Instrumentation");

	return retval;
}

size_t ForensicPlatformWrapper::actualMemoryUsageMB()
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::actualMemoryUsageMB : Total");

	size_t retval = 0;
	if(!m_state->replaying())
		retval = m_platform->memoryUsageMB();

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::actualMemoryUsageMB : Instrumentation");
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to actualMemoryUsageMB.");
	if(m_state->recording()){
		m_dataRecorder->appendActualMemoryUsageMB(retval);
	}
	if(m_state->replaying()){
		//return m_dataStore.nextActualMemoryUsageMB();
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::actualMemoryUsageMB : Instrumentation");

	return retval;
}

size_t ForensicPlatformWrapper::physicalMemoryMB()
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::physicalMemoryMB : Total");

	size_t retval = 0;
	if(!m_state->replaying())
		retval = m_platform->physicalMemoryMB();

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::physicalMemoryMB : Instrumentation");
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to physicalMemoryMB.");
	if(m_state->recording()){
		m_dataRecorder->appendPhysicalMemoryMB(retval);
	}
	if(m_state->replaying()){
		//return m_dataStore.nextPhysicalMemoryMB();
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::physicalMemoryMB : Instrumentation");

	return retval;
}

size_t ForensicPlatformWrapper::numberOfProcessors()
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::numberOfProcessors : Total");

	size_t retval = 0;
	if(m_state->replaying())
		retval = m_platform->numberOfProcessors();

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::numberOfProcessors : Instrumentation");
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to numberOfProcessors.");
	if(m_state->recording()){
		m_dataRecorder->appendNumberOfProcessors(retval);
	}
	if(m_state->replaying()){
		//return m_dataStore.nextNumberOfProcessors();
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::numberOfProcessors : Instrumentation");

	return m_platform->numberOfProcessors();
}

bool ForensicPlatformWrapper::processMemorySizesInBytes(size_t* privateBytes, size_t* sharedBytes)
{

	return m_platform->processMemorySizesInBytes(privateBytes, sharedBytes);
}

void ForensicPlatformWrapper::requestProcessMemorySizes(ProcessMemorySizesCallback* requestCallback)
{

	return m_platform->requestProcessMemorySizes(requestCallback) ;
}

bool ForensicPlatformWrapper::memoryAllocatorWasteInBytes(size_t* bytes)
{
	return m_platform->memoryAllocatorWasteInBytes(bytes) ;
}

blink::WebDiscardableMemory* ForensicPlatformWrapper::allocateAndLockDiscardableMemory(size_t bytes)
{
	return m_platform->allocateAndLockDiscardableMemory(bytes);
}

void ForensicPlatformWrapper::startHeapProfiling(const blink::WebString& prefix)
{
	m_platform->startHeapProfiling(prefix);
}

void ForensicPlatformWrapper::stopHeapProfiling()
{
	m_platform->stopHeapProfiling();
}

void ForensicPlatformWrapper::dumpHeapProfiling(const blink::WebString& reason)
{
	m_platform->dumpHeapProfiling(reason);
}

blink::WebString ForensicPlatformWrapper::getHeapProfile()
{
	return m_platform->getHeapProfile();
}

size_t ForensicPlatformWrapper::maxDecodedImageBytes()
{
	return m_platform->maxDecodedImageBytes();
}

// Message Ports -------------------------------------------------------


blink::WebMessagePortChannel* ForensicPlatformWrapper::createMessagePortChannel()
{
	return m_platform->createMessagePortChannel();
}

void ForensicPlatformWrapper::createMessageChannel(blink::WebMessagePortChannel** channel1, blink::WebMessagePortChannel** channel2)
{
	m_platform->createMessageChannel(channel1, channel2);
}

// Network -------------------------------------------------------------

blink::WebURLLoader* ForensicPlatformWrapper::createURLLoader(){
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::createURLLoader : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::createURLLoader : Instrumentation");
	ForensicWebURLLoader* loader = 0;
	if(m_state->replaying()){
	    TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::createURLLoader : Instrumentation");
		return new ForensicWebURLLoader(m_dataStore, wrappedPlatform(), ForensicInspectorStateWrapper::create(*(m_state.get())));
	} else if(m_state->recording()){
		loader = new ForensicWebURLLoader(m_dataRecorder, wrappedPlatform(), ForensicInspectorStateWrapper::create(*(m_state.get())), m_platform->createURLLoader());
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::createURLLoader : Instrumentation");
	if(m_state->recording())
		return loader;

    return m_platform->createURLLoader();
}

blink::WebPrescientNetworking* ForensicPlatformWrapper::prescientNetworking()
{
	return m_platform->prescientNetworking();
}

blink::WebSocketStreamHandle* ForensicPlatformWrapper::createSocketStreamHandle()
{
	return m_platform->createSocketStreamHandle();
}

blink::WebSocketHandle* ForensicPlatformWrapper::createWebSocketHandle()
{
	return m_platform->createWebSocketHandle();
}

blink::WebString ForensicPlatformWrapper::userAgent()
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::userAgent : Total");

	blink::WebString retval = m_platform->userAgent();

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::userAgent : Instrumentation");
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to userAgent.");

	if(m_state->recording()){
		m_dataRecorder->appendUserAgent(adoptPtr(new blink::WebString(retval)));
	}
	if(m_state->replaying()){
	    TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::userAgent : Instrumentation");
		return m_dataStore->nextUserAgent();
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::userAgent : Instrumentation");

	return retval;

}

void ForensicPlatformWrapper::cacheMetadata(const blink::WebURL& url, double responseTime, const char* data, size_t dataSize)
{
	m_platform->cacheMetadata(url, responseTime, data, dataSize);
}

blink::WebData ForensicPlatformWrapper::parseDataURL(const blink::WebURL& url, blink::WebString& mimetype, blink::WebString& charset)
{
	return m_platform->parseDataURL(url, mimetype, charset);
}

blink::WebURLError ForensicPlatformWrapper::cancelledError(const blink::WebURL& url) const
{
	return m_platform->cancelledError(url);
}

// Plugins -------------------------------------------------------------

void ForensicPlatformWrapper::getPluginList(bool refresh, blink::WebPluginListBuilder* builder)
{
	m_platform->getPluginList(refresh, builder);
}

// Public Suffix List --------------------------------------------------

blink::WebPublicSuffixList* ForensicPlatformWrapper::publicSuffixList()
{
	return m_platform->publicSuffixList();
}

// Resources -----------------------------------------------------------

blink::WebString ForensicPlatformWrapper::queryLocalizedString(blink::WebLocalizedString::Name name)
{
	return m_platform->queryLocalizedString(name);
}

blink::WebString ForensicPlatformWrapper::queryLocalizedString(blink::WebLocalizedString::Name name, const blink::WebString& parameter)
{
	return m_platform->queryLocalizedString(name, parameter);
}

blink::WebString ForensicPlatformWrapper::queryLocalizedString(blink::WebLocalizedString::Name name, const blink::WebString& parameter1, const blink::WebString& parameter2)
{
	return m_platform->queryLocalizedString(name, parameter1, parameter2);
}

// Threads -------------------------------------------------------

blink::WebThread* ForensicPlatformWrapper::createThread(const char* name)
{
	return m_platform->createThread(name);
}

blink::WebThread* ForensicPlatformWrapper::currentThread()
{
	return m_platform->currentThread();
}


// WaitableEvent -------------------------------------------------------

blink::WebWaitableEvent* ForensicPlatformWrapper::createWaitableEvent()
{
	return m_platform->createWaitableEvent();
}

blink::WebWaitableEvent* ForensicPlatformWrapper::waitMultipleEvents(const blink::WebVector<blink::WebWaitableEvent*>& events)
{
	return m_platform->waitMultipleEvents(events);
}

// Profiling -----------------------------------------------------------

void ForensicPlatformWrapper::decrementStatsCounter(const char* name)
{
	m_platform->decrementStatsCounter(name);
}

void ForensicPlatformWrapper::incrementStatsCounter(const char* name)
{
	m_platform->incrementStatsCounter(name);
}

// Resources -----------------------------------------------------------

blink::WebData ForensicPlatformWrapper::loadResource(const char* name)
{
	return m_platform->loadResource(name);
}

bool ForensicPlatformWrapper::loadAudioResource(blink::WebAudioBus* destinationBus, const char* audioFileData, size_t dataSize)
{
	return m_platform->loadAudioResource(destinationBus, audioFileData, dataSize);
}


// Screen -------------------------------------------------------------

void ForensicPlatformWrapper::screenColorProfile(blink::WebVector<char>* profile)
{
	m_platform->screenColorProfile(profile);
}


// Scrollbar ----------------------------------------------------------

blink::WebScrollbarBehavior* ForensicPlatformWrapper::scrollbarBehavior()
{
	return m_platform->scrollbarBehavior();
}


// Sudden Termination --------------------------------------------------

void ForensicPlatformWrapper::suddenTerminationChanged(bool enabled)
{
	m_platform->suddenTerminationChanged(enabled);
}

// System --------------------------------------------------------------

blink::WebString ForensicPlatformWrapper::defaultLocale()
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::defaultLocale : Total");

	blink::WebString retval = m_platform->defaultLocale();

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::defaultLocale : Instrumentation");
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to defaultLocale.");
	if(m_state->recording()) {
		m_dataRecorder->appendDefaultLocale(adoptPtr(new blink::WebString(retval)));
	}
	if(m_state->replaying()) {
		blink::WebString result = m_dataStore->nextDefaultLocale();
		if(!result.isEmpty()) {
	        TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::defaultLocale : Instrumentation");
			return result;
		}
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::defaultLocale : Instrumentation");

	return retval;
}

/*
 * currentTime and monotonicallyIncreasingTime need to be carefully constructed
 * so as to return the value back to the caller ASAP.  We must impact the time
 * accuracy as little as possible.
 */

//Code redundancy is necessary for reducing call overhead
double ForensicPlatformWrapper::currentTime()
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::currentTime : Total");

	double retval = m_platform->currentTime();

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::currentTime : Instrumentation");
	//WebCapsule::log(WebCapsule::Debug,"ForensicPlatformWrapper: Call made to currentTime from thread %u, is main thread %s", WTF::currentThread(), isMainThread() ? "true" : "false");
	if(m_state->recording()){
		m_dataRecorder->appendCurrentTime(retval);
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Recorded value from currentTime of %f", retval);
	}
	if(m_state->replaying()){
		double nowRecTime = (m_platform->currentTime() - m_dataStore->replayStartTime()) + m_dataStore->recordStartTime();
		double next = m_dataStore->nextCurrentTimePreview();

		if(next < 0) {
	        TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::currentTime : Instrumentation");
			return nowRecTime;
        }

		double retval = 0;
		if(next < nowRecTime) {
			do {
				retval = m_dataStore->nextCurrentTime();
				next = m_dataStore->nextCurrentTimePreview();
			} while(retval > 0 && next < nowRecTime);

			if(retval < 0)
				retval = nowRecTime; // we are out of recorded value... therefore we replay what the time *should* have been by now during recording

			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper::currentTime : advanced the clock : %f", retval);
	        TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::currentTime : Instrumentation");
			return retval;
		}
		else {
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper::currentTime : slowing down the clock : %f", nowRecTime);
	        TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::currentTime : Instrumentation");
			return next; // keeps returning the same value until next < nowRecTime
		}

		/* old code
		double retval = m_dataStore->nextCurrentTime();
		if(retval >= 0){
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Replaying value from currentTime of %f", retval);
			return retval;
		}
		retval = m_dataStore->currentTimeOffset();
		return m_platform->currentTime() - retval;
		*/
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::currentTime : Instrumentation");

	return retval;
}

//Code redundancy is necessary for reducing call overhead
double ForensicPlatformWrapper::monotonicallyIncreasingTime()
{
    TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::monotonicallyIncreasingTime : Total");

    double retval = m_platform->monotonicallyIncreasingTime();

    TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::monotonicallyIncreasingTime : Instrumentation");

    if(m_state->recording()){
    	m_dataRecorder->appendMonotonicallyIncreasingTime(retval);
        //WebCapsule::log(WebCapsule::Debug,"ForensicPlatformWrapper: Recorded value from monotonicallyIncreasingTime of %f", retval);
    }

    if(m_state->replaying()){
        // Don't do anything; we will return the "true" monotonicallyIncreasingTime
    }
    TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::monotonicallyIncreasingTime : Instrumentation");

    return retval;
}

void ForensicPlatformWrapper::cryptographicallyRandomValues(unsigned char* buffer, size_t length)
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicPlatformWrapper::cryptographicallyRandomValues : Total");

	if(!m_state->replaying())
		m_platform->cryptographicallyRandomValues(buffer, length);

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicPlatformWrapper::cryptographicallyRandomValues : Instrumentation");
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicPlatformWrapper: Call made to cryptographicallyRandomValues.");
	if(m_state->replaying()){
		if(m_dataStore->nextCryptographicallyRandomValues(buffer, length)){
	        TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::cryptographicallyRandomValues : Instrumentation");
			return;
		}
	}
	if(m_state->recording()){
		m_dataRecorder->appendCryptographicallyRandomValues(buffer,length);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicPlatformWrapper::cryptographicallyRandomValues : Instrumentation");
}

void ForensicPlatformWrapper::setSharedTimerFiredFunction(SharedTimerFunction timerFunction)
{
	m_platform->setSharedTimerFiredFunction(timerFunction);
}

void ForensicPlatformWrapper::setSharedTimerFireInterval(double interval)
{
	m_platform->setSharedTimerFireInterval(interval);
}

void ForensicPlatformWrapper::stopSharedTimer()
{
	m_platform->stopSharedTimer();
}

void ForensicPlatformWrapper::callOnMainThread(void (*func)(void*), void* context)
{
	m_platform->callOnMainThread(func, context);
}

// Vibration -----------------------------------------------------------

void ForensicPlatformWrapper::vibrate(unsigned time)
{
	m_platform->vibrate(time);
}

void ForensicPlatformWrapper::cancelVibration()
{
	m_platform->cancelVibration() ;
}



// Testing -------------------------------------------------------------

blink::WebUnitTestSupport* ForensicPlatformWrapper::unitTestSupport()
{
	return m_platform->unitTestSupport();
}



// Tracing -------------------------------------------------------------

const unsigned char* ForensicPlatformWrapper::getTraceCategoryEnabledFlag(const char* categoryName)
{
	return m_platform->getTraceCategoryEnabledFlag(categoryName);
}

blink::Platform::TraceEventAPIAtomicWord* ForensicPlatformWrapper::getTraceSamplingState(const unsigned bucketName)
{
	return m_platform->getTraceSamplingState(bucketName);
}

blink::Platform::TraceEventHandle ForensicPlatformWrapper::addTraceEvent(
    char phase,
    const unsigned char* categoryEnabledFlag,
    const char* name,
    unsigned long long id,
    int numArgs,
    const char** argNames,
    const unsigned char* argTypes,
    const unsigned long long* argValues,
    unsigned char flags)
{
	return m_platform->addTraceEvent(phase, categoryEnabledFlag, name, id, numArgs, argNames, argTypes, argValues, flags);
}

void ForensicPlatformWrapper::updateTraceEventDuration(const unsigned char* categoryEnabledFlag, const char* name, TraceEventHandle handle)
{
	m_platform->updateTraceEventDuration(categoryEnabledFlag, name, handle);
}

void ForensicPlatformWrapper::histogramCustomCounts(const char* name, int sample, int min, int max, int bucketCount)
{
	m_platform->histogramCustomCounts(name, sample, min, max, bucketCount);
}

void ForensicPlatformWrapper::histogramEnumeration(const char* name, int sample, int boundaryValue)
{
	m_platform->histogramEnumeration(name, sample, boundaryValue);
}

void ForensicPlatformWrapper::histogramSparse(const char* name, int sample)
{
	m_platform->histogramSparse(name, sample);
}

// GPU ----------------------------------------------------------------

blink::WebGraphicsContext3D* ForensicPlatformWrapper::createOffscreenGraphicsContext3D(const blink::WebGraphicsContext3D::Attributes& attrs, blink::WebGraphicsContext3D* shareContext)
{
	return m_platform->createOffscreenGraphicsContext3D(attrs, shareContext) ;
}

blink::WebGraphicsContext3D* ForensicPlatformWrapper::createOffscreenGraphicsContext3D(const blink::WebGraphicsContext3D::Attributes& attrs)
{
	return m_platform->createOffscreenGraphicsContext3D(attrs) ;
}

blink::WebGraphicsContext3DProvider* ForensicPlatformWrapper::createSharedOffscreenGraphicsContext3DProvider()
{
	return m_platform->createSharedOffscreenGraphicsContext3DProvider();
}

bool ForensicPlatformWrapper::canAccelerate2dCanvas()
{
	return m_platform->canAccelerate2dCanvas();
}

bool ForensicPlatformWrapper::isThreadedCompositingEnabled()
{
	return m_platform->isThreadedCompositingEnabled();
}

blink::WebCompositorSupport* ForensicPlatformWrapper::compositorSupport()
{
	return m_platform->compositorSupport();
}

blink::WebFlingAnimator* ForensicPlatformWrapper::createFlingAnimator()
{
	return m_platform->createFlingAnimator();
}

blink::WebGestureCurve* ForensicPlatformWrapper::createFlingAnimationCurve(int deviceSource, const blink::WebFloatPoint& velocity, const blink::WebSize& cumulativeScroll)
{
	return m_platform->createFlingAnimationCurve(deviceSource, velocity, cumulativeScroll);
}



// WebRTC ----------------------------------------------------------

blink::WebRTCPeerConnectionHandler* ForensicPlatformWrapper::createRTCPeerConnectionHandler(blink::WebRTCPeerConnectionHandlerClient* client)
{
	return m_platform->createRTCPeerConnectionHandler(client);
}

blink::WebMediaStreamCenter* ForensicPlatformWrapper::createMediaStreamCenter(blink::WebMediaStreamCenterClient* client)
{
	return m_platform->createMediaStreamCenter(client);
}

// WebWorker ----------------------------------------------------------

void ForensicPlatformWrapper::didStartWorkerRunLoop(const blink::WebWorkerRunLoop& runLoop)
{
	m_platform->didStartWorkerRunLoop(runLoop);
}

void ForensicPlatformWrapper::didStopWorkerRunLoop(const blink::WebWorkerRunLoop& runLoop)
{
	m_platform->didStopWorkerRunLoop(runLoop);
}

// WebCrypto ----------------------------------------------------------

blink::WebCrypto* ForensicPlatformWrapper::crypto()
{
	return m_platform->crypto();
}

// Device Motion / Orientation ----------------------------------------

void ForensicPlatformWrapper::setDeviceMotionListener(blink::WebDeviceMotionListener* listener)
{
	m_platform->setDeviceMotionListener(listener) ;
}

void ForensicPlatformWrapper::setDeviceOrientationListener(blink::WebDeviceOrientationListener* listener)
{
	m_platform->setDeviceOrientationListener(listener);
}

// Screen Orientation -------------------------------------------------

void ForensicPlatformWrapper::setScreenOrientationListener(blink::WebScreenOrientationListener* listener)
{
	m_platform->setScreenOrientationListener(listener);
}

void ForensicPlatformWrapper::lockOrientation(blink::WebScreenOrientations orientation)
{
	m_platform->lockOrientation(orientation);
}

void ForensicPlatformWrapper::unlockOrientation()
{
	m_platform->unlockOrientation();
}



// Quota -----------------------------------------------------------

void ForensicPlatformWrapper::queryStorageUsageAndQuota(
    const blink::WebURL& storagePartition,
    blink::WebStorageQuotaType type,
    blink::WebStorageQuotaCallbacks callbacks)
{
	m_platform->queryStorageUsageAndQuota(storagePartition, type, callbacks);
}



// WebDatabase --------------------------------------------------------

blink::WebDatabaseObserver* ForensicPlatformWrapper::databaseObserver()
{
	return m_platform->databaseObserver();
}

// Wrapper API --------------------------------------------------------

blink::Platform* ForensicPlatformWrapper::wrappedPlatform(){
	return m_platform;
}

PassRefPtr<ForensicReplayDataStore> ForensicPlatformWrapper::dataStore(){
	return m_dataStore;
}

PassRefPtr<ForensicReplayDataRecorder> ForensicPlatformWrapper::dataRecorder(){
	return m_dataRecorder;
}

} /* namespace WebCore */
