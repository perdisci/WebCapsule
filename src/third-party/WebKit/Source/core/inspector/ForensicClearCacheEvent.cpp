
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicClearCacheEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {

PassRefPtr<ForensicClearCacheEvent> ForensicClearCacheEvent::create(const double timestamp){
	return adoptRef(new ForensicClearCacheEvent(timestamp));
}

PassRefPtr<ForensicClearCacheEvent> ForensicClearCacheEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicClearCacheEvent(obj));
}

ForensicClearCacheEvent::ForensicClearCacheEvent(const double timestamp)
	: ForensicEvent(timestamp)
{

}

ForensicClearCacheEvent::ForensicClearCacheEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}


ForensicClearCacheEvent::~ForensicClearCacheEvent()
{

}

void ForensicClearCacheEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

PassRefPtr<JSONObject> ForensicClearCacheEvent::serialize(){
	return ForensicEvent::serialize("ForensicClearCacheEvent");
}

void ForensicClearCacheEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);
}

} /* namespace WebCore */
