/*
 * ForensicRecordingStartEvent.cpp
 *
 *  Created on: Sep 10, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicRecordingStartEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {

PassRefPtr<ForensicRecordingStartEvent> ForensicRecordingStartEvent::create(const double timestamp, const double monotonicallyIncreasingTimestamp){
	return adoptRef(new ForensicRecordingStartEvent(timestamp, monotonicallyIncreasingTimestamp));
}

PassRefPtr<ForensicRecordingStartEvent> ForensicRecordingStartEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicRecordingStartEvent(obj));
}


ForensicRecordingStartEvent::ForensicRecordingStartEvent(const double timestamp, const double monotonicallyIncreasingTimestamp)
	: ForensicEvent(timestamp)
	, m_monotonicallyIncreasingTimestamp(monotonicallyIncreasingTimestamp)
{

}

ForensicRecordingStartEvent::ForensicRecordingStartEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicRecordingStartEvent::~ForensicRecordingStartEvent() {
	// TODO Auto-generated destructor stub
}

void ForensicRecordingStartEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

size_t ForensicRecordingStartEvent::size(){
	return sizeof(m_monotonicallyIncreasingTimestamp) + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicRecordingStartEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicRecordingStartEvent");
	retval->setNumber("monotonicallyIncreasingTimestamp", monotonicallyIncreasingTimestamp());

	return retval;
}

void ForensicRecordingStartEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	bool result = guard->getNumber("monotonicallyIncreasingTimestamp", &m_monotonicallyIncreasingTimestamp);
	ASSERT(result);
}

double ForensicRecordingStartEvent::monotonicallyIncreasingTimestamp(){
	return m_monotonicallyIncreasingTimestamp;
}

} /* namespace WebCore */
