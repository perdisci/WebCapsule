
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicEvent.h"
#include "core/inspector/WebCapsule.h"

namespace WebCore{

ForensicEvent::ForensicEvent()
	: m_timestamp(0)
	, m_state(Ready)
{

}

ForensicEvent::ForensicEvent(double timestamp)
		: m_timestamp(timestamp)
		, m_state(Ready)
{
}

ForensicEvent::ForensicEvent(PassRefPtr<JSONObject> obj)
	: m_state(Ready)
{
	deserialize(obj);
}

double ForensicEvent::timestamp(){
	return m_timestamp;
}

size_t ForensicEvent::size(){
	return sizeof(m_timestamp) + sizeof(m_state);
}

PassRefPtr<JSONObject> ForensicEvent::serialize(){
	return serialize("ForensicEvent");
}

PassRefPtr<JSONObject> ForensicEvent::serialize(const String& type){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("type", type);
	retval->setNumber("timestamp", timestamp());
	return retval;
}

void ForensicEvent::deserialize(PassRefPtr<JSONObject> json){
	//WebCapsule::log(WebCapsule::Debug,"ForensicEvent::deserialize %s", json->toJSONString().latin1().data());

	bool result = json->getNumber("timestamp", &m_timestamp);
	ASSERT(result);
}

void ForensicEvent::cancel(){
	setState(Cancelled);
}

void ForensicEvent::injected(){
	setState(Injected);
}

void ForensicEvent::ready(){
	setState(Ready);
}

void ForensicEvent::failed(){
	setState(Failed);
}

ForensicEvent::State ForensicEvent::state(){
	return m_state;
}

void ForensicEvent::setState(State newState){
	m_state = newState;
}

}


