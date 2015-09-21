/*
 * ForensicOffloadingRecorder.h
 *
 *  Created on: Feb 16, 2015
 *      Author: cjneasbi
 */

#ifndef ForensicOffloadingRecorder_h
#define ForensicOffloadingRecorder_h

#include "core/inspector/ForensicRequestAssembler.h"
#include "InspectorFrontend.h"
#include "public/platform/Platform.h"

#include <map>
#include <vector>

namespace WebCore {

class ForensicOffloadingRecorder : public ForensicRequestAssembler {
public:
	static PassRefPtr<ForensicOffloadingRecorder> create(blink::Platform*);

	virtual ~ForensicOffloadingRecorder();

	virtual void setRecordStartTime(double t);
	virtual void setRecordStartTimeMonotonic(double t);

	virtual void setURLAlias(const String&, const String&);
	virtual void addToPageURLRequestsMap(String& pageURL, PassRefPtr<PageURLRequest> request);

	virtual void setKeyPressEventTargetRecordInfo(const String&, KeyPressEventTargetInfo);
	virtual void setMousePressEventTargetRecordInfo(const String&, MousePressEventTargetInfo);

    virtual void appendForensicEvent(PassRefPtr<ForensicEvent>);

    virtual void appendCurrentTime(double);
    virtual void appendMonotonicallyIncreasingTime(double);

    virtual void appendUserAgent(PassOwnPtr<blink::WebString>);

    virtual void appendMemoryUsageMB(size_t);
    virtual void appendActualMemoryUsageMB(size_t);
    virtual void appendPhysicalMemoryMB(size_t);
    virtual void appendNumberOfProcessors(size_t);

    virtual void appendDefaultLocale(PassOwnPtr<blink::WebString>);

    virtual void appendCryptographicallyRandomValues(unsigned char*, size_t);

    virtual void appendIsLinkVisited(unsigned long long, bool);
    virtual void appendVisitedLinkHash(const char*, size_t, unsigned long long);

    virtual void appendSignedPublicKeyAndChallengeString(unsigned, const blink::WebString&, const blink::WebURL&, blink::WebString&);

    virtual void appendClipboardSequenceNumber(blink::WebClipboard::Buffer, uint64);
    virtual void appendClipboardIsFormatAvailable(blink::WebClipboard::Format, blink::WebClipboard::Buffer, bool);
    virtual void appendClipboardReadAvailableTypes(blink::WebClipboard::Buffer, bool*, blink::WebVector<blink::WebString>&);
    virtual void appendClipboardReadPlainText(blink::WebClipboard::Buffer, blink::WebString&);
    virtual void appendClipboardReadHTML(blink::WebClipboard::Buffer, blink::WebURL*, unsigned*, unsigned*, blink::WebString&);
    virtual void appendClipboardReadImage(blink::WebClipboard::Buffer, blink::WebData&);
    virtual void appendClipboardReadCustomData(blink::WebClipboard::Buffer, const blink::WebString&, blink::WebString&);

    virtual void appendModaldialogConfirmResult(const String&, bool);
    virtual void appendModaldialogPromptResult(const String&,const String&);
    virtual void appendModaldialogAlertVals(const String&);

    virtual void stopRecording();

protected:
    ForensicOffloadingRecorder(blink::Platform*);
    virtual void addFinishedRequest(PassRefPtr<PendingRequestMapEntry> entry);
    virtual void addFinishedRedirect(blink::WebURLRequest&, blink::WebURLRequest&, const blink::WebURLResponse&, const V8StackTrace&, double);
    virtual void offloadV8Data();
    virtual void offloadV8Map(const std::map<std::string, std::vector<double> >&, const String&);

    virtual bool setValue(const String& name, InspectorFrontend::Forensics::Domain::Enum domain, PassRefPtr<JSONObject> key, PassRefPtr<JSONObject> value, double size);
    virtual bool addValue(const String& name, InspectorFrontend::Forensics::Domain::Enum domain, PassRefPtr<JSONObject> key, PassRefPtr<JSONObject> value, double size);
    virtual bool addMultipleValues(const String& name, InspectorFrontend::Forensics::Domain::Enum domain, PassRefPtr<JSONObject> key, PassRefPtr<JSONObject> value, double size);
    virtual bool sendMessage(const String& name, InspectorFrontend::Forensics::Domain::Enum domain, PassRefPtr<JSONObject> key, PassRefPtr<JSONObject> value, const bool batch, const bool replace, double size);


    blink::Platform* m_platform;

};

} /* namespace WebCore */
#endif /* ForensicOffloadingRecorder_h */
