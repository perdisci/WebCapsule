
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
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
