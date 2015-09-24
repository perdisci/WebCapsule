
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicReceiveResponseEvent.h"
#include "core/inspector/ForensicEventVisitor.h"
#include "platform/exported/WrappedResourceResponse.h"

namespace WebCore {

PassRefPtr<ForensicReceiveResponseEvent> ForensicReceiveResponseEvent::create(blink::WebURLResponse& response, const double timestamp)
{
	return adoptRef(new ForensicReceiveResponseEvent(response, timestamp));
}

PassRefPtr<ForensicReceiveResponseEvent> ForensicReceiveResponseEvent::create(PassRefPtr<JSONObject> obj)
{
	return adoptRef(new ForensicReceiveResponseEvent(obj));
}


ForensicReceiveResponseEvent::ForensicReceiveResponseEvent(blink::WebURLResponse& response, const double timestamp)
	: ForensicEvent(timestamp)
	, m_response(response)
{

}

ForensicReceiveResponseEvent::ForensicReceiveResponseEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicReceiveResponseEvent::~ForensicReceiveResponseEvent() {

}

blink::WebURLResponse& ForensicReceiveResponseEvent::response(){
	return m_response;
}

void ForensicReceiveResponseEvent::accept(ForensicEventVisitor& visitor){
	visitor.visit(*this);
}

//TODO find a more accurate measurement of response size
size_t ForensicReceiveResponseEvent::size(){
	return sizeof(m_response) + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicReceiveResponseEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicReceiveResponseEvent");
	retval->setObject("response", SerializationUtils::serialize(m_response));

	return retval;
}

void ForensicReceiveResponseEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	RefPtr<JSONObject> responseJSON = guard->getObject("response");
	ASSERT(responseJSON);
	OwnPtr<ResourceResponse> response = SerializationUtils::deserializeResponse(responseJSON);
	m_response = blink::WrappedResourceResponse(*(response.leakPtr()));
}

} /* namespace WebCore */
