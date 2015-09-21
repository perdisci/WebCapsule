/*
 * ForensicRequestAssembler.h
 *
 *  Created on: Feb 17, 2015
 *      Author: cjneasbi
 */

#ifndef ForensicRequestAssembler_h
#define ForensicRequestAssembler_h

#include "core/inspector/ForensicReplayData.h"
#include "core/inspector/ForensicReplayDataRecorder.h"

namespace WebCore {

class ForensicRequestAssembler: public ForensicReplayDataRecorder {
public:
	virtual ~ForensicRequestAssembler(){};

	static const char* WEBCAPSULE_HEADER;

	virtual void setRecordStartTime(double t) = 0;
	virtual void setRecordStartTimeMonotonic(double t) = 0;

	virtual const String findPageURLAliasChainRoot(const String& startURL);
	virtual void setURLAlias(const String&, const String&);
	virtual void addToPageURLRequestsMap(String& pageURL, PassRefPtr<PageURLRequest> request);
	virtual size_t sizePageURLRequests(const String&);

	virtual void setKeyPressEventTargetRecordInfo(const String&, KeyPressEventTargetInfo) = 0;
	virtual void setMousePressEventTargetRecordInfo(const String&, MousePressEventTargetInfo) = 0;

    virtual void initPendingRequest(blink::WebURLLoaderClient*, const blink::WebURLRequest&, double startTime);
    virtual void syncRequestFinished(const blink::WebURLRequest&,
    		blink::WebURLResponse&,
    		blink::WebURLError&,
    		blink::WebData&,
    		double,
    		double,
    		double);
    virtual void setPendingResponse(blink::WebURLLoaderClient*, const blink::WebURLResponse&);
    virtual void addPendingData(blink::WebURLLoaderClient*, const char*, int);
    virtual void addPendingRedirect(
    		blink::WebURLLoaderClient*,
    		blink::WebURLRequest&,
    		const blink::WebURLResponse&,
    		const blink::WebString&,
    		double startTime);
    virtual void pendingRequestFinished(blink::WebURLLoaderClient*, double, double, int64);
    virtual void pendingRequestFailed(blink::WebURLLoaderClient*, const blink::WebURLError&);
    virtual void setStackTraceForRequest(const String&, const V8StackTrace&);

    virtual void appendForensicEvent(PassRefPtr<ForensicEvent>) = 0;

    virtual void appendCurrentTime(double) = 0;
    virtual void appendMonotonicallyIncreasingTime(double) = 0;

    virtual void appendUserAgent(PassOwnPtr<blink::WebString>) = 0;
    virtual void appendMemoryUsageMB(size_t) = 0;
    virtual void appendActualMemoryUsageMB(size_t) = 0;
    virtual void appendPhysicalMemoryMB(size_t) = 0;
    virtual void appendNumberOfProcessors(size_t) = 0;
    virtual void appendDefaultLocale(PassOwnPtr<blink::WebString>) = 0;
    virtual void appendCryptographicallyRandomValues(unsigned char*, size_t) = 0;
    virtual void appendIsLinkVisited(unsigned long long, bool) = 0;
    virtual void appendVisitedLinkHash(const char*, size_t, unsigned long long) = 0;
    virtual void appendSignedPublicKeyAndChallengeString(unsigned, const blink::WebString&, const blink::WebURL&, blink::WebString&) = 0;
    virtual void appendClipboardSequenceNumber(blink::WebClipboard::Buffer, uint64) = 0;
    virtual void appendClipboardIsFormatAvailable(blink::WebClipboard::Format, blink::WebClipboard::Buffer, bool) = 0;
    virtual void appendClipboardReadAvailableTypes(blink::WebClipboard::Buffer, bool*, blink::WebVector<blink::WebString>&) = 0;
    virtual void appendClipboardReadPlainText(blink::WebClipboard::Buffer, blink::WebString&) = 0;
    virtual void appendClipboardReadHTML(blink::WebClipboard::Buffer, blink::WebURL*, unsigned*, unsigned*, blink::WebString&) = 0;
    virtual void appendClipboardReadImage(blink::WebClipboard::Buffer, blink::WebData&) = 0;
    virtual void appendClipboardReadCustomData(blink::WebClipboard::Buffer, const blink::WebString&, blink::WebString&) = 0;

    virtual void clear();

    virtual void stopRecording() = 0;

protected:
    virtual void addFinishedRequest(PassRefPtr<PendingRequestMapEntry> entry) = 0;
    virtual void addFinishedRedirect(blink::WebURLRequest&, blink::WebURLRequest&, const blink::WebURLResponse&, const V8StackTrace&, double) = 0;

    PendingRequestMap m_pendingMap; //collects data from multiple async callbacks in ForensicWebURLLoader, never to be used during replay
    StackTraceMap m_stackTraceMap; //collects stack traces associated with the creation of a request, should not be serialized

    PageURLAliasMap m_pageURLAliasMap;
    PageURLRequestsMap m_pageURLRequestsMap;

private:
    PassRefPtr<PendingRequestMapEntry> createPendingEntry(const blink::WebURLRequest& request, double startTime);


};

} /* namespace WebCore */
#endif /* ForensicRequestAssembler_h */
