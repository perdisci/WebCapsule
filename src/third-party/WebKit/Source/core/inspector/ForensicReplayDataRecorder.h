
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicReplayDataRecorder_h
#define ForensicReplayDataRecorder_h

#include "wtf/PassOwnPtr.h"
#include "wtf/PassRefPtr.h"
#include "public/platform/WebVector.h"

namespace blink {

class WebURLLoaderClient;
class WebURLRequest;
class WebURLResponse;
class WebURLError;
class WebData;
class WebString;
class WebClipboard;

}

namespace WebCore {

class KURL;
class PageURLRequest;
class KeyPressEventTargetInfo;
class V8StackTrace;
class ForensicEvent;

//A pure virtual class that defines all operations required when recording data
class ForensicReplayDataRecorder : public ThreadSafeRefCounted<ForensicReplayDataRecorder> {
public:
	virtual ~ForensicReplayDataRecorder(){};

	virtual void setRecordStartTime(double t) = 0;
	virtual void setRecordStartTimeMonotonic(double t) = 0;

	virtual const String findPageURLAliasChainRoot(const String& startURL) = 0;
	virtual void setURLAlias(const String&, const String&) = 0;
	virtual void addToPageURLRequestsMap(String& pageURL, PassRefPtr<PageURLRequest> request) = 0;
	virtual size_t sizePageURLRequests(const String&) = 0;

	virtual void setKeyPressEventTargetRecordInfo(const String&, KeyPressEventTargetInfo) = 0;
	virtual void setMousePressEventTargetRecordInfo(const String&, MousePressEventTargetInfo) = 0;

    virtual void initPendingRequest(blink::WebURLLoaderClient*, const blink::WebURLRequest&, double startTime) = 0;
    virtual void syncRequestFinished(const blink::WebURLRequest&,
    		blink::WebURLResponse&,
    		blink::WebURLError&,
    		blink::WebData&,
    		double,
    		double,
    		double) = 0;
    virtual void setPendingResponse(blink::WebURLLoaderClient*, const blink::WebURLResponse&) = 0;
    virtual void addPendingData(blink::WebURLLoaderClient*, const char*, int) = 0;
    virtual void addPendingRedirect(blink::WebURLLoaderClient*, blink::WebURLRequest&, const blink::WebURLResponse&, const blink::WebString&, double startTime) = 0;
    virtual void pendingRequestFinished(blink::WebURLLoaderClient*, double, double, int64) = 0;
    virtual void pendingRequestFailed(blink::WebURLLoaderClient*, const blink::WebURLError&) = 0;

    virtual void setStackTraceForRequest(const String&, const V8StackTrace&) = 0;

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

    //Modaldialog by Bo
    virtual void appendModaldialogConfirmResult(const String&, bool) = 0;
    virtual void appendModaldialogPromptResult(const String&,const String&) = 0;
    virtual void appendModaldialogAlertVals(const String&) = 0;

    virtual void clear() = 0;

    //used to finish any remaining recording actions.
    virtual void stopRecording() = 0;

};

} /* namespace WebCore */
#endif /* FORENSICREPLAYDATARECORDER_H_ */
