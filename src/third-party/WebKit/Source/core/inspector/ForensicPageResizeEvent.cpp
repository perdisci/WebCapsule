/*
 * ForensicPageResizeEvent.cpp
 *
 *  Created on: Jul 14, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicPageResizeEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {


PassRefPtr<ForensicPageResizeEvent> ForensicPageResizeEvent::create(
		const blink::WebSize& size,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp){
	return adoptRef(new ForensicPageResizeEvent(size, pageURL, mapLocation, timestamp));
}

PassRefPtr<ForensicPageResizeEvent> ForensicPageResizeEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicPageResizeEvent(obj));
}

ForensicPageResizeEvent::ForensicPageResizeEvent()
{
}

ForensicPageResizeEvent::ForensicPageResizeEvent(
		const blink::WebSize& size,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp)
	: ForensicPageEvent(pageURL, mapLocation, timestamp)
	, m_size(size.width, size.height)
{

}

ForensicPageResizeEvent::ForensicPageResizeEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicPageResizeEvent::~ForensicPageResizeEvent()
{

}

void ForensicPageResizeEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

size_t ForensicPageResizeEvent::size(){
	return sizeof(m_size) + ForensicPageEvent::size();
}

PassRefPtr<JSONObject> ForensicPageResizeEvent::serialize(){
	return serialize("ForensicPageResizeEvent");
}

PassRefPtr<JSONObject>  ForensicPageResizeEvent::serialize(const String& type){
	RefPtr<JSONObject> retval = ForensicPageEvent::serialize(type);
	const blink::WebSize& sizeVal = webSize();
	RefPtr<JSONObject> sizeJSON = JSONObject::create();
	sizeJSON->setNumber("height", sizeVal.height);
	sizeJSON->setNumber("width", sizeVal.width);

	retval->setObject("size", sizeJSON);

	return retval;
}


void ForensicPageResizeEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicPageEvent::deserialize(guard);

	RefPtr<JSONObject> obj = guard->getObject("size");
	ASSERT(obj);

	int width, height;
	bool result = obj->getNumber("height", &height);
	ASSERT(result);

	result = obj->getNumber("width", &width);
	ASSERT(result);

	m_size = blink::WebSize(width, height);
}

const blink::WebSize& ForensicPageResizeEvent::webSize(){
	return m_size;
}

} /* namespace WebCore */
