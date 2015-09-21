/*
 * ForensicFinishLoadingEvent.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicFinishLoadingEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {

PassRefPtr<ForensicFinishLoadingEvent> ForensicFinishLoadingEvent::create(double finishTime,
		long encodedDataLength,
		const double timestamp){
	return adoptRef(new ForensicFinishLoadingEvent(finishTime, encodedDataLength, timestamp));
}

PassRefPtr<ForensicFinishLoadingEvent> ForensicFinishLoadingEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicFinishLoadingEvent(obj));
}

ForensicFinishLoadingEvent::ForensicFinishLoadingEvent(double finishTime,
		long encodedDataLength,
		const double timestamp)
	: ForensicEvent(timestamp)
	, m_finishTime(finishTime)
	, m_encodedDataLength(encodedDataLength)
{

}

ForensicFinishLoadingEvent::ForensicFinishLoadingEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicFinishLoadingEvent::~ForensicFinishLoadingEvent() {

}

double ForensicFinishLoadingEvent::finishTime(){
	return m_finishTime;
}

long ForensicFinishLoadingEvent::encodedDataLength(){
	return m_encodedDataLength;
}

void ForensicFinishLoadingEvent::accept(ForensicEventVisitor& visitor){
	visitor.visit(*this);
}

size_t ForensicFinishLoadingEvent::size(){
	return sizeof(m_finishTime) + sizeof(m_encodedDataLength) + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicFinishLoadingEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicFinishLoadingEvent");
	retval->setNumber("finishTime", finishTime());
	retval->setNumber("encodedDataLength", encodedDataLength());

	return retval;
}

void ForensicFinishLoadingEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	bool result = guard->getNumber("finishTime", &m_finishTime);
	ASSERT(result);

	result = guard->getNumber("encodedDataLength", &m_encodedDataLength);
	ASSERT(result);

}

} /* namespace WebCore */
