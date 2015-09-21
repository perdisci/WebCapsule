/*
 * ForensicReplayDataStore.cpp
 *
 *  Created on: Jul 25, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicReplayDataStore.h"
#include "core/inspector/WebCapsule.h"
#include "core/inspector/JSONParser.h"
#include "core/inspector/ForensicClearCacheEvent.h"
#include "core/inspector/ForensicDownloadDataEvent.h"
#include "core/inspector/ForensicDidFailEvent.h"
#include "core/inspector/ForensicFinishLoadingEvent.h"
#include "core/inspector/ForensicInputEvent.h"
#include "core/inspector/ForensicLoadURLEvent.h"
#include "core/inspector/ForensicPageResizeEvent.h"
#include "core/inspector/ForensicPageScrollEvent.h"
#include "core/inspector/ForensicReceiveCachedMetadataEvent.h"
#include "core/inspector/ForensicReceiveDataEvent.h"
#include "core/inspector/ForensicReceiveResponseEvent.h"
#include "core/inspector/ForensicRecordingStartEvent.h"
#include "core/inspector/ForensicRedirectEvent.h"
#include "core/inspector/ForensicReplayStopEvent.h"
#include "core/inspector/ForensicRenderMenuListValueChangedEvent.h"
#include "core/inspector/ForensicSetAutofilledEvent.h"
#include "core/inspector/ForensicSetEditingValueEvent.h"
#include "core/inspector/ForensicLoadHistoryItemEvent.h"
#include "wtf/text/Base64.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace WebCore {

const char* ForensicReplayDataStore::WEBCAPSULE_HEADER = "X-WebCapsule";

PassRefPtr<ForensicReplayDataStore> ForensicReplayDataStore::create(){
	return adoptRef(new ForensicReplayDataStore());
}

ForensicReplayDataStore::ForensicReplayDataStore()
	: m_currentTimeVals(DoubleVector::create())
	, m_monotonicallyIncreasingTimeVals(DoubleVector::create())
	, m_userAgentVals(WebStringVector::create())
	, m_memoryUsageMBVals(SizeTypeVector::create())
	, m_actualMemoryUsageMBVals(SizeTypeVector::create())
	, m_physicalMemoryMBVals(SizeTypeVector::create())
	, m_numberOfProcessorsVals(SizeTypeVector::create())
	, m_defaultLocaleVals(WebStringVector::create())
	, m_modaldialogAlertVals(WebStringVector::create())
{
	resetOffSets();
}

ForensicReplayDataStore::~ForensicReplayDataStore() {

}

PassRefPtr<PageURLRequest> ForensicReplayDataStore::getPageURLRequest(const PageURLRequestMapLocation& mapLocation){
	if(!mapLocation.currentUrlRootAlias.isNull() && !mapLocation.currentUrlRootAlias.isEmpty() && m_pageURLRequestsMap.contains(mapLocation.currentUrlRootAlias)){
		const Vector<RefPtr<PageURLRequest> >& values = m_pageURLRequestsMap.get(mapLocation.currentUrlRootAlias);
		if(mapLocation.pos >= 0 && mapLocation.pos < values.size()){
			return values.at(mapLocation.pos);
		}
	}
	return PageURLRequest::createEmpty();
}

bool ForensicReplayDataStore::containsKeyPressEventTargetRecordInfo(const String& id){
	return m_KeyPressEventTargetRecordInfoMap.contains(id);
}

void ForensicReplayDataStore::setKeyPressEventTargetRecordInfo(const String& id, KeyPressEventTargetInfo info){
	m_KeyPressEventTargetRecordInfoMap.set(id, info);
}

const KeyPressEventTargetInfo ForensicReplayDataStore::getKeyPressEventTargetRecordInfo(const String& id){
	return m_KeyPressEventTargetRecordInfoMap.get(id);
}

void ForensicReplayDataStore::setKeyPressEventTargetReplayInfo(const String& id, KeyPressEventTargetInfo info){
	m_KeyPressEventTargetReplayInfoMap.set(id, info);
}

bool ForensicReplayDataStore::containsMousePressEventTargetRecordInfo(const String& id){
	return m_MousePressEventTargetRecordInfoMap.contains(id);
}

void ForensicReplayDataStore::setMousePressEventTargetRecordInfo(const String& id, const MousePressEventTargetInfo info){
	m_MousePressEventTargetRecordInfoMap.set(id, info);
}

const MousePressEventTargetInfo ForensicReplayDataStore::getMousePressEventTargetRecordInfo(const String& id){
	return m_MousePressEventTargetRecordInfoMap.get(id);
}

void ForensicReplayDataStore::setMousePressEventTargetReplayInfo(const String& id, const MousePressEventTargetInfo info){
	m_MousePressEventTargetReplayInfoMap.set(id, info);
}

bool ForensicReplayDataStore::containsRequest(const blink::WebURLRequest& req){
	return m_requestMap.contains(req);
}

PassRefPtr<RequestMapEntry> ForensicReplayDataStore::nextRequestEntry(const blink::WebURLRequest& req){
	if(m_requestMap.contains(req)){
		return m_requestMap.get(req)->next();
	}
	return PassRefPtr<RequestMapEntry>();
}

void ForensicReplayDataStore::addFinishedRequest(PassRefPtr<PendingRequestMapEntry> entry){
	String reqStr;
	WebURLRequestHash::toString(entry->m_request, reqStr);
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore: Adding pending request: %u\n%s", WebURLRequestHash::hash(entry->m_request), reqStr.latin1().data());

	blink::WebData data(entry->m_mapEntry->m_data.data(), entry->m_mapEntry->m_data.size());

	if(m_requestMap.contains(entry->m_request)){
		m_requestMap.get(entry->m_request)->add(entry->m_mapEntry->m_response
				, entry->m_mapEntry->m_error
				, data
				, entry->m_mapEntry->m_stackTrace
				, entry->m_mapEntry->finishTime()
				, entry->m_mapEntry->endTime()
				, entry->m_mapEntry->startTime()
				, entry->m_mapEntry->m_totalEncodedDataLength);
	} else {
		m_requestMap.set(entry->m_request,
				RequestMapValue::create(entry->m_mapEntry->m_response
						, entry->m_mapEntry->m_error
						, data
						, entry->m_mapEntry->m_stackTrace
						, entry->m_mapEntry->finishTime()
						, entry->m_mapEntry->endTime()
						, entry->m_mapEntry->startTime()
						, entry->m_mapEntry->m_totalEncodedDataLength));
	}
}

void ForensicReplayDataStore::addFinishedRedirect(
		blink::WebURLRequest& request,
		blink::WebURLRequest& redirectRequest,
		const blink::WebURLResponse& redirectResponse,
		const V8StackTrace& stackTrace,
		double startTime){
	if(m_redirectMap.contains(request)){
		m_redirectMap.get(request)->add(redirectRequest, redirectResponse, stackTrace, startTime);
	} else {
		m_redirectMap.set(request, RedirectMapValue::create(redirectRequest, redirectResponse, stackTrace, startTime));
	}

}

bool ForensicReplayDataStore::containsStackTraceForRequest(const String& key){
	return m_stackTraceMap.contains(key);
}

void ForensicReplayDataStore::clearStackTraceForRequest(const String& key){
	m_stackTraceMap.remove(key);
}

V8StackTrace ForensicReplayDataStore::peekStackTraceForRequest(const String& key){
	V8StackTrace retval;
	if(containsStackTraceForRequest(key)){
		retval.appendVector(m_stackTraceMap.get(key));
	}
	return retval;
}

V8StackTrace ForensicReplayDataStore::getStackTraceForRequest(const String& key){
	V8StackTrace retval = peekStackTraceForRequest(key);
	clearStackTraceForRequest(key);
	return retval;
}

blink::WebURLRequest ForensicReplayDataStore::findMatchingRequest(const String& stackTraceId, double startTime, int sourceMask){
	V8StackTrace stackTrace = getStackTraceForRequest(stackTraceId);
	if(stackTrace.size() > 0){
		String key = stackTrace.serialize()->toJSONString();
		if(m_stackTraceMatchingMap.contains(key)){
			double replayDelta = startTime - replayStartTime();
			double lastdiff = std::numeric_limits<double>::max();
			Vector<V8StackTraceMatchingEntry>* value = m_stackTraceMatchingMap.get(key);

			size_t matchIndex = 0;
			for(size_t i = matchIndex; i < value->size(); i++){ //find min delta diff
				V8StackTraceMatchingEntry& entry = value->at(i);
				if(sourceMask & entry.source()){ //check the mask
					WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- Request start: %f Replay Start: %f Entry Start: %f Record Start: %f", startTime, replayStartTime(), entry.startTime(), recordStartTime());
					double diff =  fabs(replayDelta - (entry.startTime() - recordStartTime()) );
					WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- Calculated diff: %f ", diff);
					if(diff < lastdiff){
						lastdiff = diff;
						matchIndex = i;
					} else {
						break;
					}
				}
			}

			V8StackTraceMatchingEntry& matchEntry = value->at(matchIndex);
			blink::WebURLRequest rec(*(matchEntry.request()));

			WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- Attempting to match %s.", rec.url().string().latin1().c_str());

			//NOTE if below is too restrictive just return rec
			if(matchEntry.source() == V8StackTraceMatchingEntry::Terminal){ //match in terminal requests
				ASSERT(m_requestMap.contains(rec));
				RefPtr<RequestMapEntry> entry = m_requestMap.get(rec)->current();
				if(entry){
					ASSERT(entry->stackTrace());
					if(entry->startTime() == matchEntry.startTime() && entry->stackTrace()->serialize()->toJSONString() == key){ //make sure the current entry is what we matched to
						return rec;
					} else {
						WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- Current terminal entry is not the selected match.");
					}
				} else {
					WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- No more terminal requests.");
				}
			} else { //match in redirects
				ASSERT(m_redirectMap.contains(rec));
				RefPtr<RedirectEntry> entry = m_redirectMap.get(rec)->current();
				if(entry){
					ASSERT(entry->stackTrace());
					if(entry->startTime() == matchEntry.startTime() && entry->stackTrace()->serialize()->toJSONString() == key){ //make sure the current entry is what we matched to
						return rec;
					} else {
						WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- Current redirect entry is not the selected match.");
					}
				} else {
					WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- No more redirect requests.");
				}
			}
			return blink::WebURLRequest(); //our top choice didn't match

		} else { // stack trace not seen before
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- Could not find stacktrace in index: \n%s", key.latin1().data());
		}
	} else { // no stack trace, nothing to match on
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore --STACKTRACE MATCHING-- No stacktrace for id: \n%s", stackTraceId.latin1().data());
	}
	return blink::WebURLRequest();

}

bool ForensicReplayDataStore::containsRedirect(const blink::WebURLRequest& req){
	return m_redirectMap.contains(req);
}

PassRefPtr<RedirectEntry> ForensicReplayDataStore::nextRedirectPair(const blink::WebURLRequest& req){
	if(m_redirectMap.contains(req)){
		return m_redirectMap.get(req)->next();
	}
	return PassRefPtr<RedirectEntry>();
}

const Vector<RefPtr<ForensicEvent> >& ForensicReplayDataStore::forensicEvents(){
	return m_events;
}

void ForensicReplayDataStore::appendForensicEvent(PassRefPtr<ForensicEvent> event){
	m_events.append(event);
}

void ForensicReplayDataStore::appendCurrentTime(double val){
	//m_currentTimeVals.append(val);
	m_currentTimeVals->add(val);
}

//returns negative if no more values
double ForensicReplayDataStore::nextCurrentTime(){
	double* retval = m_currentTimeVals->next();
	if(retval){
		return *retval;
	} else {
		return -1;
	}
}

// same as nextCurrentTime() but does not advance value index
double ForensicReplayDataStore::nextCurrentTimePreview(){
	double* retval = m_currentTimeVals->nextPreview();
	if(retval){
		return *retval;
	} else {
		return -1;
	}
}

double ForensicReplayDataStore::recordStartTime() {
    return m_recordStartTime;
}

double ForensicReplayDataStore::replayStartTime() {
    return m_replayStartTime;
}

void ForensicReplayDataStore::setRecordStartTime(double t) {
    m_recordStartTime = t;
}

void ForensicReplayDataStore::setReplayStartTime(double t) {
    m_replayStartTime = t;
}

double ForensicReplayDataStore::recordStartTimeMonotonic() {
    return m_recordStartTimeMonotonic;
}

double ForensicReplayDataStore::replayStartTimeMonotonic() {
    return m_replayStartTimeMonotonic;
}

void ForensicReplayDataStore::setRecordStartTimeMonotonic(double t) {
    m_recordStartTimeMonotonic = t;
}

void ForensicReplayDataStore::setReplayStartTimeMonotonic(double t) {
    m_replayStartTimeMonotonic = t;
}

double ForensicReplayDataStore::currentTimeOffset(){
	return m_currentTimeOffset;
}

void ForensicReplayDataStore::setCurrentTimeOffset(double currentTimeOffset){
	ASSERT(currentTimeOffset >= 0);
	m_currentTimeOffset = currentTimeOffset;
}

void ForensicReplayDataStore::appendMonotonicallyIncreasingTime(double val){
	m_monotonicallyIncreasingTimeVals->add(val);
}

//returns negative if no more values
double ForensicReplayDataStore::nextMonotonicallyIncreasingTime(){
	double* retval = m_monotonicallyIncreasingTimeVals->next();
	if(retval){
		return *retval;
	} else {
		return -1;
	}
}

//like nextMonotonicallyIncreasingTime(), but does not advance the clock
double ForensicReplayDataStore::nextMonotonicallyIncreasingTimePreview(){
	double* retval = m_monotonicallyIncreasingTimeVals->nextPreview();
	if(retval){
		return *retval;
	} else {
		return -1;
	}
}

double ForensicReplayDataStore::monotonicallyIncreasingTimeOffset(){
	return m_monotonicallyIncreasingTimeOffset;
}

void ForensicReplayDataStore::setMonotonicallyIncreasingTimeOffset(double monotonicallyIncreasingTimeOffset){
    // FIXME: Somethings this assertion is triggered
    // for example when loading a dumped recording trace...
	// ASSERT(monotonicallyIncreasingTimeOffset >= 0);
	m_monotonicallyIncreasingTimeOffset = monotonicallyIncreasingTimeOffset;
    if(m_monotonicallyIncreasingTimeOffset < 0)
        m_monotonicallyIncreasingTimeOffset = 0;
}

void ForensicReplayDataStore::appendCryptographicallyRandomValues(unsigned char* buffer, size_t length){
	PassOwnPtr<blink::WebData> bufferData = adoptPtr(new blink::WebData((const char*)buffer, length));
	if(m_cryptographicallyRandomValuesMap.contains(length)){
		m_cryptographicallyRandomValuesMap.get(length)->add(bufferData);
	}
	else{
		m_cryptographicallyRandomValuesMap.add(length, WebDataVector::create(bufferData));
	}
}

bool ForensicReplayDataStore::nextCryptographicallyRandomValues(unsigned char* buffer, size_t length){
	//XXX: WORKAROUND should be replaced with ASSERT(m_cryptographicallyRandomValuesMap.contains(length));
	if(m_cryptographicallyRandomValuesMap.contains(length)){
		blink::WebData* data = m_cryptographicallyRandomValuesMap.get(length)->next();
		ASSERT(data);
		memcpy(buffer, data->data(), length);
		return true;
	}
	return false;
}

void ForensicReplayDataStore::appendIsLinkVisited(unsigned long long hash, bool retval){
	if(m_isLinkVisitedMap.contains(hash)){
		m_isLinkVisitedMap.get(hash)->add(retval);
	}
	else{
		m_isLinkVisitedMap.add(hash, BoolVector::create(retval));
	}
}

bool ForensicReplayDataStore::nextIsLinkVisited(unsigned long long hash){
	//XXX: WORKAROUND should be replaced with ASSERT(m_isLinkVisitedMap.contains(hash));
	if(!m_isLinkVisitedMap.contains(hash)){
		return false;
	}

	bool* retval = m_isLinkVisitedMap.get(hash)->next();

	//XXX: WORKAROUND should be replaced with ASSERT(retval);
	if(!retval){
		retval = m_isLinkVisitedMap.get(hash)->last();
	}

	return *retval;
}

void ForensicReplayDataStore::appendVisitedLinkHash(const char* url, size_t length, unsigned long long retval){
	String key(url, length);
	if(m_visitedLinkHashMap.contains(key)){
		m_visitedLinkHashMap.get(key)->add(retval);
	}
	else{
		m_visitedLinkHashMap.add(key, ULLVector::create(retval));
	}
}

unsigned long long ForensicReplayDataStore::nextVisitedLinkHash(const char* url, size_t length){
	String key(url, length);

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore::nextVisitedLinkHash (hash=%d) %s", key.impl()->hash(), key.latin1().data());

	//XXX: WORKAROUND should be replaced with ASSERT(m_visitedLinkHashMap.contains(key));
	if(!m_visitedLinkHashMap.contains(key)){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore::nextVisitedLinkHash --no_key-- (hash=%d) %s", key.impl()->hash(), key.latin1().data());
		return 0;
	}

	unsigned long long* retval = m_visitedLinkHashMap.get(key)->next();

	//XXX: WORKAROUND should be replaced with ASSERT(retval);
	if(!retval){
		retval = m_visitedLinkHashMap.get(key)->last();
	}

	return *retval;
}

void ForensicReplayDataStore::appendSignedPublicKeyAndChallengeString(unsigned keySizeIndex,
		const blink::WebString& challenge,
		const blink::WebURL& url,
		blink::WebString& retval)
{
	PublicKeyAndChallengeStringKey key(keySizeIndex, challenge, url);
	PassOwnPtr<blink::WebString> str = adoptPtr(new blink::WebString(retval));
	if(m_signedPublicKeyAndChallengeStringMap.contains(key)){
		m_signedPublicKeyAndChallengeStringMap.get(key)->add(str);
	}
	else{
		m_signedPublicKeyAndChallengeStringMap.add(key, WebStringVector::create(str));
	}
}

blink::WebString ForensicReplayDataStore::nextSignedPublicKeyAndChallengeString(unsigned keySizeIndex, const blink::WebString& challenge, const blink::WebURL& url){
	PublicKeyAndChallengeStringKey key(keySizeIndex, challenge, url);
	ASSERT(m_signedPublicKeyAndChallengeStringMap.contains(key));
	blink::WebString* retval = m_signedPublicKeyAndChallengeStringMap.get(key)->next();
	ASSERT(retval);
	return blink::WebString(*retval);
}


void ForensicReplayDataStore::appendClipboardSequenceNumber(blink::WebClipboard::Buffer buffer, uint64 seq){
	if(m_clipboardSequenceNumberMap.contains(buffer)){
		m_clipboardSequenceNumberMap.get(buffer)->add(seq);
	}
	else{
		m_clipboardSequenceNumberMap.add(buffer, UInt64Vector::create(seq));
	}
}

uint64 ForensicReplayDataStore::nextClipboardSequenceNumber(blink::WebClipboard::Buffer buffer){
	ASSERT(m_clipboardSequenceNumberMap.contains(buffer));
	uint64* seq = m_clipboardSequenceNumberMap.get(buffer)->next();
	ASSERT(seq != 0);
	return *seq;
}

//modal dialog by Bo
void ForensicReplayDataStore::appendModaldialogConfirmResult(const String& message, bool result){
	if(m_modaldialogConfirmResultMap.contains(message)){
		m_modaldialogConfirmResultMap.get(message)->add(result);
	}
	else{
		m_modaldialogConfirmResultMap.add(message, BoolVector::create(result));
	}
}

bool ForensicReplayDataStore::nextModaldialogConfirmResult(const String& message){
	ASSERT(m_modaldialogConfirmResultMap.contains(message));
	bool* result = m_modaldialogConfirmResultMap.get(message)->next();
	ASSERT(result != 0);
	return *result;
}

void ForensicReplayDataStore::appendModaldialogPromptResult(const String& message, const String& result){
	PassOwnPtr<blink::WebString> result1 = adoptPtr(new blink::WebString(result));
	if(m_modaldialogPromptResultMap.contains(message)){
		m_modaldialogPromptResultMap.get(message)->add(result1);
	}
	else{
		m_modaldialogPromptResultMap.add(message, WebStringVector::create(result1));
	}
}

String ForensicReplayDataStore::nextModaldialogPromptResult(const String& message){
	ASSERT(m_modaldialogPromptResultMap.contains(message));
	blink::WebString* result = m_modaldialogPromptResultMap.get(message)->next();
	ASSERT(result != 0);
	return String(*result);
}

void ForensicReplayDataStore::appendModaldialogAlertVals(const String& message){
	PassOwnPtr<blink::WebString> message1 = adoptPtr(new blink::WebString(message));
	m_modaldialogAlertVals->add(message1);
}


void ForensicReplayDataStore::appendClipboardIsFormatAvailable(blink::WebClipboard::Format format, blink::WebClipboard::Buffer buffer, bool retval){
	IsFormatAvailableKey key(format, buffer);
	if(m_clipboardIsFormatAvailableMap.contains(key)){
		m_clipboardIsFormatAvailableMap.get(key)->add(retval);
	}
	else{
		m_clipboardIsFormatAvailableMap.add(key, BoolVector::create(retval));
	}
}

bool ForensicReplayDataStore::nextClipboardIsFormatAvailable(blink::WebClipboard::Format format, blink::WebClipboard::Buffer buffer){
	IsFormatAvailableKey key(format, buffer);
	ASSERT(m_clipboardIsFormatAvailableMap.contains(key));
	bool* retval = m_clipboardIsFormatAvailableMap.get(key)->next();
	ASSERT(retval != 0);
	return *retval;
}

void ForensicReplayDataStore::appendClipboardReadAvailableTypes(blink::WebClipboard::Buffer buffer, bool* containsFilenames, blink::WebVector<blink::WebString>& retval){
	ReadAvailableTypesKey key(buffer, *containsFilenames);

	PassOwnPtr<blink::WebVector<blink::WebString> > vec = adoptPtr(new blink::WebVector<blink::WebString>(retval.size()));
	blink::WebVector<blink::WebString>* vecptr = vec.get();
	for(size_t i = 0; i < retval.size(); i++){
		(*vecptr)[i] = blink::WebString(retval[i]);
	}

	if(m_clipboardReadAvailableTypesMap.contains(key)){
		m_clipboardReadAvailableTypesMap.get(key)->add(vec);
	}
	else{
		m_clipboardReadAvailableTypesMap.add(key, AvailableTypesVector::create(vec));
	}
}

blink::WebVector<blink::WebString> ForensicReplayDataStore::nextClipboardReadAvailableTypes(blink::WebClipboard::Buffer buffer, bool* containsFilenames){
	ReadAvailableTypesKey key(buffer, *containsFilenames);

	ASSERT(m_clipboardReadAvailableTypesMap.contains(key));
	blink::WebVector<blink::WebString>* val = m_clipboardReadAvailableTypesMap.get(key)->next();
	ASSERT(val != 0);

	blink::WebVector<blink::WebString> retval(val->size());
	for(size_t i = 0; i < val->size(); i++){
		retval[i] = blink::WebString((*val)[i]);
	}

	return retval;
}

void ForensicReplayDataStore::appendClipboardReadPlainText(blink::WebClipboard::Buffer buffer, blink::WebString& retval){
	PassOwnPtr<blink::WebString> str = adoptPtr(new blink::WebString(retval));
	if(m_clipboardReadPlainTextMap.contains(buffer)){
		m_clipboardReadPlainTextMap.get(buffer)->add(str);
	}
	else{
		m_clipboardReadPlainTextMap.add(buffer, WebStringVector::create(str));
	}
}

blink::WebString ForensicReplayDataStore::nextClipboardReadPlainText(blink::WebClipboard::Buffer buffer){
	ASSERT(m_clipboardReadPlainTextMap.contains(buffer));
	blink::WebString* retval = m_clipboardReadPlainTextMap.get(buffer)->next();
	ASSERT(retval != 0);
	return blink::WebString(*retval);
}

void ForensicReplayDataStore::appendClipboardReadHTML(blink::WebClipboard::Buffer buffer,
		blink::WebURL* sourceUrl,
		unsigned* fragmentStart,
		unsigned* fragmentEnd,
		blink::WebString& retval){
	ReadHTMLKey key(buffer, *sourceUrl, *fragmentStart, *fragmentEnd);
	PassOwnPtr<blink::WebString> html = adoptPtr(new blink::WebString(retval));
	if(m_clipboardReadHTMLMap.contains(key)){
		m_clipboardReadHTMLMap.get(key)->add(html);
	}
	else{
		m_clipboardReadHTMLMap.add(key, WebStringVector::create(html));
	}
}

blink::WebString ForensicReplayDataStore::nextClipboardReadHTML(blink::WebClipboard::Buffer buffer,
		blink::WebURL* sourceUrl,
		unsigned* fragmentStart,
		unsigned* fragmentEnd){
	ReadHTMLKey key(buffer, *sourceUrl, *fragmentStart, *fragmentEnd);
	ASSERT(m_clipboardReadHTMLMap.contains(key));
	blink::WebString* retval = m_clipboardReadHTMLMap.get(key)->next();
	ASSERT(retval != 0);
	return blink::WebString(*retval);
}

void ForensicReplayDataStore::appendClipboardReadImage(blink::WebClipboard::Buffer buffer, blink::WebData& retval){
	PassOwnPtr<blink::WebData> data = adoptPtr(new blink::WebData(retval));
	if(m_clipboardReadImageMap.contains(buffer)){
		m_clipboardReadImageMap.get(buffer)->add(data);
	}
	else{
		m_clipboardReadImageMap.add(buffer, WebDataVector::create(data));
	}
}

blink::WebData ForensicReplayDataStore::nextClipboardReadImage(blink::WebClipboard::Buffer buffer){
	ASSERT(m_clipboardReadImageMap.contains(buffer));
	blink::WebData* retval = m_clipboardReadImageMap.get(buffer)->next();
	ASSERT(retval != 0);
	return blink::WebData(*retval);
}

void ForensicReplayDataStore::appendClipboardReadCustomData(blink::WebClipboard::Buffer buffer, const blink::WebString& type, blink::WebString& retval){
	CustomDataKey key(buffer, type);
	PassOwnPtr<blink::WebString> str = adoptPtr(new blink::WebString(retval));
	if(m_clipboardReadCustomDataMap.contains(key)){
		m_clipboardReadCustomDataMap.get(key)->add(str);
	}
	else{
		m_clipboardReadCustomDataMap.add(key, WebStringVector::create(str));
	}
}

blink::WebString ForensicReplayDataStore::nextClipboardReadCustomData(blink::WebClipboard::Buffer buffer, const blink::WebString& type){
	CustomDataKey key(buffer, type);
	ASSERT(m_clipboardReadCustomDataMap.contains(key));
	blink::WebString* retval = m_clipboardReadCustomDataMap.get(key)->next();
	ASSERT(retval != 0);
	return blink::WebString(*retval);
}


void ForensicReplayDataStore::appendUserAgent(PassOwnPtr<blink::WebString> val){
	m_userAgentVals->add(val);
}

blink::WebString ForensicReplayDataStore::nextUserAgent()
{
	blink::WebString* retval = m_userAgentVals->next();
	if(retval){
		return blink::WebString(*retval);
	} else {
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore: nextUserAgent out of values.  Using workaround.");
		Vector<OwnPtr<blink::WebString> >::iterator end = m_userAgentVals->end();
		--end;
		blink::WebString* retval = (*end).get();
		return blink::WebString(*retval);
	}
}

void ForensicReplayDataStore::appendMemoryUsageMB(size_t val){
	m_memoryUsageMBVals->add(val);
}

size_t ForensicReplayDataStore::nextMemoryUsageMB(){
	size_t* retval = m_memoryUsageMBVals->next();
	ASSERT(retval);
	return *retval;
}

void ForensicReplayDataStore::appendActualMemoryUsageMB(size_t val){
	m_actualMemoryUsageMBVals->add(val);
}

size_t ForensicReplayDataStore::nextActualMemoryUsageMB(){
	size_t* retval = m_actualMemoryUsageMBVals->next();
	ASSERT(retval);
	return *retval;
}

void ForensicReplayDataStore::appendPhysicalMemoryMB(size_t val){
	m_physicalMemoryMBVals->add(val);
}

size_t ForensicReplayDataStore::nextPhysicalMemoryMB(){
	size_t* retval = m_physicalMemoryMBVals->next();
	ASSERT(retval);
	return *retval;
}

void ForensicReplayDataStore::appendNumberOfProcessors(size_t val){
	m_numberOfProcessorsVals->add(val);
}

size_t ForensicReplayDataStore::nextNumberOfProcessors(){
	size_t* retval = m_numberOfProcessorsVals->next();
	ASSERT(retval);
	return *retval;
}

void ForensicReplayDataStore::appendDefaultLocale(PassOwnPtr<blink::WebString> val){
	m_defaultLocaleVals->add(val);
}

blink::WebString ForensicReplayDataStore::nextDefaultLocale()
{
	blink::WebString* retval = m_defaultLocaleVals->next();
	if(retval){
		return blink::WebString(*retval);
	} else {
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore: nextDefaultLocale out of values.  Using workaround.");
		if(!m_defaultLocaleVals->isEmpty()){
			Vector<OwnPtr<blink::WebString> >::iterator end = m_defaultLocaleVals->end();
			--end;
			blink::WebString* retval = (*end).get();
			return blink::WebString(*retval);
		}
		return blink::WebString("");
	}
}

void ForensicReplayDataStore::clear(){
	ForensicRequestAssembler::clear();
	m_requestMap.clear();
    m_redirectMap.clear();
    m_events.clear();
	m_currentTimeVals->clear();
	m_monotonicallyIncreasingTimeVals->clear();
	m_cryptographicallyRandomValuesMap.clear();
	m_isLinkVisitedMap.clear();
	m_visitedLinkHashMap.clear();
	m_signedPublicKeyAndChallengeStringMap.clear();
	m_clipboardSequenceNumberMap.clear();
	//Modaldialog by Bo
	m_modaldialogConfirmResultMap.clear();
	m_modaldialogPromptResultMap.clear();
	m_modaldialogAlertVals->clear();

	m_clipboardIsFormatAvailableMap.clear();
	m_clipboardReadAvailableTypesMap.clear();
	m_clipboardReadPlainTextMap.clear();
	m_clipboardReadHTMLMap.clear();
	m_clipboardReadImageMap.clear();
	m_clipboardReadCustomDataMap.clear();
	m_KeyPressEventTargetReplayInfoMap.clear();
	m_MousePressEventTargetReplayInfoMap.clear();
	m_userAgentVals->clear();
	m_memoryUsageMBVals->clear();
	m_actualMemoryUsageMBVals->clear();
	m_physicalMemoryMBVals->clear();
	m_numberOfProcessorsVals->clear();

	resetOffSets();
}

void ForensicReplayDataStore::reset(){

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore::reset() : resetting data structures for replay");

	resetMaps();
	resetVectors();
	resetOffSets();
	resetEventState();
}

//Don't test pending map, it stores temp data
bool ForensicReplayDataStore::containsData(){
	return !m_events.isEmpty()
			|| !m_currentTimeVals->isEmpty()
			|| !m_monotonicallyIncreasingTimeVals->isEmpty()
			|| !m_pageURLRequestsMap.isEmpty()
			|| !m_pageURLAliasMap.isEmpty()
			|| !m_requestMap.isEmpty()
			|| !m_redirectMap.isEmpty()
			|| !m_cryptographicallyRandomValuesMap.isEmpty()
			|| !m_isLinkVisitedMap.isEmpty()
			|| !m_visitedLinkHashMap.isEmpty()
			|| !m_signedPublicKeyAndChallengeStringMap.isEmpty()
			|| !m_clipboardSequenceNumberMap.isEmpty()
			/*modaldialog by Bo*/
			|| !m_modaldialogConfirmResultMap.isEmpty()
			|| !m_modaldialogPromptResultMap.isEmpty()
			|| !m_modaldialogAlertVals->isEmpty()
			|| !m_clipboardIsFormatAvailableMap.isEmpty()
			|| !m_clipboardReadAvailableTypesMap.isEmpty()
			|| !m_clipboardReadPlainTextMap.isEmpty()
			|| !m_clipboardReadHTMLMap.isEmpty()
			|| !m_clipboardReadImageMap.isEmpty()
			|| !m_clipboardReadCustomDataMap.isEmpty()
			|| !m_KeyPressEventTargetReplayInfoMap.isEmpty()
			|| !m_MousePressEventTargetReplayInfoMap.isEmpty()
			|| !m_userAgentVals->isEmpty()
			|| !m_memoryUsageMBVals->isEmpty()
			|| !m_actualMemoryUsageMBVals->isEmpty()
			|| !m_numberOfProcessorsVals->isEmpty()
			|| !m_defaultLocaleVals->isEmpty();

}

PassRefPtr<JSONObject> ForensicReplayDataStore::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("type", "ForensicReplayDataStore");
	serializeVectors(retval);
	serializeMaps(retval);
	retval->setNumber("recordStartTime",m_recordStartTime);
	retval->setNumber("recordStartTimeMonotonic",m_recordStartTimeMonotonic);
	return retval;
}

PassRefPtr<JSONObject> ForensicReplayDataStore::serializeReplayInfo(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("type", "ForensicReplayInfo");
	serializeReplayInfoMaps(retval);
	retval->setNumber("replayStartTime",m_replayStartTime);
	return retval;
}

void ForensicReplayDataStore::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	deserializeVectors(guard);
	deserializeMaps(guard);

	bool result = false;

	//these values should always exist in the offloaded data
	result = guard->getNumber("recordStartTime", &m_recordStartTime);
	ASSERT(result);
	result = guard->getNumber("recordStartTimeMonotonic", &m_recordStartTimeMonotonic);
	ASSERT(result);

}

//Don't reset pending map, it isn't used for replay
void ForensicReplayDataStore::resetMaps(){
    resetKeyPressEventTargetReplayInfoMap();
    resetMousePressEventTargetReplayInfoMap();
    resetPageURLRequestsMap();
	resetRequests();
	resetRedirects();
	resetStackTraces();
	resetStackTraceMatchingMap();
	resetCRV();
    resetIsLinkVisitedMap();
    resetVisitedLinkHashMap();
    resetSignedPublicKeyAndChallengeStringMap();
	resetClipboardSequenceNumberMap();
	//modaldialog by Bo
	resetModaldialogConfirmResultMap();
	resetModaldialogPromptResultMap();

    resetClipboardIsFormatAvailableMap();
    resetClipboardReadAvailableTypesMap();
    resetClipboardReadPlainTextMap();
    resetClipboardReadHTMLMap();
    resetClipboardReadImageMap();
    resetClipboardReadCustomDataMap();
}


void ForensicReplayDataStore::resetKeyPressEventTargetReplayInfoMap() {
	m_KeyPressEventTargetReplayInfoMap.clear();
}

void ForensicReplayDataStore::resetMousePressEventTargetReplayInfoMap() {
	m_MousePressEventTargetReplayInfoMap.clear();
}

void ForensicReplayDataStore::resetPageURLRequestsMap() {

	// do not delete the elements of the PageURLRequestsMap
	// we only need to reset the isConsumed flag for each PageURLRequest

	WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicReplayDataStore::resetPageURLRequestsMap : map size = %d", m_pageURLRequestsMap.size());

	PageURLRequestsMap::iterator::Values itStart = m_pageURLRequestsMap.values().begin();
	PageURLRequestsMap::iterator::Values itEnd = m_pageURLRequestsMap.values().end();
	for(PageURLRequestsMap::iterator::Values it=itStart; it!=itEnd; ++it) {
		Vector<RefPtr<PageURLRequest> > v = *it;
		for(unsigned int i=0; i < v.size(); i++) {
			v[i]->setIsConsumed(false);
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore::resetPageURLRequestsMap : RESETTING IS CONSUMED = %d | %s", v[i]->isConsumed(), v[i]->webURLRequest().url().string().latin1().data());
		}
	}
}

void ForensicReplayDataStore::resetRequests(){
	RequestMap::const_iterator::Keys end = m_requestMap.end().keys();
	for (RequestMap::const_iterator::Keys it = m_requestMap.begin().keys(); it != end; ++it){
		m_requestMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetRedirects(){
	RedirectMap::const_iterator::Keys end = m_redirectMap.end().keys();
	for (RedirectMap::const_iterator::Keys it = m_redirectMap.begin().keys(); it != end; ++it){
		m_redirectMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetStackTraces(){
	m_stackTraceMap.clear();
}

void ForensicReplayDataStore::resetStackTraceMatchingMap(){
	m_stackTraceMatchingMap.clear();

	//add all terminal requests
	RequestMap::const_iterator::Keys end = m_requestMap.end().keys();
	for (RequestMap::const_iterator::Keys it = m_requestMap.begin().keys(); it != end; ++it){
		const blink::WebURLRequest* request = &(*it);
		RequestMapValue* value = m_requestMap.get(*it);
		value->reset();
		while(RefPtr<RequestMapEntry> entry = value->next()){
			V8StackTrace* stackTrace = entry->stackTrace();
			if(stackTrace){
				String key = stackTrace->serialize()->toJSONString();

				if(!m_stackTraceMatchingMap.contains(key)){
					m_stackTraceMatchingMap.set(String(key), adoptPtr(new Vector<V8StackTraceMatchingEntry>()));
				}
				m_stackTraceMatchingMap.get(key)->append(V8StackTraceMatchingEntry(request, entry->startTime(), V8StackTraceMatchingEntry::Terminal));
			}
		}
		value->reset();
	}

	//add all redirect requests
	RedirectMap::const_iterator::Keys end2 = m_redirectMap.end().keys();
	for (RedirectMap::const_iterator::Keys it = m_redirectMap.begin().keys(); it != end2; ++it){
		const blink::WebURLRequest* request = &(*it);
		RedirectMapValue* value = m_redirectMap.get(*it);
		value->reset();
		while(RefPtr<RedirectEntry> entry = value->next()){
			V8StackTrace* stackTrace = entry->stackTrace();
			if(stackTrace){
				String key = stackTrace->serialize()->toJSONString();

				if(!m_stackTraceMatchingMap.contains(key)){
					m_stackTraceMatchingMap.set(String(key), adoptPtr(new Vector<V8StackTraceMatchingEntry>()));
				}
				m_stackTraceMatchingMap.get(key)->append(V8StackTraceMatchingEntry(request, entry->startTime(), V8StackTraceMatchingEntry::Redirect));
			}
		}
		value->reset();
	}

	//sort all vectors by startTime
	StackTraceMatchingMap::const_iterator::Keys end3 = m_stackTraceMatchingMap.end().keys();
	for (StackTraceMatchingMap::const_iterator::Keys it = m_stackTraceMatchingMap.begin().keys(); it != end3; ++it){
		Vector<V8StackTraceMatchingEntry>* values = m_stackTraceMatchingMap.get(*it);
		std::sort(values->begin(), values->end());
	}

	WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicReplayDataStore::resetStackTraceMatchingMap: Map Contents %s", serializeStackTraceMatchingMap()->toJSONString().latin1().data());
}

void ForensicReplayDataStore::resetCRV(){
	CRVMap::const_iterator::Keys end = m_cryptographicallyRandomValuesMap.end().keys();
	for (CRVMap::const_iterator::Keys it = m_cryptographicallyRandomValuesMap.begin().keys(); it != end; ++it){
		m_cryptographicallyRandomValuesMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetIsLinkVisitedMap(){
	IsLinkVisitedMap::const_iterator::Keys end = m_isLinkVisitedMap.end().keys();
	for (IsLinkVisitedMap::const_iterator::Keys it = m_isLinkVisitedMap.begin().keys(); it != end; ++it){
		m_isLinkVisitedMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetVisitedLinkHashMap(){
	VisitedLinkHashMap::const_iterator::Keys end = m_visitedLinkHashMap.end().keys();
	for (VisitedLinkHashMap::const_iterator::Keys it = m_visitedLinkHashMap.begin().keys(); it != end; ++it){
		m_visitedLinkHashMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetSignedPublicKeyAndChallengeStringMap(){
	SignedPublicKeyAndChallengeStringMap::const_iterator::Keys end = m_signedPublicKeyAndChallengeStringMap.end().keys();
	for (SignedPublicKeyAndChallengeStringMap::const_iterator::Keys it = m_signedPublicKeyAndChallengeStringMap.begin().keys(); it != end; ++it){
		m_signedPublicKeyAndChallengeStringMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetClipboardSequenceNumberMap(){
	ClipboardSequenceNumberMap::const_iterator::Keys end = m_clipboardSequenceNumberMap.end().keys();
	for (ClipboardSequenceNumberMap::const_iterator::Keys it = m_clipboardSequenceNumberMap.begin().keys(); it != end; ++it){
		m_clipboardSequenceNumberMap.get(*it)->reset();
	}
}

//Modaldialog by Bo
void ForensicReplayDataStore::resetModaldialogConfirmResultMap(){
	ModaldialogConfirmResultMap::const_iterator::Keys end = m_modaldialogConfirmResultMap.end().keys();
	for (ModaldialogConfirmResultMap::const_iterator::Keys it = m_modaldialogConfirmResultMap.begin().keys(); it != end; ++it){
		m_modaldialogConfirmResultMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetModaldialogPromptResultMap(){
	ModaldialogPromptResultMap::const_iterator::Keys end = m_modaldialogPromptResultMap.end().keys();
	for (ModaldialogPromptResultMap::const_iterator::Keys it = m_modaldialogPromptResultMap.begin().keys(); it != end; ++it){
		m_modaldialogPromptResultMap.get(*it)->reset();
	}
}
void ForensicReplayDataStore::resetClipboardIsFormatAvailableMap(){
	ClipboardIsFormatAvailableMap::const_iterator::Keys end = m_clipboardIsFormatAvailableMap.end().keys();
	for (ClipboardIsFormatAvailableMap::const_iterator::Keys it = m_clipboardIsFormatAvailableMap.begin().keys(); it != end; ++it){
		m_clipboardIsFormatAvailableMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetClipboardReadAvailableTypesMap(){
	ClipboardReadAvailableTypesMap::const_iterator::Keys end = m_clipboardReadAvailableTypesMap.end().keys();
	for (ClipboardReadAvailableTypesMap::const_iterator::Keys it = m_clipboardReadAvailableTypesMap.begin().keys(); it != end; ++it){
		m_clipboardReadAvailableTypesMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetClipboardReadPlainTextMap(){
	ClipboardReadPlainTextMap::const_iterator::Keys end = m_clipboardReadPlainTextMap.end().keys();
	for (ClipboardReadPlainTextMap::const_iterator::Keys it = m_clipboardReadPlainTextMap.begin().keys(); it != end; ++it){
		m_clipboardReadPlainTextMap.get(*it)->reset();
	}
}


void ForensicReplayDataStore::resetClipboardReadHTMLMap(){
	ClipboardReadHTMLMap::const_iterator::Keys end = m_clipboardReadHTMLMap.end().keys();
	for (ClipboardReadHTMLMap::const_iterator::Keys it = m_clipboardReadHTMLMap.begin().keys(); it != end; ++it){
		m_clipboardReadHTMLMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetClipboardReadImageMap(){
	ClipboardReadImageMap::const_iterator::Keys end = m_clipboardReadImageMap.end().keys();
	for (ClipboardReadImageMap::const_iterator::Keys it = m_clipboardReadImageMap.begin().keys(); it != end; ++it){
		m_clipboardReadImageMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetClipboardReadCustomDataMap(){
	ClipboardReadCustomDataMap::const_iterator::Keys end = m_clipboardReadCustomDataMap.end().keys();
	for (ClipboardReadCustomDataMap::const_iterator::Keys it = m_clipboardReadCustomDataMap.begin().keys(); it != end; ++it){
		m_clipboardReadCustomDataMap.get(*it)->reset();
	}
}

void ForensicReplayDataStore::resetVectors(){
	m_currentTimeVals->reset();
	m_monotonicallyIncreasingTimeVals->reset();
	m_userAgentVals->reset();
	m_memoryUsageMBVals->reset();
	m_actualMemoryUsageMBVals->reset();
	m_physicalMemoryMBVals->reset();
	m_numberOfProcessorsVals->reset();
	m_defaultLocaleVals->reset();
	//Modaldialog by Bo
	m_modaldialogAlertVals->reset();
}

void ForensicReplayDataStore::resetOffSets(){
	setCurrentTimeOffset(0);
	setMonotonicallyIncreasingTimeOffset(0);
}

void ForensicReplayDataStore::resetEventState(){
	Vector<RefPtr<ForensicEvent> >::iterator it;
	for(it = m_events.begin(); it != m_events.end(); ++it){
		(*it)->ready();
	}
}

void ForensicReplayDataStore::markPageURLRequestAsConsumed(String& pageURL) {
	// XXX: we replaced the ASSERT below with an IF statement,
	// because replay incongruence may cause an attempt to load a URL into
	// a main frame which was never recorded (e.g., a click replayed on the wrong element
	// may cause a never see before pageURL to be requested for a main frame)
	// ASSERT(pageURLRequestsMap().contains(pageURL));
	if(!m_pageURLRequestsMap.contains(pageURL)){
		return;
	}

	// FIXME: there probably exists a more efficient way to append a PageURLRequest to the following HashMap value Vector
	Vector<RefPtr<PageURLRequest> > v = m_pageURLRequestsMap.get(pageURL);
	ASSERT(v.size()>0); // if pageURL is in the map, then there must be at least one PageURLRequest associated to it
	v.first()->setIsConsumed(true); // mark the PageURLRequest as consumed

	/* We should not remove elements, if we want to enable multiple replays of the same recording
	v.remove(0); // remove the first still available PageURLRequest for pageURL
	if(v.size() > 0)
		pageURLRequestsMap().set(pageURL, v);
	else
		pageURLRequestsMap().remove(pageURL);
	*/
}

void ForensicReplayDataStore::serializeRequests(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_requestMap.isEmpty()){
		RequestMap::const_iterator::Keys end = m_requestMap.end().keys();
		for (RequestMap::const_iterator::Keys it = m_requestMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setObject(JSON_KEY, SerializationUtils::serialize(*it));
			mapValue->setArray(JSON_VALUE, m_requestMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("requests", mapValues);
}

void ForensicReplayDataStore::deserializeRequests(PassRefPtr<JSONObject> json){
	if(!m_requestMap.isEmpty()){
		m_requestMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("requests");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			RefPtr<JSONObject> keyJSON = mapValue->getObject(JSON_KEY);
			ASSERT(keyJSON);

			OwnPtr<RequestMapValue> value = RequestMapValue::create();
			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_requestMap.add(blink::WrappedResourceRequest(*(SerializationUtils::deserializeRequest(keyJSON).leakPtr())), value.release());
		}
	}
}

void ForensicReplayDataStore::serializeRedirects(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_redirectMap.isEmpty()){
		RedirectMap::const_iterator::Keys end = m_redirectMap.end().keys();
		for (RedirectMap::const_iterator::Keys it = m_redirectMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setObject(JSON_KEY, SerializationUtils::serialize(*it));
			mapValue->setArray(JSON_VALUE, m_redirectMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("redirects", mapValues);
}

void ForensicReplayDataStore::deserializeRedirects(PassRefPtr<JSONObject> json){
	if(!m_redirectMap.isEmpty()){
		m_redirectMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("redirects");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			RefPtr<JSONObject> keyJSON = mapValue->getObject(JSON_KEY);
			ASSERT(keyJSON);

			OwnPtr<RedirectMapValue> value = RedirectMapValue::create();
			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_redirectMap.add(blink::WrappedResourceRequest(*(SerializationUtils::deserializeRequest(keyJSON).leakPtr())), value.release());
		}
	}
}

void ForensicReplayDataStore::serializeKeyPressEventTargetMap(PassRefPtr<JSONObject> json, const KeyPressEventTargetInfoMap& map, const String& state){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!map.isEmpty()){
		KeyPressEventTargetInfoMap::const_iterator::Keys end = map.end().keys();
		for (KeyPressEventTargetInfoMap::const_iterator::Keys it = map.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setString(JSON_KEY, *it);
			mapValue->setObject(JSON_VALUE, map.get(*it).serialize());
			mapValues->pushObject(mapValue);
		}
	}
	if(state=="record")
		json->setArray("keyPressEventTargetRecordInfoMap", mapValues);
	else if(state=="replay")
		json->setArray("keyPressEventTargetReplayInfoMap", mapValues);
}

void ForensicReplayDataStore::deserializeKeyPressEventTargetRecordInfoMap(PassRefPtr<JSONObject> json){
	if(!m_KeyPressEventTargetRecordInfoMap.isEmpty()){
		m_KeyPressEventTargetRecordInfoMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("keyPressEventTargetRecordInfoMap");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			bool result;
			String key;
			result = mapValue->getString(JSON_KEY, &key);
			ASSERT(result);

			RefPtr<JSONObject> valueJSON = mapValue->getObject(JSON_VALUE);
			ASSERT(valueJSON);

			KeyPressEventTargetInfo value;
			value.deserialize(valueJSON);

			m_KeyPressEventTargetRecordInfoMap.add(key, value);
		}
	}
}

void ForensicReplayDataStore::serializeMousePressEventTargetMap(PassRefPtr<JSONObject> json, const MousePressEventTargetInfoMap& map, const String& state){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!map.isEmpty()){
		MousePressEventTargetInfoMap::const_iterator::Keys end = map.end().keys();
		for (MousePressEventTargetInfoMap::const_iterator::Keys it = map.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setString(JSON_KEY, *it);
			mapValue->setObject(JSON_VALUE, map.get(*it).serialize());
			mapValues->pushObject(mapValue);
		}
	}
	if(state=="record")
		json->setArray("mousePressEventTargetRecordInfoMap", mapValues);
	else if(state=="replay")
		json->setArray("mousePressEventTargetReplayInfoMap", mapValues);
}

void ForensicReplayDataStore::deserializeMousePressEventTargetRecordInfoMap(PassRefPtr<JSONObject> json){
	if(!m_MousePressEventTargetRecordInfoMap.isEmpty()){
		m_MousePressEventTargetRecordInfoMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("mousePressEventTargetRecordInfoMap");
	if(mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			bool result;
			String key;
			result = mapValue->getString(JSON_KEY, &key);
			ASSERT(result);

			RefPtr<JSONObject> valueJSON = mapValue->getObject(JSON_VALUE);
			ASSERT(valueJSON);

			MousePressEventTargetInfo value;
			value.deserialize(valueJSON);

			m_MousePressEventTargetRecordInfoMap.add(key, value);
		}
	}
}

//StackTraceMatchingMap m_stackTraceMatchingMap;
PassRefPtr<JSONArray>  ForensicReplayDataStore::serializeStackTraceMatchingMap(){
	RefPtr<JSONArray> retval = JSONArray::create();
	StackTraceMatchingMap::const_iterator::Keys end = m_stackTraceMatchingMap.end().keys();
	for (StackTraceMatchingMap::const_iterator::Keys it = m_stackTraceMatchingMap.begin().keys(); it != end; ++it){
		RefPtr<JSONObject> mapValue = JSONObject::create();
		//WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore::serializeStackTraceMatchingMap: Key %s", (*it).latin1().data());

		String key = *it;
		mapValue->setString(JSON_KEY, key);

		RefPtr<JSONArray> valueJSON = JSONArray::create();
		Vector<V8StackTraceMatchingEntry>* value = m_stackTraceMatchingMap.get(key);
		//WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore::serializeStackTraceMatchingMap: Value length %i", value->size());
		Vector<V8StackTraceMatchingEntry>::iterator end2 = value->end();
		for(Vector<V8StackTraceMatchingEntry>::iterator it2 = value->begin(); it2 != end2; ++it2){
			valueJSON->pushObject((*it2).serialize());
		}
		mapValue->setArray(JSON_VALUE, valueJSON);
		retval->pushObject(mapValue);
	}
	return retval;
}

void ForensicReplayDataStore::serializePageURLAliasMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_pageURLAliasMap.isEmpty()){
		PageURLAliasMap::const_iterator::Keys end = m_pageURLAliasMap.end().keys();
		for (PageURLAliasMap::const_iterator::Keys it = m_pageURLAliasMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setString(JSON_KEY, *it);
			mapValue->setString(JSON_VALUE, m_pageURLAliasMap.get(*it));
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("pageURLAliasMap", mapValues);
}

void ForensicReplayDataStore::deserializePageURLAliasMap(PassRefPtr<JSONObject> json){
	if(!m_pageURLAliasMap.isEmpty()){
		m_pageURLAliasMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("pageURLAliasMap");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			bool result;
			String key, value;
			result = mapValue->getString(JSON_KEY, &key);
			ASSERT(result);

			result = mapValue->getString(JSON_VALUE, &value);
			ASSERT(result);

			m_pageURLAliasMap.add(key, value);
		}
	}
}

void ForensicReplayDataStore::serializedPageURLRequestsMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_pageURLRequestsMap.isEmpty()){
		PageURLRequestsMap::const_iterator::Keys end = m_pageURLRequestsMap.end().keys();
		for (PageURLRequestsMap::const_iterator::Keys it = m_pageURLRequestsMap.begin().keys(); it != end; ++it){

			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setString(JSON_KEY, *it);

			const Vector<RefPtr<PageURLRequest> >& values = m_pageURLRequestsMap.get(*it);
			RefPtr<JSONArray> valuesJSON = JSONArray::create();
			Vector<RefPtr<PageURLRequest> >::const_iterator end2 = values.end();
			for(Vector<RefPtr<PageURLRequest> >::const_iterator it2 = values.begin(); it2 != end2; ++it2){
				valuesJSON->pushObject((*it2)->serialize());
			}

			mapValue->setArray(JSON_VALUE, valuesJSON);
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("pageURLRequests", mapValues);
}

void ForensicReplayDataStore::deserializedPageURLRequestsMap(PassRefPtr<JSONObject> json){
	if(!m_pageURLRequestsMap.isEmpty()){
		m_pageURLRequestsMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("pageURLRequests");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			bool result;
			String key;
			result = mapValue->getString(JSON_KEY, &key);
			ASSERT(result);

			Vector<RefPtr<PageURLRequest> > values;
			RefPtr<JSONArray> valuesJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valuesJSON);

			for(size_t i = 0; i < valuesJSON->length(); i++){
				RefPtr<JSONObject> valueJSON = valuesJSON->get(i)->asObject();
				ASSERT(valueJSON);

				RefPtr<PageURLRequest> value = PageURLRequest::createEmpty();
				value->deserialize(valueJSON);
				values.append(value);
			}

			m_pageURLRequestsMap.add(key, values);
		}
	}
}

void ForensicReplayDataStore::serializeCRV(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_cryptographicallyRandomValuesMap.isEmpty()){
		CRVMap::const_iterator::Keys end = m_cryptographicallyRandomValuesMap.end().keys();
		for (CRVMap::const_iterator::Keys it = m_cryptographicallyRandomValuesMap.begin().keys(); it != end; ++it){

			RefPtr<JSONObject> mapValue = JSONObject::create(); //key,values pair
			mapValue->setNumber(JSON_KEY, *it);
			mapValue->setArray(JSON_VALUE, m_cryptographicallyRandomValuesMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("cryptographicallyRandomValues", mapValues);
}

void ForensicReplayDataStore::deserializeCRV(PassRefPtr<JSONObject> json){
	if(!m_cryptographicallyRandomValuesMap.isEmpty()){
		m_cryptographicallyRandomValuesMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("cryptographicallyRandomValues");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			size_t key;
			OwnPtr<WebDataVector> value = WebDataVector::create();

			bool result = mapValue->getNumber(JSON_KEY, &key);
			ASSERT(result);
			//key = temp;

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_cryptographicallyRandomValuesMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeIsLinkVisitedMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_isLinkVisitedMap.isEmpty()){
		IsLinkVisitedMap::const_iterator::Keys end = m_isLinkVisitedMap.end().keys();
		for (IsLinkVisitedMap::const_iterator::Keys it = m_isLinkVisitedMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setNumber(JSON_KEY, *it); //XXX warning: unsigned long long to double conversion
			mapValue->setArray(JSON_VALUE, m_isLinkVisitedMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("isLinkVisited", mapValues);
}

void ForensicReplayDataStore::deserializeIsLinkVisitedMap(PassRefPtr<JSONObject> json){
	if(!m_isLinkVisitedMap.isEmpty()){
		m_isLinkVisitedMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("isLinkVisited");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			double temp;
			unsigned long long key;
			OwnPtr<BoolVector> value = BoolVector::create();

			bool result = mapValue->getNumber(JSON_KEY, &temp);
			ASSERT(result);
			key = temp;

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_isLinkVisitedMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeVisitedLinkHashMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_visitedLinkHashMap.isEmpty()){
		VisitedLinkHashMap::const_iterator::Keys end = m_visitedLinkHashMap.end().keys();
		for (VisitedLinkHashMap::const_iterator::Keys it = m_visitedLinkHashMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setString(JSON_KEY, *it);
			mapValue->setArray(JSON_VALUE, m_visitedLinkHashMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("visitedLinkHash", mapValues);
}

void ForensicReplayDataStore::deserializeVisitedLinkHashMap(PassRefPtr<JSONObject> json){
	if(!m_visitedLinkHashMap.isEmpty()){
		m_visitedLinkHashMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("visitedLinkHash");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			String key;
			OwnPtr<ULLVector> value = ULLVector::create();

			bool result = mapValue->getString(JSON_KEY, &key);
			ASSERT(result);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_visitedLinkHashMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeSignedPublicKeyAndChallengeStringMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_signedPublicKeyAndChallengeStringMap.isEmpty()){
		SignedPublicKeyAndChallengeStringMap::const_iterator::Keys end = m_signedPublicKeyAndChallengeStringMap.end().keys();
		for (SignedPublicKeyAndChallengeStringMap::const_iterator::Keys it = m_signedPublicKeyAndChallengeStringMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			GenericKey key = (*it);
			mapValue->setObject(JSON_KEY, key.serialize());
			mapValue->setArray(JSON_VALUE, m_signedPublicKeyAndChallengeStringMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("signedPublicKeyAndChallengeString", mapValues);
}

void ForensicReplayDataStore::deserializeSignedPublicKeyAndChallengeStringMap(PassRefPtr<JSONObject> json){
	if(!m_signedPublicKeyAndChallengeStringMap.isEmpty()){
		m_signedPublicKeyAndChallengeStringMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("signedPublicKeyAndChallengeString");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			PublicKeyAndChallengeStringKey key;
			OwnPtr<WebStringVector> value = WebStringVector::create();

			RefPtr<JSONObject> keyJSON = mapValue->getObject(JSON_KEY);
			ASSERT(keyJSON);
			key.deserialize(keyJSON);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_signedPublicKeyAndChallengeStringMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeClipboardSequenceNumberMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_clipboardSequenceNumberMap.isEmpty()){
		ClipboardSequenceNumberMap::const_iterator::Keys end = m_clipboardSequenceNumberMap.end().keys();
		for (ClipboardSequenceNumberMap::const_iterator::Keys it = m_clipboardSequenceNumberMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setNumber(JSON_KEY, *it);
			mapValue->setArray(JSON_VALUE, m_clipboardSequenceNumberMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("clipboardSequenceNumber", mapValues);
}

void ForensicReplayDataStore::deserializeClipboardSequenceNumberMap(PassRefPtr<JSONObject> json){
	if(!m_clipboardSequenceNumberMap.isEmpty()){
		m_clipboardSequenceNumberMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("clipboardSequenceNumber");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			int key;
			OwnPtr<UInt64Vector> value = UInt64Vector::create();

			bool result = mapValue->getNumber(JSON_KEY, &key);
			ASSERT(result);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_clipboardSequenceNumberMap.add(key, value.release());
		}
	}
}
//modaldialog by Bo
void ForensicReplayDataStore::serializeModaldialogConfirmResultMap(PassRefPtr<JSONObject> json){

	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_modaldialogConfirmResultMap.isEmpty()){
		ModaldialogConfirmResultMap::const_iterator::Keys end = m_modaldialogConfirmResultMap.end().keys();
		for (ModaldialogConfirmResultMap::const_iterator::Keys it = m_modaldialogConfirmResultMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setString(JSON_KEY, *it);
			mapValue->setArray(JSON_VALUE, m_modaldialogConfirmResultMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("modaldialogConfirmResult", mapValues);

}

void ForensicReplayDataStore::deserializeModaldialogConfirmResultMap(PassRefPtr<JSONObject> json){


	if(!m_modaldialogConfirmResultMap.isEmpty()){
		m_modaldialogConfirmResultMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("modaldialogConfirmResult");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			String key;
			OwnPtr<BoolVector> value = BoolVector::create();

			bool result = mapValue->getString(JSON_KEY, &key);
			ASSERT(result);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_modaldialogConfirmResultMap.add(key, value.release());
		}
	}

}

void ForensicReplayDataStore::serializeModaldialogPromptResultMap(PassRefPtr<JSONObject> json){

	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_modaldialogPromptResultMap.isEmpty()){
		ModaldialogPromptResultMap::const_iterator::Keys end = m_modaldialogPromptResultMap.end().keys();
		for (ModaldialogPromptResultMap::const_iterator::Keys it = m_modaldialogPromptResultMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setString(JSON_KEY, *it);
			mapValue->setArray(JSON_VALUE, m_modaldialogPromptResultMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("modaldialogPromptResult", mapValues);

}

void ForensicReplayDataStore::deserializeModaldialogPromptResultMap(PassRefPtr<JSONObject> json){


	if(!m_modaldialogPromptResultMap.isEmpty()){
		m_modaldialogPromptResultMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("modaldialogPromptResult");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			String key;
			OwnPtr<WebStringVector> value = WebStringVector::create();

			bool result = mapValue->getString(JSON_KEY, &key);
			ASSERT(result);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_modaldialogPromptResultMap.add(key, value.release());
		}
	}

}

void ForensicReplayDataStore::serializeClipboardIsFormatAvailableMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_clipboardIsFormatAvailableMap.isEmpty()){
		ClipboardIsFormatAvailableMap::const_iterator::Keys end = m_clipboardIsFormatAvailableMap.end().keys();
		for (ClipboardIsFormatAvailableMap::const_iterator::Keys it = m_clipboardIsFormatAvailableMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			GenericKey key = (*it);
			mapValue->setObject(JSON_KEY, key.serialize());
			mapValue->setArray(JSON_VALUE, m_clipboardIsFormatAvailableMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("clipboardIsFormatAvailable", mapValues);
}

void ForensicReplayDataStore::deserializeClipboardIsFormatAvailableMap(PassRefPtr<JSONObject> json){
	if(!m_clipboardIsFormatAvailableMap.isEmpty()){
		m_clipboardIsFormatAvailableMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("clipboardIsFormatAvailable");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			IsFormatAvailableKey key;
			OwnPtr<BoolVector> value = BoolVector::create();

			RefPtr<JSONObject> keyJSON = mapValue->getObject(JSON_KEY);
			ASSERT(keyJSON);
			key.deserialize(keyJSON);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_clipboardIsFormatAvailableMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeClipboardReadAvailableTypesMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_clipboardReadAvailableTypesMap.isEmpty()){
		ClipboardReadAvailableTypesMap::const_iterator::Keys end = m_clipboardReadAvailableTypesMap.end().keys();
		for (ClipboardReadAvailableTypesMap::const_iterator::Keys it = m_clipboardReadAvailableTypesMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			GenericKey key = (*it);
			mapValue->setObject(JSON_KEY, key.serialize());
			mapValue->setArray(JSON_VALUE, m_clipboardReadAvailableTypesMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("clipboardReadAvailableTypes", mapValues);
}

void ForensicReplayDataStore::deserializeClipboardReadAvailableTypesMap(PassRefPtr<JSONObject> json){
	if(!m_clipboardReadAvailableTypesMap.isEmpty()){
		m_clipboardReadAvailableTypesMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("clipboardReadAvailableTypes");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			ReadAvailableTypesKey key;
			OwnPtr<AvailableTypesVector> value = AvailableTypesVector::create();

			RefPtr<JSONObject> keyJSON = mapValue->getObject(JSON_KEY);
			ASSERT(keyJSON);
			key.deserialize(keyJSON);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_clipboardReadAvailableTypesMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeClipboardReadPlainTextMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_clipboardReadPlainTextMap.isEmpty()){
		ClipboardReadPlainTextMap::const_iterator::Keys end = m_clipboardReadPlainTextMap.end().keys();
		for (ClipboardReadPlainTextMap::const_iterator::Keys it = m_clipboardReadPlainTextMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setNumber(JSON_KEY, *it);
			mapValue->setArray(JSON_VALUE, m_clipboardReadPlainTextMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("clipboardReadPlainText", mapValues);
}

void ForensicReplayDataStore::deserializeClipboardReadPlainTextMap(PassRefPtr<JSONObject> json){
	if(!m_clipboardReadPlainTextMap.isEmpty()){
		m_clipboardReadPlainTextMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("clipboardReadPlainText");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			int key;
			OwnPtr<WebStringVector> value = WebStringVector::create();

			bool result = mapValue->getNumber(JSON_KEY, &key);
			ASSERT(result);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_clipboardReadPlainTextMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeClipboardReadHTMLMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_clipboardReadHTMLMap.isEmpty()){
		ClipboardReadHTMLMap::const_iterator::Keys end = m_clipboardReadHTMLMap.end().keys();
		for (ClipboardReadHTMLMap::const_iterator::Keys it = m_clipboardReadHTMLMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			GenericKey key = (*it);
			mapValue->setObject(JSON_KEY, key.serialize());
			mapValue->setArray(JSON_VALUE, m_clipboardReadHTMLMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("clipboardReadHTML", mapValues);
}

void ForensicReplayDataStore::deserializeClipboardReadHTMLMap(PassRefPtr<JSONObject> json){
	if(!m_clipboardReadHTMLMap.isEmpty()){
		m_clipboardReadHTMLMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("clipboardReadHTML");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			ReadHTMLKey key;
			OwnPtr<WebStringVector> value = WebStringVector::create();

			RefPtr<JSONObject> keyJSON = mapValue->getObject(JSON_KEY);
			ASSERT(keyJSON);
			key.deserialize(keyJSON);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_clipboardReadHTMLMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeClipboardReadImageMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_clipboardReadImageMap.isEmpty()){
		ClipboardReadImageMap::const_iterator::Keys end = m_clipboardReadImageMap.end().keys();
		for (ClipboardReadImageMap::const_iterator::Keys it = m_clipboardReadImageMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			mapValue->setNumber(JSON_KEY, *it);
			mapValue->setArray(JSON_VALUE, m_clipboardReadImageMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("clipboardReadImage", mapValues);
}

void ForensicReplayDataStore::deserializeClipboardReadImageMap(PassRefPtr<JSONObject> json){
	if(!m_clipboardReadImageMap.isEmpty()){
		m_clipboardReadImageMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("clipboardReadImage");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			int key;
			OwnPtr<WebDataVector> value = WebDataVector::create();

			bool result = mapValue->getNumber(JSON_KEY, &key);
			ASSERT(result);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_clipboardReadImageMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeClipboardReadCustomDataMap(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> mapValues = JSONArray::create();
	if(!m_clipboardReadCustomDataMap.isEmpty()){
		ClipboardReadCustomDataMap::const_iterator::Keys end = m_clipboardReadCustomDataMap.end().keys();
		for (ClipboardReadCustomDataMap::const_iterator::Keys it = m_clipboardReadCustomDataMap.begin().keys(); it != end; ++it){
			RefPtr<JSONObject> mapValue = JSONObject::create();
			GenericKey key = (*it);
			mapValue->setObject(JSON_KEY, key.serialize());
			mapValue->setArray(JSON_VALUE, m_clipboardReadCustomDataMap.get(*it)->serialize());
			mapValues->pushObject(mapValue);
		}
	}
	json->setArray("clipboardReadCustomData", mapValues);
}

void ForensicReplayDataStore::deserializeClipboardReadCustomDataMap(PassRefPtr<JSONObject> json){
	if(!m_clipboardReadCustomDataMap.isEmpty()){
		m_clipboardReadCustomDataMap.clear();
	}

	RefPtr<JSONArray> mapValues = json->getArray("clipboardReadCustomData");
	if (mapValues){
		for(size_t i = 0; i < mapValues->length(); i++){
			RefPtr<JSONObject> mapValue = mapValues->get(i)->asObject();
			ASSERT(mapValue);

			CustomDataKey key;
			OwnPtr<WebStringVector> value = WebStringVector::create();

			RefPtr<JSONObject> keyJSON = mapValue->getObject(JSON_KEY);
			ASSERT(keyJSON);
			key.deserialize(keyJSON);

			RefPtr<JSONArray> valueJSON = mapValue->getArray(JSON_VALUE);
			ASSERT(valueJSON);
			value->deserialize(valueJSON);

			m_clipboardReadCustomDataMap.add(key, value.release());
		}
	}
}

void ForensicReplayDataStore::serializeMaps(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	serializeRequests(guard);
	serializeRedirects(guard);
	serializeKeyPressEventTargetMap(guard, m_KeyPressEventTargetRecordInfoMap, String("record"));
	serializeMousePressEventTargetMap(guard, m_MousePressEventTargetRecordInfoMap, String("record"));
	serializePageURLAliasMap(guard);
	serializedPageURLRequestsMap(guard);
	serializeCRV(guard);
    serializeIsLinkVisitedMap(guard);
    serializeVisitedLinkHashMap(guard);
    serializeSignedPublicKeyAndChallengeStringMap(guard);
	serializeClipboardSequenceNumberMap(guard);
	//modaldialog by Bo
	serializeModaldialogConfirmResultMap(guard);
	serializeModaldialogPromptResultMap(guard);
    serializeClipboardIsFormatAvailableMap(guard);
    serializeClipboardReadAvailableTypesMap(guard);
    serializeClipboardReadPlainTextMap(guard);
    serializeClipboardReadHTMLMap(guard);
    serializeClipboardReadImageMap(guard);
    serializeClipboardReadCustomDataMap(guard);
}

void ForensicReplayDataStore::serializeReplayInfoMaps(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	serializeKeyPressEventTargetMap(guard, m_KeyPressEventTargetRecordInfoMap, String("record"));
	serializeKeyPressEventTargetMap(guard, m_KeyPressEventTargetReplayInfoMap, String("replay"));
	serializeMousePressEventTargetMap(guard, m_MousePressEventTargetRecordInfoMap, String("record"));
	serializeMousePressEventTargetMap(guard, m_MousePressEventTargetReplayInfoMap, String("replay"));
}


void ForensicReplayDataStore::serializeVectors(PassRefPtr<JSONObject> json){
	RefPtr<JSONArray> eventsJSON = JSONArray::create();
	if(!m_events.isEmpty()){
		Vector<RefPtr<ForensicEvent> >::iterator it;
		for(it = m_events.begin(); it != m_events.end(); ++it){
			eventsJSON->pushObject((*it)->serialize());
		}
	}
	json->setArray("events", eventsJSON);

	json->setArray("currentTimeVals", m_currentTimeVals->serialize());
	json->setArray("monotonicallyIncreasingTimeVals", m_monotonicallyIncreasingTimeVals->serialize());
	json->setArray("userAgentVals", m_userAgentVals->serialize());
	json->setArray("memoryUsageMBVals", m_memoryUsageMBVals->serialize());
	json->setArray("actualMemoryUsageMBVals", m_actualMemoryUsageMBVals->serialize());
	json->setArray("physicalMemoryMBVals", m_physicalMemoryMBVals->serialize());
	json->setArray("numberOfProcessorsVals", m_numberOfProcessorsVals->serialize());
	json->setArray("defaultLocaleVals", m_defaultLocaleVals->serialize());

	//Modaldialog by Bo
	json->setArray("modaldialogAlertVals", m_modaldialogAlertVals->serialize());

}

void ForensicReplayDataStore::deserializeMaps(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	deserializeRequests(guard);
	deserializeRedirects(guard);
	deserializeKeyPressEventTargetRecordInfoMap(guard);
	deserializeMousePressEventTargetRecordInfoMap(guard);
	deserializePageURLAliasMap(guard);
	deserializedPageURLRequestsMap(guard);
	deserializeCRV(guard);
	deserializeIsLinkVisitedMap(guard);
	deserializeVisitedLinkHashMap(guard);
	deserializeSignedPublicKeyAndChallengeStringMap(guard);
	deserializeClipboardSequenceNumberMap(guard);
	//Modaldialog by Bo
	deserializeModaldialogConfirmResultMap(guard);
	deserializeModaldialogPromptResultMap(guard);
	deserializeClipboardIsFormatAvailableMap(guard);
	deserializeClipboardReadAvailableTypesMap(guard);
	deserializeClipboardReadPlainTextMap(guard);
	deserializeClipboardReadHTMLMap(guard);
	deserializeClipboardReadImageMap(guard);
	deserializeClipboardReadCustomDataMap(guard);
}

void ForensicReplayDataStore::deserializeVector(PassRefPtr<JSONObject> json, const String& key, SerializableVector& buf){
	WebCapsule::log(WebCapsule::DebugLogLevel,"Deserializing Vector: %s", key.latin1().data());
	RefPtr<JSONArray> jsonValues = json->getArray(key);
	if (jsonValues){
		buf.deserialize(jsonValues);
	}
}

void ForensicReplayDataStore::deserializeVectors(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	WebCapsule::log(WebCapsule::DebugLogLevel,"Deserializing Vector: %s", "events");
	RefPtr<JSONArray> eventsJSON = guard->getArray("events");
	ASSERT(eventsJSON); //we record at least one event in every recording
	m_events.clear();
	//WebCapsule::log(WebCapsule::Debug,"Deserializing Events: %s", eventsJSON->toJSONString().latin1().data());
	for(size_t i = 0; i < eventsJSON->length(); i++){
		RefPtr<JSONObject> eventJSON = eventsJSON->get(i)->asObject();
		ASSERT(eventJSON);
		//WebCapsule::log(WebCapsule::Debug,"Deserializing Event: %s", eventJSON->toJSONString().latin1().data());
		m_events.append(deserializeForensicEvent(eventJSON));
	}

	deserializeVector(guard, "currentTimeVals", *(m_currentTimeVals.get()));
	deserializeVector(guard, "monotonicallyIncreasingTimeVals", *(m_monotonicallyIncreasingTimeVals.get()));
	deserializeVector(guard, "userAgentVals", *(m_userAgentVals.get()));
	deserializeVector(guard, "memoryUsageMBVals", *(m_memoryUsageMBVals.get()));
	deserializeVector(guard, "actualMemoryUsageMBVals", *(m_actualMemoryUsageMBVals.get()));
	deserializeVector(guard, "physicalMemoryMBVals", *(m_physicalMemoryMBVals.get()));
	deserializeVector(guard, "numberOfProcessorsVals", *(m_numberOfProcessorsVals.get()));
	deserializeVector(guard, "defaultLocaleVals", *(m_defaultLocaleVals.get()));

	//Modaldialog by Bo
	deserializeVector(guard, "modaldialogAlertVals", *(m_modaldialogAlertVals.get()));
}

PassRefPtr<ForensicEvent> ForensicReplayDataStore::deserializeForensicEvent(PassRefPtr<JSONObject> json){
	String type;
	bool result = json->getString("type", &type);
	ASSERT(result);

	if(type == "ForensicClearCacheEvent"){
		return ForensicClearCacheEvent::create(json);
	} else if(type == "ForensicDidFailEvent"){
		return ForensicDidFailEvent::create(json);
	} else if(type == "ForensicDownloadDataEvent"){
		return ForensicDownloadDataEvent::create(json);
	} else if(type == "ForensicFinishLoadingEvent"){
		return ForensicFinishLoadingEvent::create(json);
	} else if(type == "ForensicInputEvent"){
		return ForensicInputEvent::create(json);
	} else if(type == "ForensicLoadURLEvent"){
		return ForensicLoadURLEvent::create(json);
	} else if(type == "ForensicPageResizeEvent"){
		return ForensicPageResizeEvent::create(json);
	} else if(type == "ForensicPageScrollEvent"){
		return ForensicPageScrollEvent::create(json);
	} else if(type == "ForensicReceiveCachedMetadataEvent"){
		return ForensicReceiveCachedMetadataEvent::create(json);
	} else if(type == "ForensicReceiveDataEvent"){
		return ForensicReceiveDataEvent::create(json);
	} else if(type == "ForensicReceiveResponseEvent"){
		return ForensicReceiveResponseEvent::create(json);
	} else if(type == "ForensicRecordingStartEvent"){
		return ForensicRecordingStartEvent::create(json);
	} else if(type == "ForensicRedirectEvent"){
		return ForensicRedirectEvent::create(json);
	} else if(type == "ForensicReplayStopEvent"){
		return ForensicReplayStopEvent::create(json);
	} else if(type == "ForensicRenderMenuListValueChangedEvent"){
		return ForensicRenderMenuListValueChangedEvent::create(json);
	} else if(type == "ForensicSetAutofilledEvent"){
		return ForensicSetAutofilledEvent::create(json);
	} else if(type == "ForensicSetEditingValueEvent"){
		return ForensicSetEditingValueEvent::create(json);
	} else if(type == "ForensicLoadHistoryItemEvent"){
		return ForensicLoadHistoryItemEvent::create(json);
	} else {
		ASSERT(false); //unknown type
	}

	return nullptr; //Should never reach here
}

} /* namespace WebCore */
