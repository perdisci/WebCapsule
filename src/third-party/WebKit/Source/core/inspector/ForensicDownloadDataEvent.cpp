
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicDownloadDataEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {

PassRefPtr<ForensicDownloadDataEvent> ForensicDownloadDataEvent::create(int dataLength,
		int encodedDataLength,
		const double timestamp){
	return adoptRef(new ForensicDownloadDataEvent(dataLength, encodedDataLength, timestamp));
}

PassRefPtr<ForensicDownloadDataEvent> ForensicDownloadDataEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicDownloadDataEvent(obj));
}

ForensicDownloadDataEvent::ForensicDownloadDataEvent(int dataLength,
		int encodedDataLength,
		const double timestamp)
	: ForensicEvent(timestamp)
	, m_dataLength(dataLength)
	, m_encodedDataLength(encodedDataLength)
{

}

ForensicDownloadDataEvent::ForensicDownloadDataEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicDownloadDataEvent::~ForensicDownloadDataEvent() {

}

int ForensicDownloadDataEvent::dataLength(){
	return m_dataLength;
}

int ForensicDownloadDataEvent::encodedDataLength(){
	return m_encodedDataLength;
}

void ForensicDownloadDataEvent::accept(ForensicEventVisitor& visitor){
	visitor.visit(*this);
}

size_t ForensicDownloadDataEvent::size(){
	return sizeof(m_dataLength) + sizeof(m_encodedDataLength) + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicDownloadDataEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicDownloadDataEvent");
	retval->setNumber("dataLength", dataLength());
	retval->setNumber("encodedDataLength", encodedDataLength());

	return retval;
}

void ForensicDownloadDataEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	bool result = guard->getNumber("dataLength", &m_dataLength);
	ASSERT(result);

	result = guard->getNumber("encodedDataLength", &m_encodedDataLength);
	ASSERT(result);
}


} /* namespace WebCore */
