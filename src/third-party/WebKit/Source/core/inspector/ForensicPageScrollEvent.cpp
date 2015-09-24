
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicPageScrollEvent.h"
#include "core/inspector/ForensicEventVisitor.h"

namespace WebCore {


PassRefPtr<ForensicPageScrollEvent> ForensicPageScrollEvent::create(
		const blink::WebSize& size,
		double pageScaleDelta,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp){
	return adoptRef(new ForensicPageScrollEvent(size, pageScaleDelta, pageURL, mapLocation, timestamp));
}

PassRefPtr<ForensicPageScrollEvent> ForensicPageScrollEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicPageScrollEvent(obj));
}

ForensicPageScrollEvent::ForensicPageScrollEvent(
		const blink::WebSize& size,
		double pageScaleDelta,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp)
	: ForensicPageResizeEvent(size, pageURL, mapLocation, timestamp)
	, m_pageScaleDelta(pageScaleDelta)
{

}

ForensicPageScrollEvent::ForensicPageScrollEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}

ForensicPageScrollEvent::~ForensicPageScrollEvent()
{

}

void ForensicPageScrollEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

size_t ForensicPageScrollEvent::size(){
	return sizeof(m_pageScaleDelta) + ForensicPageResizeEvent::size();
}

PassRefPtr<JSONObject> ForensicPageScrollEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicPageResizeEvent::serialize("ForensicPageScrollEvent");
	retval->setNumber("pageScaleDelta", pageScaleDelta());

	return retval;
}

void ForensicPageScrollEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicPageResizeEvent::deserialize(guard);

	double temp;
	bool result = guard->getNumber("pageScaleDelta", &temp);
	ASSERT(result);

	m_pageScaleDelta = temp;
}

float ForensicPageScrollEvent::pageScaleDelta(){
	return m_pageScaleDelta;
}

} /* namespace WebCore */
