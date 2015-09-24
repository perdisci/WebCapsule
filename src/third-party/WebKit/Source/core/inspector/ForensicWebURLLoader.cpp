
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicWebURLLoader.h"
#include "core/inspector/InspectorForensicsAgent.h"
#include "core/inspector/ForensicEventInjectorTask.h"
#include "core/inspector/ForensicDidFailEvent.h"
#include "core/inspector/ForensicReceiveResponseEvent.h"
#include "core/inspector/ForensicFinishLoadingEvent.h"
#include "core/inspector/ForensicDownloadDataEvent.h"
#include "core/inspector/ForensicRedirectEvent.h"
#include "core/inspector/ForensicReceiveDataEvent.h"
#include "core/inspector/ForensicReplayData.h"
#include "platform/network/ResourceLoadInfo.h"
#include "platform/network/ResourceResponse.h"
#include "wtf/Assertions.h"
#include "wtf/CurrentTime.h"
#include "public/platform/Platform.h"
#include "core/inspector/WebURLRequestTraits.h"
#include "core/inspector/WebCapsule.h"
#include "wtf/HashMap.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/OwnPtr.h"
#include "platform/TraceEvent.h"

namespace WebCore {

static unsigned int s_reqID = 0;

ForensicWebURLLoader::ForensicWebURLLoader(
		PassRefPtr<ForensicReplayDataStore> dataStore
		, blink::Platform* platform
		, PassOwnPtr<ForensicInspectorStateWrapper> state)
	: m_platform(platform)
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::ForensicWebURLLoader : Total");
	m_dataStore = dataStore;
	m_state = state;
	m_client = 0;
    m_reqID = ++s_reqID; 

	initMissingSynchronousRequest();
}

ForensicWebURLLoader::ForensicWebURLLoader(
		PassRefPtr<ForensicReplayDataRecorder> dataRecorder,
		blink::Platform* platform,
		PassOwnPtr<ForensicInspectorStateWrapper> state,
		blink::WebURLLoader* loader)
	: m_dataRecorder(dataRecorder)
	, m_platform(platform)
	, m_state(state)
{
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::ForensicWebURLLoader : Total");

	m_loader = adoptPtr(loader);
	m_client = 0;
    m_reqID = ++s_reqID;

	initMissingSynchronousRequest();
}

void ForensicWebURLLoader::loadSynchronously(const blink::WebURLRequest& request,
                blink::WebURLResponse& response, blink::WebURLError& error, blink::WebData& data)
{
        TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::loadSynchronously : Total");

        double startTime = 0;
        if(!m_state->replaying()){
        	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::loadSynchronously : getStartTime : Instrumentation");
        	if(m_state->recording()){
        		startTime = m_platform->currentTime();
        	}
        	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::loadSynchronously : getStartTime : Instrumentation");
        	m_loader->loadSynchronously(request, response, error, data);
        }

        TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::loadSynchronously : Instrumentation");
        if(m_state->replaying()){
                loadSynchronouslyReplay(request, response, error, data);
        } else if(m_state->recording()){
        		//must remove the WEBCAPSULE_HEADER before send the request over the wire
                loadSynchronouslyRecording(request, startTime, response, error, data);
        }
        TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::loadSynchronously : Instrumentation");
}

void ForensicWebURLLoader::loadSynchronouslyRecording(
		const blink::WebURLRequest& request,
		double startTime,
		blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data)
{
	m_dataRecorder->syncRequestFinished(request, response, error, data, startTime, m_platform->monotonicallyIncreasingTime(), m_platform->currentTime());
}

void ForensicWebURLLoader::loadSynchronouslyReplay(const blink::WebURLRequest& request,
		blink::WebURLResponse& response, blink::WebURLError& error, blink::WebData& data){

	blink::WebString headerKey(String(ForensicReplayDataStore::WEBCAPSULE_HEADER));
	blink::WebString stackTraceId = request.httpHeaderField(headerKey);
	blink::WebURLRequest& curRequest = const_cast<blink::WebURLRequest&>(request);
	curRequest.clearHTTPHeaderField(headerKey);
	double startTime = m_platform->currentTime();

	if(!m_dataStore->containsRequest(curRequest)){ //request is missing
		//perform stack trace matching
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadSynchronouslyReplay --STACKTRACE MATCHING-- : finding match for request \n%s", curRequest.url().string().latin1().c_str());
		blink::WebURLRequest match = m_dataStore->findMatchingRequest(
				String(stackTraceId.latin1().c_str()),
				startTime,
				V8StackTraceMatchingEntry::Terminal); //we never see redirects for sync requests
		if(!match.isNull()){
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadSynchronouslyReplay --STACKTRACE MATCHING-- : match found \nRequest: %s\nMatch: %s",
					curRequest.url().string().latin1().c_str(),
					match.url().string().latin1().c_str());
			curRequest = match;
		} else {
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadSynchronouslyReplay --STACKTRACE MATCHING-- : match not found for request \n%s", curRequest.url().string().latin1().c_str());
		}
	}

	String reqStr;
    WebURLRequestHash::toString(curRequest, reqStr);
	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadSynchronouslyReplay : request_hash=%u\n%s", WebURLRequestHash::hash(request), reqStr.latin1().data());

	// ASSERT(m_dataStore->requestMap().contains(request)); // we should handle missing synchronous requests, instead of stopping here
	if(m_dataStore->containsRequest(curRequest)) {
		RefPtr<RequestMapEntry> entry = m_dataStore->nextRequestEntry(curRequest);
		ASSERT(entry);
		response.assign(entry->m_response);
		data.assign(entry->m_data.data(), entry->m_data.size());
		error = entry->m_error;
	}
	else {
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadSynchronouslyReplay : REPLAY INCONGRUENCE : missing request : request_hash=%u", WebURLRequestHash::hash(request), reqStr.latin1().data());
		handleMissingSynchronousRequest(curRequest, response, error, data);
	}
}


void ForensicWebURLLoader::initMissingSynchronousRequest() {
	m_missingSynchronousResponse = blink::WebURLResponse();
	m_missingSynchronousResponse.initialize();
	m_missingSynchronousResponse.setHTTPStatusCode(204); // 204 is probably more appropriate than 404
	m_missingSynchronousResponse.setHTTPStatusText(blink::WebString("No Content"));
	m_missingSynchronousResponse.setHTTPVersion(blink::WebURLResponse::HTTP_1_1);
	m_missingSynchronousResponse.setIsMultipartPayload(false);
	m_missingSynchronousResponse.setURL(blink::WebURL()); // NOTE: empty URL is a potential problem during response replay; it needs to be set properly!
	m_missingDataString = String("");
}

//XXX: this is a workaround for missing synchronous requests during replay; it should be eventually removed
void ForensicWebURLLoader::handleMissingSynchronousRequest(const blink::WebURLRequest& request,
		blink::WebURLResponse& response, blink::WebURLError& error, blink::WebData& data){

	// FIXME: setURL() may cause a problem in case multiple threads call a missing synchronous request at the same time
	// However, this only affects replay. For now, we tolerate the risk of such a problem.
	// Notice also that multiple threads should call the same instance of ForensicWebURLLoader.
	// In case only one thread using this instance of ForensicWebURLLoader, there should be no problem
	// because we are dealing with synchornous requests;
	// It may also be that a response with empty URL could also work fine... to be tested!
	// m_missingSynchronousResponse.setURL(request.url());

	response.assign(m_missingSynchronousResponse);
	data.assign(m_missingDataString.latin1().data(), 0); // no data
	error = blink::WebURLError(); // no error
}


//XXX: this is a workaround for missing asynchronous requests during replay; it should be eventually removed
void ForensicWebURLLoader::handleMissingAsynchronousRequest(const blink::WebURLRequest& request, blink::WebURLLoaderClient* client){
	m_missingResponse = adoptPtr(new blink::WebURLResponse());
	m_missingResponse->initialize();
	m_missingResponse->setHTTPStatusCode(204); // 204 is probably more appropriate than 404
	m_missingResponse->setHTTPStatusText(blink::WebString("No Content"));
	m_missingResponse->setHTTPVersion(blink::WebURLResponse::HTTP_1_1);
	m_missingResponse->setIsMultipartPayload(false);
	m_missingResponse->setURL(request.url());

	m_events.append(ForensicReceiveResponseEvent::create(*(m_missingResponse.get()), m_platform->currentTime()));
	m_events.append(ForensicFinishLoadingEvent::create(m_platform->monotonicallyIncreasingTime(), blink::WebURLLoaderClient::kUnknownEncodedDataLength, m_platform->currentTime()));
	m_platform->currentThread()->postTask(new ForensicEventInjectorTask(m_events, m_platform, ForensicInspectorStateWrapper::create(*(m_state.get())), ForensicEventInjector::create(this, client), m_dataStore->recordStartTime(), m_dataStore->replayStartTime(), true, 0.0));
}

//returns request is not in the redirect map
blink::WebURLRequest& ForensicWebURLLoader::createRedirectEvents(const blink::WebURLRequest& request, Vector<RefPtr<ForensicEvent> >& store){
	blink::WebURLRequest& curRequest = const_cast<blink::WebURLRequest&>(request);
	WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicWebURLLoader::createRedirectEvents : Original URL = %s", curRequest.url().string().latin1().data());

	while(RefPtr<RedirectEntry> pair = m_dataStore->nextRedirectPair(curRequest)){
		double nowRecTime = (m_platform->currentTime() - m_dataStore->replayStartTime()) + m_dataStore->recordStartTime();
				WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicWebURLLoader::createRedirectEvents : Map does contain URL = %s", curRequest.url().string().latin1().data());
				WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicWebURLLoader::createRedirectEvents : Getting request");
				ResourceRequest& req = *(pair->m_request.get());
				WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicWebURLLoader::createRedirectEvents : Getting response");
				ResourceResponse& resp = *(pair->m_response.get());
				store.append(ForensicRedirectEvent::create(req, resp, nowRecTime));
				curRequest = blink::WrappedResourceRequest(req);
	}

	return curRequest;
}



void ForensicWebURLLoader::loadAsynchronously(const blink::WebURLRequest& request, blink::WebURLLoaderClient* client){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::loadAsynchronously : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::loadAsynchronously : Instrumentation");
    if(m_state->recording()) {
        TRACE_EVENT2("blink", "WebCapsule::ForensicWebURLLoader::loadAsynchronously : StartTime", "startTime", m_platform->monotonicallyIncreasingTime(), "ID", m_reqID);
    }

	m_client = client;
	if(m_state->replaying()){
		loadAsynchronouslyReplay(request);
	} else if(m_state->recording()){
		loadAsynchronouslyRecording(request);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::loadAsynchronously : Instrumentation");

	if(m_loader && !m_state->replaying()){
		m_loader->loadAsynchronously(request, this);
	}

}

void ForensicWebURLLoader::loadAsynchronouslyRecording(const blink::WebURLRequest& request){
	m_dataRecorder->initPendingRequest(m_client, request, m_platform->currentTime());
}

void ForensicWebURLLoader::loadAsynchronouslyReplay(const blink::WebURLRequest& request){

	blink::WebString headerKey(String(ForensicReplayDataStore::WEBCAPSULE_HEADER));
	blink::WebString stackTraceId = request.httpHeaderField(headerKey);
	blink::WebURLRequest& curRequest = const_cast<blink::WebURLRequest&>(request);
	curRequest.clearHTTPHeaderField(headerKey);
	double startTime = m_platform->currentTime();

	if(!m_dataStore->containsRequest(curRequest) && !m_dataStore->containsRedirect(curRequest)){ //request is missing
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadAsynchronouslyReplay --STACKTRACE MATCHING-- : finding match for request \n%s", curRequest.url().string().latin1().c_str());
		//perform stack trace matching
		blink::WebURLRequest match = m_dataStore->findMatchingRequest(
				String(stackTraceId.latin1().c_str()),
				startTime);
		if(!match.isNull()){
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadAsynchronouslyReplay --STACKTRACE MATCHING-- : match found \nRequest: %s\nMatch: %s",
					curRequest.url().string().latin1().c_str(),
					match.url().string().latin1().c_str());
			curRequest = match;
		} else {
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadAsynchronouslyReplay --STACKTRACE MATCHING-- : match not found request \n%s", curRequest.url().string().latin1().c_str());
		}
	}

	curRequest = createRedirectEvents(curRequest, m_events);

	String reqStr;
	WebURLRequestHash::toString(curRequest, reqStr);
	if(!m_dataStore->containsRequest(curRequest)){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadAsynchronouslyReplay : REPLAY INCONGRUENCE : missing request : request_hash=%u\n%s", WebURLRequestHash::hash(curRequest), reqStr.latin1().data());
		handleMissingAsynchronousRequest(curRequest, m_client);
		return;
	}

	ASSERT(m_dataStore->containsRequest(curRequest));

	RefPtr<RequestMapEntry> entry = m_dataStore->nextRequestEntry(curRequest);
	if(!entry){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadAsynchronouslyReplay : REPLAY INCONGRUENCE : too few asynchronous requests : request_hash=%u\n%s", WebURLRequestHash::hash(curRequest), reqStr.latin1().data());
		handleMissingAsynchronousRequest(curRequest, m_client);
		return;
	}
	ASSERT(entry);

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicWebURLLoader::loadAsynchronouslyReplay : Serving asynchronous request: request_hash=%u\n%s\nTotal Encoded Data Length: %li", WebURLRequestHash::hash(curRequest), reqStr.latin1().data(), entry->m_totalEncodedDataLength);

	const double NET_DELAY_EPSILON = 0.0001;
	double nowRecTime = (m_platform->currentTime() - m_dataStore->replayStartTime()) + m_dataStore->recordStartTime();
	double endRecTime = entry->endTime();
	if(nowRecTime >= endRecTime)
		endRecTime = nowRecTime + NET_DELAY_EPSILON; // endTime cannot be earlier than the timestamp of ReceiveResponseEvent

	if (entry->m_error.reason) {
		m_events.append(ForensicDidFailEvent::create(entry->m_error, nowRecTime));
	} else {
		m_events.append(ForensicReceiveResponseEvent::create(entry->m_response, nowRecTime));
		RefPtr<ResourceLoadInfo> resourceLoadInfo = entry->m_response.toResourceResponse().resourceLoadInfo();
		int64 encodedDataLength = resourceLoadInfo ? resourceLoadInfo->encodedDataLength : blink::WebURLLoaderClient::kUnknownEncodedDataLength;
		if(curRequest.downloadToFile()){  //TODO find out how didDownloadData is called in relation to didReceiveData
			m_events.append(ForensicDownloadDataEvent::create(entry->m_data.size(), encodedDataLength, endRecTime));
		} else if(entry->m_data.size() > 0) { // @Roberto - FIXED: some responses have no data; we should call ForensicReceiveDataEvent only if some data was received 
			//TODO encodedDataLength the finished loading time as part of ForensicReplayDataStore::RequestMapEntry
			m_events.append(ForensicReceiveDataEvent::create(entry->m_data.data(), entry->m_data.size(), entry->m_totalEncodedDataLength, endRecTime));
		}
		endRecTime += NET_DELAY_EPSILON; // just to make sure events are sorted correctly when the ForensicEventInjectorTask runs
		m_events.append(ForensicFinishLoadingEvent::create(entry->finishTime(), entry->m_totalEncodedDataLength, endRecTime));
	}

	m_platform->currentThread()->postTask(new ForensicEventInjectorTask(m_events, m_platform, ForensicInspectorStateWrapper::create(*(m_state.get())), ForensicEventInjector::create(this, m_client), m_dataStore->recordStartTime(), m_dataStore->replayStartTime(), true, 1.0));
}

void ForensicWebURLLoader::cancel(){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::cancel : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::cancel : Instrumentation");
	Vector<RefPtr<ForensicEvent> >::iterator it;
	for(it = m_events.begin(); it != m_events.end(); ++it){
		RefPtr<ForensicEvent> event = *it;
		if(event->state() == ForensicEvent::Ready){
			event->cancel();
		}
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::cancel : Instrumentation");

	if(m_loader){
		m_loader->cancel();
	}
}

void ForensicWebURLLoader::setDefersLoading(bool defer){
	if(m_loader){
		m_loader->setDefersLoading(defer);
	}
}

void ForensicWebURLLoader::willSendRequest(blink::WebURLLoader* request, blink::WebURLRequest& redirectRequest, const blink::WebURLResponse& redirectResponse){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::willSendRequest : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::willSendRequest : Instrumentation");
	if(m_state->recording()){
		m_dataRecorder->addPendingRedirect(m_client, redirectRequest, redirectResponse, m_platform->userAgent(), m_platform->currentTime());
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::willSendRequest : Instrumentation");

	m_client->willSendRequest(request, redirectRequest, redirectResponse);
}

void ForensicWebURLLoader::didSendData(blink::WebURLLoader*, unsigned long long bytesSent, unsigned long long totalBytesToBeSent){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::didSendData : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::didSendData : Instrumentation");
	if(m_state->recording()){
		//TODO implement
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::didSendData : Instrumentation");

	m_client->didSendData(this, bytesSent, totalBytesToBeSent);
}

void ForensicWebURLLoader::didReceiveResponse(blink::WebURLLoader*, const blink::WebURLResponse& response){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveResponse : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveResponse : Instrumentation");
	if(m_state->recording()){
		m_dataRecorder->setPendingResponse(m_client, response);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveResponse : Instrumentation");

	m_client->didReceiveResponse(this, response);
}

void ForensicWebURLLoader::didReceiveData(blink::WebURLLoader*, const char* data, int length, int encodedDataLength){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveData : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveData : Instrumentation");
	if(m_state->recording()){
		m_dataRecorder->addPendingData(m_client, data, length);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveData : Instrumentation");

	m_client->didReceiveData(this, data, length, encodedDataLength);
}

void ForensicWebURLLoader::didReceiveCachedMetadata(blink::WebURLLoader*, const char* data, int length){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveCachedMetadata : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveCachedMetadata : Instrumentation");
	if(m_state->recording()){
		//TODO implement
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::didReceiveCachedMetadata : Instrumentation");

	m_client->didReceiveCachedMetadata(this, data, length);
}

void ForensicWebURLLoader::didFinishLoading(blink::WebURLLoader*, double finishTime, int64 encodedDataLength){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::didFinishLoading : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::didFinishLoading : Instrumentation");
    if(m_state->recording()) {
        TRACE_EVENT2("blink", "WebCapsule::ForensicWebURLLoader::didFinishLoading : FinishTime", "finishTime", finishTime, "ID", m_reqID);
	}

	if(m_state->recording()){
		m_dataRecorder->pendingRequestFinished(m_client, finishTime, m_platform->currentTime(), encodedDataLength);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::didFinishLoading : Instrumentation");

	m_client->didFinishLoading(this, finishTime, encodedDataLength);
}

void ForensicWebURLLoader::didFail(blink::WebURLLoader*, const blink::WebURLError& error){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::didFail : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::didFail : Instrumentation");
	if(m_state->recording()){
		m_dataRecorder->pendingRequestFailed(m_client, error);
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::didFail : Instrumentation");

	m_client->didFail(this, error);
}

void ForensicWebURLLoader::didDownloadData(blink::WebURLLoader*, int length, int encodedDataLength){
	TRACE_EVENT0("blink", "WebCapsule::ForensicWebURLLoader::didDownloadData : Total");

	TRACE_EVENT_BEGIN0("blink", "WebCapsule::ForensicWebURLLoader::didDownloadData : Instrumentation");
	if(m_state->recording()){
		//TODO implement
	}
	TRACE_EVENT_END0("blink", "WebCapsule::ForensicWebURLLoader::didDownloadData : Instrumentation");

	m_client->didDownloadData(this, length, encodedDataLength);
}



} /* namespace blink */
