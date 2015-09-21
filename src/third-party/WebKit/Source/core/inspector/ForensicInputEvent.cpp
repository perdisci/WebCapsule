/*
 * ForensicInputEvent.cpp
 *
 *  Created on: Apr 1, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicInputEvent.h"
#include "core/inspector/ForensicEventVisitor.h"
#include "wtf/text/Base64.h"

namespace WebCore{

PassRefPtr<ForensicInputEvent> ForensicInputEvent::create(
		const blink::WebInputEvent& inputEvent,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const double timestamp){
	return adoptRef(new ForensicInputEvent(inputEvent, pageURL, mapLocation, String(), timestamp));
}

PassRefPtr<ForensicInputEvent> ForensicInputEvent::create(
		const blink::WebInputEvent& inputEvent,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const String& domTextSnapshot,
		const double timestamp){
	return adoptRef(new ForensicInputEvent(inputEvent, pageURL, mapLocation, domTextSnapshot, timestamp));
}

PassRefPtr<ForensicInputEvent> ForensicInputEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicInputEvent(obj));
}

ForensicInputEvent::ForensicInputEvent(
		const blink::WebInputEvent& inputEvent,
		const String& pageURL,
		const PageURLRequestMapLocation& mapLocation,
		const String& domTextSnapshot,
		const double timestamp)
	: ForensicPageEvent(pageURL, mapLocation, timestamp)
{
	m_inputEvent = copy(inputEvent);
	m_domTextSnapshot = domTextSnapshot;
}


ForensicInputEvent::ForensicInputEvent(PassRefPtr<JSONObject> obj)
{
	deserialize(obj);
}


ForensicInputEvent::~ForensicInputEvent()
{
}

PassOwnPtr<blink::WebInputEvent> ForensicInputEvent::inputEvent(){
	return copy(*(m_inputEvent.get()));
}

const String& ForensicInputEvent::domTextSnapshot() {
	return m_domTextSnapshot;
}

void ForensicInputEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

size_t ForensicInputEvent::size(){
	return m_inputEvent->size + m_domTextSnapshot.sizeInBytes() + ForensicPageEvent::size();
}

PassRefPtr<JSONObject> ForensicInputEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicPageEvent::serialize("ForensicInputEvent");
	OwnPtr<blink::WebInputEvent> event = inputEvent();
	RefPtr<JSONObject> inputEventJSON = JSONObject::create();
	inputEventJSON->setNumber("type", event->type);
	inputEventJSON->setString("data", base64Encode(reinterpret_cast<char*>(event.get()), event->size));
	inputEventJSON->setNumber("size", event->size);
	retval->setObject("inputEvent", inputEventJSON);
	if(!m_domTextSnapshot.isEmpty())
		retval->setString("domTextSnapshot", m_domTextSnapshot);

	return retval;
}

void ForensicInputEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicPageEvent::deserialize(guard);

	RefPtr<JSONObject> inputEventJSON = guard->getObject("inputEvent");
	int type = 0;

	inputEventJSON->getNumber("type", &type);
	m_inputEvent = createEvent(type);

	String strData;
	Vector<char> data;
	inputEventJSON->getString("data", &strData);
	bool result = base64Decode(strData, data);
	ASSERT(result);

	int size = 0;
	inputEventJSON->getNumber("size", &size);

	memcpy(m_inputEvent.get(), data.data(), size);

	String tmpDom;
	bool domExists = guard->getString("domTextSnapshot", &tmpDom);
	if(domExists)
		m_domTextSnapshot = tmpDom;
}

PassOwnPtr<blink::WebInputEvent> ForensicInputEvent::createEvent(int type){
	blink::WebInputEvent* retval;
	if(blink::WebInputEvent::isKeyboardEventType(type)){
		retval = new blink::WebKeyboardEvent();
	} else if(blink::WebInputEvent::isMouseEventType(type)) {
		retval = new blink::WebMouseEvent();
	} else if(blink::WebInputEvent::isGestureEventType(type)){
		retval = new blink::WebGestureEvent();
	} else if(blink::WebInputEvent::isTouchEventType(type)){
		retval = new blink::WebTouchEvent();
	} else {
        retval = new blink::WebMouseWheelEvent();
	}

	return adoptPtr(retval);
}

PassOwnPtr<blink::WebInputEvent> ForensicInputEvent::copy(const blink::WebInputEvent& inputEvent){
	blink::WebInputEvent* retval;
	if(blink::WebInputEvent::isKeyboardEventType(inputEvent.type)){
		retval = new blink::WebKeyboardEvent();
	} else if(blink::WebInputEvent::isMouseEventType(inputEvent.type)) {
		retval = new blink::WebMouseEvent();
	} else if(blink::WebInputEvent::isGestureEventType(inputEvent.type)){
		retval = new blink::WebGestureEvent();
	} else if(blink::WebInputEvent::isTouchEventType(inputEvent.type)){
		retval = new blink::WebTouchEvent();
	} else {
        retval = new blink::WebMouseWheelEvent();
	}

	memcpy(retval, &inputEvent, inputEvent.size);
	return adoptPtr(retval);
}

}
