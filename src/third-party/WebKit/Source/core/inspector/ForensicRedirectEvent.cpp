
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicRedirectEvent.h"
#include "core/inspector/ForensicEventVisitor.h"
#include "platform/CrossThreadCopier.h"


namespace WebCore {

PassRefPtr<ForensicRedirectEvent> ForensicRedirectEvent::create(ResourceRequest& request,
			ResourceResponse& response,
			const double timestamp)
{
	return adoptRef(new ForensicRedirectEvent(request, response, timestamp));
}

PassRefPtr<ForensicRedirectEvent> ForensicRedirectEvent::create(PassRefPtr<JSONObject> obj)
{
	return adoptRef(new ForensicRedirectEvent(obj));
}

ForensicRedirectEvent::ForensicRedirectEvent(ResourceRequest& request,
		ResourceResponse& response,
		const double timestamp)
	: ForensicEvent(timestamp)
	, m_wrappedRequest(0)
	, m_wrappedResponse(0)
{
	m_request = ResourceRequest::adopt(CrossThreadCopier<ResourceRequest>::copy(request));
	ASSERT(request.isNull() == m_request->isNull());

	m_response = ResourceResponse::adopt(CrossThreadCopier<ResourceResponse>::copy(response));
	ASSERT(response.isNull() == m_response->isNull());
}

ForensicRedirectEvent::ForensicRedirectEvent(PassRefPtr<JSONObject> obj)
	: m_wrappedRequest(0)
	, m_wrappedResponse(0)
{
	deserialize(obj);
}

ForensicRedirectEvent::~ForensicRedirectEvent() {
	if(m_wrappedRequest){
		delete m_wrappedRequest;
	}

	if(m_wrappedResponse){
		delete m_wrappedResponse;
	}
}

blink::WebURLRequest& ForensicRedirectEvent::request(){
	if(!m_wrappedRequest){
		ResourceRequest& req = *(m_request.get());
		m_wrappedRequest = new blink::WrappedResourceRequest(req);
	}
	return *m_wrappedRequest;

	//ResourceRequest& req = *(m_request.get());
	//return *(new blink::WrappedResourceRequest(req));
}


blink::WebURLResponse& ForensicRedirectEvent::response(){
	if(!m_wrappedResponse){
		ResourceResponse& resp = *(m_response.get());
		m_wrappedResponse = new blink::WrappedResourceResponse(resp);
	}
	return *m_wrappedResponse;


	//ResourceResponse& resp = *(m_response.get());
	//return *(new blink::WrappedResourceResponse(resp));
}

void ForensicRedirectEvent::accept(ForensicEventVisitor& visitor){
	visitor.visit(*this);
}

//TODO more accurately measure the size of request and response
size_t ForensicRedirectEvent::size(){
	return sizeof(*(m_request.get())) + sizeof(*(m_response.get())) + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicRedirectEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicRedirectEvent");
	retval->setObject("request", SerializationUtils::serialize(*m_request.get()));
	retval->setObject("response", SerializationUtils::serialize(*m_response.get()));
	return retval;
}

void ForensicRedirectEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	RefPtr<JSONObject> requestJSON = guard->getObject("request");
	ASSERT(requestJSON);
	m_request = SerializationUtils::deserializeRequest(requestJSON);

	RefPtr<JSONObject> responseJSON = guard->getObject("response");
	ASSERT(responseJSON);
	m_response = SerializationUtils::deserializeResponse(responseJSON);
}

} /* namespace WebCore */
