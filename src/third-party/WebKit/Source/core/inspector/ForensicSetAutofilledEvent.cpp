/*
 * ForensicSetAutofilledEvent.cpp
 *
 *  Created on: Dec 14, 2014
 *      Author: roberto.perdisci
 */

#include "config.h"
#include "core/inspector/ForensicSetAutofilledEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {


PassRefPtr<ForensicSetAutofilledEvent> ForensicSetAutofilledEvent::create(
		const String inputElementName,
		bool autofilled,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp){
	return adoptRef(new ForensicSetAutofilledEvent(inputElementName, autofilled, pageURL, mapLocation, timestamp));
}

PassRefPtr<ForensicSetAutofilledEvent> ForensicSetAutofilledEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicSetAutofilledEvent(obj));
}

ForensicSetAutofilledEvent::ForensicSetAutofilledEvent(const String inputElementName,
		bool autofilled,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp)
	: ForensicPageEvent(pageURL, mapLocation, timestamp)
	, m_inputElementName(inputElementName)
	, m_autofilled(autofilled)
{

}

ForensicSetAutofilledEvent::ForensicSetAutofilledEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicSetAutofilledEvent::~ForensicSetAutofilledEvent()
{

}

void ForensicSetAutofilledEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

size_t ForensicSetAutofilledEvent::size(){
	return m_inputElementName.sizeInBytes() + sizeof(m_autofilled) + ForensicPageEvent::size();
}

PassRefPtr<JSONObject> ForensicSetAutofilledEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicPageEvent::serialize("ForensicSetAutofilledEvent");
	retval->setString("inputElementName", inputElementName());
	retval->setBoolean("isAutofilled", isAutofilled());

	return retval;
}

void ForensicSetAutofilledEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicPageEvent::deserialize(guard);

	bool result = guard->getString("inputElementName", &m_inputElementName);
	ASSERT(result);

	result = guard->getBoolean("isAutofilled", &m_autofilled);
	ASSERT(result);
}

String ForensicSetAutofilledEvent::inputElementName(){
        return m_inputElementName;
}

bool ForensicSetAutofilledEvent::isAutofilled(){
        return m_autofilled;
}


} /* namespace WebCore */
