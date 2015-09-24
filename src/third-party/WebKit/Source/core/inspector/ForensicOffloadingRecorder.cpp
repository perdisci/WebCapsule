
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicOffloadingRecorder.h"
#include "core/inspector/InspectorForensicsAgent.h"
#include "core/inspector/ForensicSerialization.h"
#include "wtf/Atomics.h"
#include <v8.h>

namespace WebCore {

static volatile int64_t s_seqId  = 0;

PassRefPtr<ForensicOffloadingRecorder> ForensicOffloadingRecorder::create(blink::Platform* platform){

}

ForensicOffloadingRecorder::ForensicOffloadingRecorder(blink::Platform* platform)
	: m_platform(platform)
{

}

ForensicOffloadingRecorder::~ForensicOffloadingRecorder() {

}

void ForensicOffloadingRecorder::setRecordStartTime(double t){

}

void ForensicOffloadingRecorder::setRecordStartTimeMonotonic(double t){
}

void ForensicOffloadingRecorder::setURLAlias(const String& alias, const String& orig){

}

void ForensicOffloadingRecorder::addToPageURLRequestsMap(String& pageURL, PassRefPtr<PageURLRequest> request){

}

void ForensicOffloadingRecorder::setKeyPressEventTargetRecordInfo(const String& id, KeyPressEventTargetInfo info){

}

void ForensicOffloadingRecorder::setMousePressEventTargetRecordInfo(const String& id, MousePressEventTargetInfo info){

}

void ForensicOffloadingRecorder::appendForensicEvent(PassRefPtr<ForensicEvent> event){

}


void ForensicOffloadingRecorder::appendCurrentTime(double t){

}


void ForensicOffloadingRecorder::appendMonotonicallyIncreasingTime(double t){

}

void ForensicOffloadingRecorder::appendUserAgent(PassOwnPtr<blink::WebString> userAgent){

}

void ForensicOffloadingRecorder::appendMemoryUsageMB(size_t size){

}

void ForensicOffloadingRecorder::appendActualMemoryUsageMB(size_t size){

}

void ForensicOffloadingRecorder::appendPhysicalMemoryMB(size_t size){

}

void ForensicOffloadingRecorder::appendNumberOfProcessors(size_t size){

}

void ForensicOffloadingRecorder::appendDefaultLocale(PassOwnPtr<blink::WebString> locale){

}

void ForensicOffloadingRecorder::appendCryptographicallyRandomValues(unsigned char* buf, size_t size){

}

void ForensicOffloadingRecorder::appendIsLinkVisited(unsigned long long hash, bool retval){

}

void ForensicOffloadingRecorder::appendVisitedLinkHash(const char* url, size_t length, unsigned long long retval){

}

void ForensicOffloadingRecorder::appendSignedPublicKeyAndChallengeString(
		unsigned keySizeIndex,
		const blink::WebString& challenge,
		const blink::WebURL& url,
		blink::WebString& retval){

}

void ForensicOffloadingRecorder::appendClipboardSequenceNumber(blink::WebClipboard::Buffer buffer, uint64 seq){

}

void ForensicOffloadingRecorder::appendClipboardIsFormatAvailable(blink::WebClipboard::Format format, blink::WebClipboard::Buffer buffer, bool retval){

}

void ForensicOffloadingRecorder::appendClipboardReadAvailableTypes(blink::WebClipboard::Buffer buffer, bool* containsFilenames, blink::WebVector<blink::WebString>& retval){

}

void ForensicOffloadingRecorder::appendClipboardReadPlainText(blink::WebClipboard::Buffer buffer, blink::WebString& retval){

}

void ForensicOffloadingRecorder::appendClipboardReadHTML(blink::WebClipboard::Buffer buffer, blink::WebURL* sourceUrl, unsigned* fragmentStart, unsigned* fragmentEnd, blink::WebString& retval){

}

void ForensicOffloadingRecorder::appendClipboardReadImage(blink::WebClipboard::Buffer buffer, blink::WebData& retval){

}

void ForensicOffloadingRecorder::appendClipboardReadCustomData(blink::WebClipboard::Buffer buffer, const blink::WebString& type, blink::WebString& retval){

}

void ForensicOffloadingRecorder::appendModaldialogConfirmResult(const String& message, bool result){

}

void ForensicOffloadingRecorder::appendModaldialogPromptResult(const String& message, const String& result){

}

void ForensicOffloadingRecorder::appendModaldialogAlertVals(const String& message){

}

void ForensicOffloadingRecorder::stopRecording(){

}

void ForensicOffloadingRecorder::addFinishedRequest(PassRefPtr<PendingRequestMapEntry> entry){

}

//RedirectEntry::create(request, response, stackTrace, startTime)
void ForensicOffloadingRecorder::addFinishedRedirect(
		blink::WebURLRequest& request,
		blink::WebURLRequest& redirectRequest,
		const blink::WebURLResponse& redirectResponse,
		const V8StackTrace& stackTrace,
		double startTime){

}

void ForensicOffloadingRecorder::offloadV8Data(){

}

void ForensicOffloadingRecorder::offloadV8Map(const std::map<std::string, std::vector<double> >& src, const String& mapName){

}

//use if you want to override the data located at key or if key should be associated with a single value (e.g. a single string)
bool ForensicOffloadingRecorder::setValue(const String& name, InspectorFrontend::Forensics::Domain::Enum domain, PassRefPtr<JSONObject> key, PassRefPtr<JSONObject> value, double size){

}

//use if you want to append to the data located at key, in this case value is treated as a single value to be appended
bool ForensicOffloadingRecorder::addValue(const String& name, InspectorFrontend::Forensics::Domain::Enum domain, PassRefPtr<JSONObject> key, PassRefPtr<JSONObject> value, double size){

}

//use if you want to append multiple values to the data located at key, in this case value is treated as a list of values to be appended
bool ForensicOffloadingRecorder::addMultipleValues(const String& name, InspectorFrontend::Forensics::Domain::Enum domain, PassRefPtr<JSONObject> key, PassRefPtr<JSONObject> value, double size){

}

bool ForensicOffloadingRecorder::sendMessage(const String& name, InspectorFrontend::Forensics::Domain::Enum domain, PassRefPtr<JSONObject> key, PassRefPtr<JSONObject> value, const bool batch, const bool replace, double size){

}

} /* namespace WebCore */
