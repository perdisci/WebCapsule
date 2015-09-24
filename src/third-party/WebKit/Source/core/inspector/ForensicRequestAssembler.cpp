
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicRequestAssembler.h"

namespace WebCore {

const char* ForensicRequestAssembler::WEBCAPSULE_HEADER = "X-WebCapsule";

const String ForensicRequestAssembler::findPageURLAliasChainRoot(const String& startURL) {
	if(m_pageURLAliasMap.isEmpty() || !m_pageURLAliasMap.contains(startURL)){
		return startURL;
	}

	String s = startURL;
	//WebCapsule::log(WebCapsule::VeryVerboseLogLevel, "STARTURL=%s", s.latin1().data());
	do {
		if(!m_pageURLAliasMap.contains(s) || s == m_pageURLAliasMap.get(s)){ // avoid looping on self-alias!
			break;
		}
		//WebCapsule::log(WebCapsule::VeryVerboseLogLevel, "CONTAINS STARTURL=%s (%s)", s.latin1().data(), aliasMap->get(s).latin1().data());
		s = m_pageURLAliasMap.get(s);
	} while(s!=startURL); // the while condition is important to avoid looping indefinitely over possible "circular" alias chains
	//WebCapsule::log(WebCapsule::VeryVerboseLogLevel, "RETURNING STARTURL=%s", s.latin1().data());
	return s;
}

void ForensicRequestAssembler::setURLAlias(const String& alias, const String& orig){
	m_pageURLAliasMap.set(alias, orig);
}

void ForensicRequestAssembler::addToPageURLRequestsMap(String& pageURL, PassRefPtr<PageURLRequest> request) {
	if(!m_pageURLRequestsMap.isEmpty() && !m_pageURLRequestsMap.contains(pageURL)){
		m_pageURLRequestsMap.set(pageURL,Vector<RefPtr<PageURLRequest> >());
	}

	// FIXME: there probably exists a more efficient way to append a PageURLRequest to the following HashMap value Vector
	Vector<RefPtr<PageURLRequest> > v = m_pageURLRequestsMap.get(pageURL);
	v.append(request);
	m_pageURLRequestsMap.set(pageURL, v);
}

size_t ForensicRequestAssembler::sizePageURLRequests(const String& url){
	return m_pageURLRequestsMap.get(url).size();
}

void ForensicRequestAssembler::initPendingRequest(blink::WebURLLoaderClient* client, const blink::WebURLRequest& request, double startTime){
	ASSERT(client);
	m_pendingMap.set(client, createPendingEntry(request, startTime));
}

PassRefPtr<PendingRequestMapEntry> ForensicRequestAssembler::createPendingEntry(const blink::WebURLRequest& request, double startTime){
	blink::WebString value = request.httpHeaderField(blink::WebString(String(WEBCAPSULE_HEADER)));
	V8StackTrace stackTrace = V8StackTrace();
	//NOTE value can be null if request made outside of ResourceLoader, for example content::MultiResolutionImageResourceFetcher
	if(!value.isEmpty() && !value.isNull()){
		const_cast<blink::WebURLRequest&>(request).clearHTTPHeaderField(blink::WebString(String(WEBCAPSULE_HEADER)));
		String key(value);
		stackTrace.appendVector(m_stackTraceMap.get(key));
		m_stackTraceMap.remove(key);
	}

	RefPtr<PendingRequestMapEntry> entry = PendingRequestMapEntry::create(const_cast<blink::WebURLRequest&>(request));
	entry->m_mapEntry->setStackTrace(stackTrace);
	entry->m_mapEntry->setStartTime(startTime);
	return entry;
}

void ForensicRequestAssembler::syncRequestFinished(const blink::WebURLRequest& request,
		blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data,
		double startTime,
		double finishTime,
		double endTime){

	RefPtr<PendingRequestMapEntry> entry = createPendingEntry(request, startTime);

	String reqStr;
	WebURLRequestHash::toString(entry->m_request, reqStr);
	WebCapsule::log(WebCapsule::DebugLogLevel,"syncRequestFinished : %u\n%s", WebURLRequestHash::hash(entry->m_request), reqStr.latin1().data());

	entry->m_mapEntry->setResponse(response);
	entry->m_mapEntry->m_data.append(data.data(), data.size());
	entry->m_mapEntry->m_error = error;
	entry->m_mapEntry->setFinishTime(finishTime);
	entry->m_mapEntry->setEndTime(endTime);
	entry->m_mapEntry->setTotalEncodedDataLength(blink::WebURLLoaderClient::kUnknownEncodedDataLength);

	addFinishedRequest(entry);
}

void ForensicRequestAssembler::setPendingResponse(blink::WebURLLoaderClient* client, const blink::WebURLResponse& response){
	ASSERT(m_pendingMap.contains(client));
	m_pendingMap.get(client)->m_mapEntry->setResponse(response);
}

void ForensicRequestAssembler::addPendingData(blink::WebURLLoaderClient* client, const char* data, int length){
	ASSERT(m_pendingMap.contains(client));
	m_pendingMap.get(client)->m_mapEntry->m_data.append(data, length);
}

void ForensicRequestAssembler::addPendingRedirect(blink::WebURLLoaderClient* client, blink::WebURLRequest& redirectRequest, const blink::WebURLResponse& redirectResponse, const blink::WebString& userAgent, double startTime){
	ASSERT(m_pendingMap.contains(client));
	PassRefPtr<PendingRequestMapEntry> entry = m_pendingMap.get(client);

	//XXX Note: we are modifying the data the will be reinjected here, this might cause unintended consequences
	redirectRequest.setHTTPHeaderField(blink::WebString("User-Agent"), userAgent);

	String originURLStr(entry->m_request.url().spec().data());
	String requestURLStr(redirectRequest.url().spec().data());
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore: Recorded Redirect: Original URL %s\nRedirect URL %s", originURLStr.latin1().data(), requestURLStr.latin1().data());

	// Assist self-healing ------------
	// FIXME: notice that any existing alias for requestURLStr will be replaced;
	// in the future we might need to track multiple possible redirection changes to the same requestURLStr
	if(requestURLStr!=originURLStr)
		setURLAlias(requestURLStr,originURLStr);
	// --------------------------------

	V8StackTrace stackTrace;
	V8StackTrace* stackTraceTemp = entry->m_mapEntry->stackTrace();
	if(stackTraceTemp){
		stackTrace.appendVector(*stackTraceTemp);
	}

	addFinishedRedirect(entry->m_request, redirectRequest, redirectResponse, stackTrace, entry->m_mapEntry->startTime());

	entry->setRequest(redirectRequest);
	entry->m_mapEntry->clearStackTrace(); //the stack trace does not reflect the redirect request
	entry->m_mapEntry->setStartTime(startTime); //update the start time  //TODO implement
}

void ForensicRequestAssembler::pendingRequestFinished(blink::WebURLLoaderClient* client, double finishTime, double endTime, int64 encodedDataLength){
	ASSERT(m_pendingMap.contains(client));
	RefPtr<PendingRequestMapEntry> entry = m_pendingMap.get(client);
	entry->m_mapEntry->setFinishTime(finishTime);
	entry->m_mapEntry->setEndTime(endTime);
	entry->m_mapEntry->setTotalEncodedDataLength(encodedDataLength);

	addFinishedRequest(entry);
	m_pendingMap.remove(client);
}

void ForensicRequestAssembler::pendingRequestFailed(blink::WebURLLoaderClient* client, const blink::WebURLError& error){
	ASSERT(m_pendingMap.contains(client));
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicReplayDataStore: Request Failed: Error %i Domain %s Description %s", error.reason, error.domain.latin1().c_str(), error.localizedDescription.latin1().c_str());
	RefPtr<PendingRequestMapEntry> entry = m_pendingMap.get(client);
	entry->m_mapEntry->m_error = error;

	addFinishedRequest(entry);
	m_pendingMap.remove(client);
}

void ForensicRequestAssembler::setStackTraceForRequest(const String& key, const V8StackTrace& value){
	m_stackTraceMap.set(String(key), V8StackTrace(value));
}

void ForensicRequestAssembler::clear(){
    m_pendingMap.clear();
    m_stackTraceMap.clear();
    m_pageURLAliasMap.clear();
    m_pageURLRequestsMap.clear();
}

} /* namespace WebCore */
