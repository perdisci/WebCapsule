
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicDidFailEvent.h"
#include "core/inspector/ForensicEventVisitor.h"
#include "public/platform/WebURLError.h"
#include "platform/weborigin/KURL.h"

namespace WebCore {

PassRefPtr<ForensicDidFailEvent> ForensicDidFailEvent::create(blink::WebURLError& error, const double timestamp){
	return adoptRef(new ForensicDidFailEvent(error, timestamp));
}

PassRefPtr<ForensicDidFailEvent> ForensicDidFailEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicDidFailEvent(obj));
}

ForensicDidFailEvent::ForensicDidFailEvent(blink::WebURLError& error, const double timestamp)
	: ForensicEvent(timestamp)
	, m_error(error)
{

}

ForensicDidFailEvent::ForensicDidFailEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicDidFailEvent::~ForensicDidFailEvent() {
	// TODO Auto-generated destructor stub
}

blink::WebURLError& ForensicDidFailEvent::error(){
	return m_error;
}

void ForensicDidFailEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

size_t ForensicDidFailEvent::size(){
	size_t retval = Utils::size(m_error)
			+ ForensicEvent::size();

	return retval;
}

PassRefPtr<JSONObject> ForensicDidFailEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicDidFailEvent");
	retval->setObject("error", SerializationUtils::serialize(m_error));
	return retval;
}

void ForensicDidFailEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	RefPtr<JSONObject> errorJSON = guard->getObject("error");
	ASSERT(errorJSON);
	SerializationUtils::deserialize(errorJSON, m_error);
}

} /* namespace WebCore */
