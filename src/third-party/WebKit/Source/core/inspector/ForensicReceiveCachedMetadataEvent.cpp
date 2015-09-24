
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicReceiveCachedMetadataEvent.h"
#include "core/inspector/ForensicEventVisitor.h"
#include "wtf/text/Base64.h"

namespace WebCore {

PassRefPtr<ForensicReceiveCachedMetadataEvent> ForensicReceiveCachedMetadataEvent::create(const char* data,
		int dataLength,
		const double timestamp){
	return adoptRef(new ForensicReceiveCachedMetadataEvent(data, dataLength, timestamp));
}

PassRefPtr<ForensicReceiveCachedMetadataEvent> ForensicReceiveCachedMetadataEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicReceiveCachedMetadataEvent(obj));
}

ForensicReceiveCachedMetadataEvent::ForensicReceiveCachedMetadataEvent(const char* data,
		int dataLength,
		const double timestamp)
	: ForensicEvent(timestamp)
{
	m_data.append(data, dataLength);
}

ForensicReceiveCachedMetadataEvent::ForensicReceiveCachedMetadataEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicReceiveCachedMetadataEvent::~ForensicReceiveCachedMetadataEvent() {

}

const Vector<char>& ForensicReceiveCachedMetadataEvent::data(){
	return m_data;
}

void ForensicReceiveCachedMetadataEvent::accept(ForensicEventVisitor& visitor){
	visitor.visit(*this);
}

size_t ForensicReceiveCachedMetadataEvent::size(){
	return m_data.size() + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicReceiveCachedMetadataEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicReceiveCachedMetadataEvent");
	retval->setString("data", base64Encode(data()));

	return retval;
}

void ForensicReceiveCachedMetadataEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	String temp;
	bool result = guard->getString("data", &temp);
	ASSERT(result);

	base64Decode(temp, m_data);
}

} /* namespace WebCore */
