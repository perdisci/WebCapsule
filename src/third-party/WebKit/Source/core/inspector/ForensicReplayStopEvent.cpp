/*
 * ForensicReplayStopEvent.cpp
 *
 *  Created on: Sep 4, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicReplayStopEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {


PassRefPtr<ForensicReplayStopEvent> ForensicReplayStopEvent::create(const double timestamp){
	return adoptRef(new ForensicReplayStopEvent(timestamp));
}

PassRefPtr<ForensicReplayStopEvent> ForensicReplayStopEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicReplayStopEvent(obj));
}


ForensicReplayStopEvent::ForensicReplayStopEvent(const double timestamp)
	: ForensicEvent(timestamp)
{

}

ForensicReplayStopEvent::ForensicReplayStopEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicReplayStopEvent::~ForensicReplayStopEvent()
{

}

void ForensicReplayStopEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

PassRefPtr<JSONObject> ForensicReplayStopEvent::serialize(){
	return ForensicEvent::serialize("ForensicReplayStopEvent");
}

void ForensicReplayStopEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);
}

} /* namespace WebCore */
