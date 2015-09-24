
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicReceiveDataEvent.h"
#include "core/inspector/ForensicEventVisitor.h"
#include "wtf/text/Base64.h"

namespace WebCore {

PassRefPtr<ForensicReceiveDataEvent> ForensicReceiveDataEvent::create(const char* data,
		int dataLength,
		int encodedDataLength,
		const double timestamp){
	return adoptRef(new ForensicReceiveDataEvent(data, dataLength, encodedDataLength, timestamp));
}

PassRefPtr<ForensicReceiveDataEvent> ForensicReceiveDataEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicReceiveDataEvent(obj));
}

ForensicReceiveDataEvent::ForensicReceiveDataEvent(const char* data,
		int dataLength,
		int encodedDataLength,
		const double timestamp)
	: ForensicEvent(timestamp)
	, m_encodedDataLength(encodedDataLength)
{
	m_data.append(data, dataLength);
}

ForensicReceiveDataEvent::ForensicReceiveDataEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}


ForensicReceiveDataEvent::~ForensicReceiveDataEvent() {

}

const Vector<char>& ForensicReceiveDataEvent::data(){
	return m_data;
}

int ForensicReceiveDataEvent::encodedDataLength(){
	return m_encodedDataLength;
}

void ForensicReceiveDataEvent::accept(ForensicEventVisitor& visitor){
	visitor.visit(*this);
}

size_t ForensicReceiveDataEvent::size(){
	return m_data.size() + sizeof(m_encodedDataLength) + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicReceiveDataEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicReceiveDataEvent");
	retval->setString("data", base64Encode(data()));
	retval->setNumber("encodedDataLength", encodedDataLength());

	return retval;
}

void ForensicReceiveDataEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	String temp;
	bool result = guard->getString("data", &temp);
	ASSERT(result);
	base64Decode(temp, m_data);

	result = guard->getNumber("encodedDataLength", &m_encodedDataLength);
	ASSERT(result);
}

} /* namespace WebCore */
