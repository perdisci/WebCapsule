/*
 * ForensicSetEditingValueEvent.cpp
 *
 *  Created on: Dec 14, 2014
 *      Author: roberto.perdisci
 */

#include "config.h"
#include "core/inspector/ForensicSetEditingValueEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {

PassRefPtr<ForensicSetEditingValueEvent> ForensicSetEditingValueEvent::create(
		const String inputElementName,
		const String value,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp){
	return adoptRef(new ForensicSetEditingValueEvent(inputElementName, value, pageURL, mapLocation, timestamp));
}

PassRefPtr<ForensicSetEditingValueEvent> ForensicSetEditingValueEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicSetEditingValueEvent(obj));
}

ForensicSetEditingValueEvent::ForensicSetEditingValueEvent(
		const String inputElementName,
		const String value,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp)
	: ForensicPageEvent(pageURL, mapLocation, timestamp)
	, m_inputElementName(inputElementName)
	, m_value(value)
{

}

ForensicSetEditingValueEvent::ForensicSetEditingValueEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicSetEditingValueEvent::~ForensicSetEditingValueEvent()
{

}

void ForensicSetEditingValueEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

size_t ForensicSetEditingValueEvent::size(){
	return m_inputElementName.sizeInBytes() + m_value.sizeInBytes() + ForensicPageEvent::size();
}

PassRefPtr<JSONObject> ForensicSetEditingValueEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicPageEvent::serialize("ForensicSetEditingValueEvent");
	retval->setString("inputElementName", inputElementName());
	retval->setString("value", value());

	return retval;
}

void ForensicSetEditingValueEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicPageEvent::deserialize(guard);

	bool result = guard->getString("inputElementName", &m_inputElementName);
	ASSERT(result);

	result = guard->getString("value", &m_value);
	ASSERT(result);
}

String ForensicSetEditingValueEvent::inputElementName(){
	return m_inputElementName;
}

String ForensicSetEditingValueEvent::value(){
        return m_value;
}


} /* namespace WebCore */
