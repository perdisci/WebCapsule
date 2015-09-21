/*
 * ForensicRenderMenuListValueChangedEvent.cpp
 *
 *  Created on: Dec 14, 2014
 *      Author: roberto.perdisci
 */

#include "config.h"
#include "core/inspector/ForensicRenderMenuListValueChangedEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {

PassRefPtr<ForensicRenderMenuListValueChangedEvent> ForensicRenderMenuListValueChangedEvent::create(
		unsigned listIndex, bool fireOnChange, const String& te, const String& pe,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp) {
	return adoptRef(new ForensicRenderMenuListValueChangedEvent(listIndex, fireOnChange, te, pe, pageURL, mapLocation, timestamp));
}

PassRefPtr<ForensicRenderMenuListValueChangedEvent> ForensicRenderMenuListValueChangedEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicRenderMenuListValueChangedEvent(obj));
}

ForensicRenderMenuListValueChangedEvent::ForensicRenderMenuListValueChangedEvent(
		unsigned listIndex, bool fireOnChange, const String& te, const String& pe,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp)
	: ForensicPageEvent(pageURL, mapLocation, timestamp)
	, m_listIndex(listIndex)
	, m_fireOnChange(fireOnChange)
	, m_targetElementStr(te)
	, m_parentElementStr(pe)
{

}

ForensicRenderMenuListValueChangedEvent::ForensicRenderMenuListValueChangedEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicRenderMenuListValueChangedEvent::~ForensicRenderMenuListValueChangedEvent()
{

}

void ForensicRenderMenuListValueChangedEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

size_t ForensicRenderMenuListValueChangedEvent::size(){
	return sizeof(m_listIndex)
			+ sizeof(m_fireOnChange)
			+ m_targetElementStr.sizeInBytes()
			+ m_parentElementStr.sizeInBytes()
			+ ForensicPageEvent::size();
}

PassRefPtr<JSONObject> ForensicRenderMenuListValueChangedEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicPageEvent::serialize("ForensicRenderMenuListValueChangedEvent");
	retval->setNumber("listIndex", listIndex());
	retval->setBoolean("fireOnChange", fireOnChange());
	retval->setString("targetElementStr", targetElementStr());
	retval->setString("parentElementStr", parentElementStr());

	return retval;
}

void ForensicRenderMenuListValueChangedEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicPageEvent::deserialize(guard);

	bool result = guard->getNumber("listIndex", &m_listIndex);
	ASSERT(result);

	result = guard->getBoolean("fireOnChange", &m_fireOnChange);
	ASSERT(result);

	result = guard->getString("targetElementStr", &m_targetElementStr);
	ASSERT(result);

	result = guard->getString("parentElementStr", &m_parentElementStr);
	ASSERT(result);
}

unsigned ForensicRenderMenuListValueChangedEvent::listIndex(){
	return m_listIndex;
}

bool ForensicRenderMenuListValueChangedEvent::fireOnChange(){
	return m_fireOnChange;
}

const String& ForensicRenderMenuListValueChangedEvent::targetElementStr(){
        return m_targetElementStr;
}

const String& ForensicRenderMenuListValueChangedEvent::parentElementStr(){
        return m_parentElementStr;
}


} /* namespace WebCore */
