
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicPlatformWrapper_h
#define ForensicPlatformWrapper_h

#include "public/platform/Platform.h"
#include "wtf/Vector.h"
#include "core/inspector/InspectorState.h"
#include "core/inspector/ForensicInspectorStateWrapper.h"


namespace blink {
class WebAudioBus;
class WebBlobRegistry;
class WebContentDecryptionModule;
class WebClipboard;
class WebCompositorSupport;
class WebCookieJar;
class WebCrypto;
class WebDatabaseObserver;
class WebDeviceMotionListener;
class WebDeviceOrientationListener;
class WebDiscardableMemory;
class WebFallbackThemeEngine;
class WebFileSystem;
class WebFileUtilities;
class WebFlingAnimator;
class WebGestureCurveTarget;
class WebGestureCurve;
class WebGraphicsContext3DProvider;
class WebIDBFactory;
class WebMIDIAccessor;
class WebMIDIAccessorClient;
class WebMediaStreamCenter;
class WebMediaStreamCenterClient;
class WebMessagePortChannel;
class WebMimeRegistry;
class WebPluginListBuilder;
class WebPrescientNetworking;
class WebPublicSuffixList;
class WebRTCPeerConnectionHandler;
class WebRTCPeerConnectionHandlerClient;
class WebSandboxSupport;
class WebScreenOrientationListener;
class WebScrollbarBehavior;
class WebSocketHandle;
class WebSocketStreamHandle;
class WebSpeechSynthesizer;
class WebSpeechSynthesizerClient;
class WebStorageNamespace;
struct WebFloatPoint;
class WebThemeEngine;
class WebThread;
class WebURL;
class WebURLLoader;
class WebUnitTestSupport;
class WebWaitableEvent;
class WebWorkerRunLoop;
struct WebLocalizedString;
struct WebSize;
}

namespace WebCore {

class ForensicReplayDataStore;
class ForensicReplayDataRecorder;

class ForensicPlatformWrapper: public blink::Platform {
public:
	ForensicPlatformWrapper(Platform*, PassOwnPtr<ForensicInspectorStateWrapper>, PassRefPtr<ForensicReplayDataStore>, PassRefPtr<ForensicReplayDataRecorder>);
	virtual ~ForensicPlatformWrapper();

    virtual blink::WebCookieJar* cookieJar() OVERRIDE;
    virtual blink::WebClipboard* clipboard() OVERRIDE;
    virtual blink::WebFileUtilities* fileUtilities() OVERRIDE;
    virtual blink::WebMimeRegistry* mimeRegistry() OVERRIDE;
    virtual blink::WebSandboxSupport* sandboxSupport() OVERRIDE;
    virtual blink::WebThemeEngine* themeEngine() OVERRIDE;
    virtual blink::WebFallbackThemeEngine* fallbackThemeEngine() OVERRIDE;
    virtual blink::WebSpeechSynthesizer* createSpeechSynthesizer(blink::WebSpeechSynthesizerClient*) OVERRIDE;


    // Audio --------------------------------------------------------------

    virtual double audioHardwareSampleRate() OVERRIDE;
    virtual size_t audioHardwareBufferSize() OVERRIDE;
    virtual unsigned audioHardwareOutputChannels() OVERRIDE;
    virtual blink::WebAudioDevice* createAudioDevice(size_t, unsigned, unsigned, double, blink::WebAudioDevice::RenderCallback*, const blink::WebString&) OVERRIDE;


    // MIDI ----------------------------------------------------------------

    virtual blink::WebMIDIAccessor* createMIDIAccessor(blink::WebMIDIAccessorClient*) OVERRIDE;


    // Blob ----------------------------------------------------------------

    virtual blink::WebBlobRegistry* blobRegistry() OVERRIDE;


    // Database ------------------------------------------------------------

    virtual FileHandle databaseOpenFile(const blink::WebString&, int) OVERRIDE;
    virtual int databaseDeleteFile(const blink::WebString&, bool) OVERRIDE;
    virtual long databaseGetFileAttributes(const blink::WebString&) OVERRIDE;
    virtual long long databaseGetFileSize(const blink::WebString&) OVERRIDE;
    virtual long long databaseGetSpaceAvailableForOrigin(const blink::WebString&) OVERRIDE;


    // DOM Storage --------------------------------------------------

    virtual blink::WebStorageNamespace* createLocalStorageNamespace() OVERRIDE;


    // FileSystem ----------------------------------------------------------

    virtual blink::WebFileSystem* fileSystem() OVERRIDE;


    // IDN conversion ------------------------------------------------------

    virtual blink::WebString convertIDNToUnicode(const blink::WebString& host, const blink::WebString& languages) OVERRIDE;


    // IndexedDB ----------------------------------------------------------

    virtual blink::WebIDBFactory* idbFactory() OVERRIDE;


    // Gamepad -------------------------------------------------------------

    virtual void sampleGamepads(blink::WebGamepads&) OVERRIDE;
    virtual void setGamepadListener(blink::WebGamepadListener*) OVERRIDE;


    // History -------------------------------------------------------------

    virtual unsigned long long visitedLinkHash(const char*, size_t) OVERRIDE;
    virtual bool isLinkVisited(unsigned long long) OVERRIDE;


    // Keygen --------------------------------------------------------------

    virtual blink::WebString signedPublicKeyAndChallengeString(unsigned, const blink::WebString&, const blink::WebURL&) OVERRIDE;


    // Memory --------------------------------------------------------------

    virtual size_t memoryUsageMB() OVERRIDE;
    virtual size_t actualMemoryUsageMB() OVERRIDE;
    virtual size_t physicalMemoryMB() OVERRIDE;
    virtual size_t numberOfProcessors() OVERRIDE;
    virtual bool processMemorySizesInBytes(size_t*, size_t*) OVERRIDE;
    virtual void requestProcessMemorySizes(ProcessMemorySizesCallback*) OVERRIDE;
    virtual bool memoryAllocatorWasteInBytes(size_t*) OVERRIDE;
    virtual blink::WebDiscardableMemory* allocateAndLockDiscardableMemory(size_t) OVERRIDE;
    virtual void startHeapProfiling(const blink::WebString&) OVERRIDE;
    virtual void stopHeapProfiling() OVERRIDE;
    virtual void dumpHeapProfiling(const blink::WebString&) OVERRIDE;
    virtual blink::WebString getHeapProfile() OVERRIDE;
    virtual size_t maxDecodedImageBytes() OVERRIDE;


    // Message Ports -------------------------------------------------------


    virtual blink::WebMessagePortChannel* createMessagePortChannel() OVERRIDE;
    virtual void createMessageChannel(blink::WebMessagePortChannel**, blink::WebMessagePortChannel**) OVERRIDE;


    // Network -------------------------------------------------------------

    virtual blink::WebURLLoader* createURLLoader() OVERRIDE;
    virtual blink::WebPrescientNetworking* prescientNetworking() OVERRIDE;
    virtual blink::WebSocketStreamHandle* createSocketStreamHandle() OVERRIDE;
    virtual blink::WebSocketHandle* createWebSocketHandle() OVERRIDE;
    virtual blink::WebString userAgent() OVERRIDE;
    virtual void cacheMetadata(const blink::WebURL&, double, const char*, size_t) OVERRIDE;
    virtual blink::WebData parseDataURL(const blink::WebURL&, blink::WebString&, blink::WebString&) OVERRIDE;
    virtual blink::WebURLError cancelledError(const blink::WebURL&) const OVERRIDE;


    // Plugins -------------------------------------------------------------

    virtual void getPluginList(bool, blink::WebPluginListBuilder*) OVERRIDE;


    // Public Suffix List --------------------------------------------------

    virtual blink::WebPublicSuffixList* publicSuffixList() OVERRIDE;


    // Resources -----------------------------------------------------------

    virtual blink::WebString queryLocalizedString(blink::WebLocalizedString::Name) OVERRIDE;
    virtual blink::WebString queryLocalizedString(blink::WebLocalizedString::Name, const blink::WebString&) OVERRIDE;
    virtual blink::WebString queryLocalizedString(blink::WebLocalizedString::Name, const blink::WebString&, const blink::WebString&) OVERRIDE;


    // Threads -------------------------------------------------------

    virtual blink::WebThread* createThread(const char*) OVERRIDE;
    virtual blink::WebThread* currentThread() OVERRIDE;


    // WaitableEvent -------------------------------------------------------

    virtual blink::WebWaitableEvent* createWaitableEvent() OVERRIDE;
    virtual blink::WebWaitableEvent* waitMultipleEvents(const blink::WebVector<blink::WebWaitableEvent*>&) OVERRIDE;


    // Profiling -----------------------------------------------------------

    virtual void decrementStatsCounter(const char* name) OVERRIDE;
    virtual void incrementStatsCounter(const char* name) OVERRIDE;


    // Resources -----------------------------------------------------------

    virtual blink::WebData loadResource(const char*) OVERRIDE;
    virtual bool loadAudioResource(blink::WebAudioBus*, const char*, size_t) OVERRIDE;

    // Screen -------------------------------------------------------------

    virtual void screenColorProfile(blink::WebVector<char>*) OVERRIDE;


    // Scrollbar ----------------------------------------------------------

    virtual blink::WebScrollbarBehavior* scrollbarBehavior() OVERRIDE;


    // Sudden Termination --------------------------------------------------

    virtual void suddenTerminationChanged(bool) OVERRIDE;


    // System --------------------------------------------------------------

    virtual blink::WebString defaultLocale() OVERRIDE;
    virtual double currentTime() OVERRIDE;
    virtual double monotonicallyIncreasingTime() OVERRIDE;
    virtual void cryptographicallyRandomValues(unsigned char*, size_t) OVERRIDE;
    virtual void setSharedTimerFiredFunction(SharedTimerFunction) OVERRIDE;
    virtual void setSharedTimerFireInterval(double) OVERRIDE;
    virtual void stopSharedTimer() OVERRIDE;
    virtual void callOnMainThread(void (*func)(void*), void* context) OVERRIDE;


    // Vibration -----------------------------------------------------------

    virtual void vibrate(unsigned) OVERRIDE;
    virtual void cancelVibration() OVERRIDE;


    // Testing -------------------------------------------------------------

    virtual blink::WebUnitTestSupport* unitTestSupport() OVERRIDE;


    // Tracing -------------------------------------------------------------

    virtual const unsigned char* getTraceCategoryEnabledFlag(const char* categoryName) OVERRIDE;
    virtual TraceEventAPIAtomicWord* getTraceSamplingState(const unsigned bucketName) OVERRIDE;
    virtual TraceEventHandle addTraceEvent(
        char phase,
        const unsigned char* categoryEnabledFlag,
        const char* name,
        unsigned long long id,
        int numArgs,
        const char** argNames,
        const unsigned char* argTypes,
        const unsigned long long* argValues,
        unsigned char flags) OVERRIDE;
    virtual void updateTraceEventDuration(const unsigned char* categoryEnabledFlag, const char* name, TraceEventHandle) OVERRIDE;
    virtual void histogramCustomCounts(const char* name, int sample, int min, int max, int bucketCount) OVERRIDE;
    virtual void histogramEnumeration(const char* name, int sample, int boundaryValue) OVERRIDE;
    virtual void histogramSparse(const char* name, int sample) OVERRIDE;


    // GPU ----------------------------------------------------------------

    virtual blink::WebGraphicsContext3D* createOffscreenGraphicsContext3D(const blink::WebGraphicsContext3D::Attributes&, blink::WebGraphicsContext3D*) OVERRIDE;
    virtual blink::WebGraphicsContext3D* createOffscreenGraphicsContext3D(const blink::WebGraphicsContext3D::Attributes&) OVERRIDE;
    virtual blink::WebGraphicsContext3DProvider* createSharedOffscreenGraphicsContext3DProvider() OVERRIDE;
    virtual bool canAccelerate2dCanvas() OVERRIDE;
    virtual bool isThreadedCompositingEnabled() OVERRIDE;
    virtual blink::WebCompositorSupport* compositorSupport() OVERRIDE;
    virtual blink::WebFlingAnimator* createFlingAnimator() OVERRIDE;
    virtual blink::WebGestureCurve* createFlingAnimationCurve(int, const blink::WebFloatPoint&, const blink::WebSize&) OVERRIDE;


    // WebRTC ----------------------------------------------------------

    virtual blink::WebRTCPeerConnectionHandler* createRTCPeerConnectionHandler(blink::WebRTCPeerConnectionHandlerClient*) OVERRIDE;
    virtual blink::WebMediaStreamCenter* createMediaStreamCenter(blink::WebMediaStreamCenterClient*) OVERRIDE;


    // WebWorker ----------------------------------------------------------

    virtual void didStartWorkerRunLoop(const blink::WebWorkerRunLoop&) OVERRIDE;
    virtual void didStopWorkerRunLoop(const blink::WebWorkerRunLoop&) OVERRIDE;


    // WebCrypto ----------------------------------------------------------

    virtual blink::WebCrypto* crypto() OVERRIDE;


    // Device Motion / Orientation ----------------------------------------

    virtual void setDeviceMotionListener(blink::WebDeviceMotionListener*) OVERRIDE;
    virtual void setDeviceOrientationListener(blink::WebDeviceOrientationListener*) OVERRIDE;

    // Screen Orientation -------------------------------------------------

    virtual void setScreenOrientationListener(blink::WebScreenOrientationListener*) OVERRIDE;
    virtual void lockOrientation(blink::WebScreenOrientations) OVERRIDE;
    virtual void unlockOrientation() OVERRIDE;


    // Quota -----------------------------------------------------------

    virtual void queryStorageUsageAndQuota(
        const blink::WebURL& storagePartition,
        blink::WebStorageQuotaType,
        blink::WebStorageQuotaCallbacks) OVERRIDE;


    // WebDatabase --------------------------------------------------------

    virtual blink::WebDatabaseObserver* databaseObserver() OVERRIDE;

    // Wrapper API --------------------------------------------------------

    blink::Platform* wrappedPlatform();
    PassRefPtr<ForensicReplayDataStore> dataStore();
    PassRefPtr<ForensicReplayDataRecorder> dataRecorder();

private:
	Platform* m_platform;
	OwnPtr<ForensicInspectorStateWrapper> m_state;
	RefPtr<ForensicReplayDataStore> m_dataStore;
	RefPtr<ForensicReplayDataRecorder> m_dataRecorder;
};

} /* namespace WebCore */
#endif /* ForensicPlatformWrapper_h */
