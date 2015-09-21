/*
 * ForensicLoadURLEvent.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicLoadURLEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {

PassRefPtr<ForensicLoadURLEvent> ForensicLoadURLEvent::create(String url, const double timestamp){
	return adoptRef(new ForensicLoadURLEvent(url, timestamp));
}

PassRefPtr<ForensicLoadURLEvent> ForensicLoadURLEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicLoadURLEvent(obj));
}

ForensicLoadURLEvent::ForensicLoadURLEvent(String url, const double timestamp)
	: ForensicEvent(timestamp)
	, m_url(url)
{

}

ForensicLoadURLEvent::ForensicLoadURLEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicLoadURLEvent::~ForensicLoadURLEvent()
{

}

void ForensicLoadURLEvent::accept(ForensicEventVisitor& visitor)
{
	visitor.visit(*this);
}

size_t ForensicLoadURLEvent::size(){
	return m_url.sizeInBytes() + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicLoadURLEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicLoadURLEvent");
	retval->setString("url", url());
	return retval;
}


void ForensicLoadURLEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	bool result = guard->getString("url", &m_url);
	ASSERT(result);
}

String& ForensicLoadURLEvent::url()
{
	return m_url;
}

} /* namespace WebCore */
