
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicInputEvent_h
#define ForensicInputEvent_h

#include "core/inspector/ForensicPageEvent.h"
#include "public/web/WebInputEvent.h"

namespace WebCore {
class ForensicEventVisitor;

class ForensicInputEvent FINAL : public ForensicPageEvent {
public:
	static PassRefPtr<ForensicInputEvent> create(const blink::WebInputEvent&, const String&, const PageURLRequestMapLocation&, const double);
	static PassRefPtr<ForensicInputEvent> create(const blink::WebInputEvent&, const String&, const PageURLRequestMapLocation&, const String&, const double);
	static PassRefPtr<ForensicInputEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicInputEvent();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

	//Returns a copy of the stored WebInputEvent
	//We return a copy to enable multiple replays of the same input.
	PassOwnPtr<blink::WebInputEvent> inputEvent();
	const String& domTextSnapshot();

private:
	ForensicInputEvent(const blink::WebInputEvent&, const String&, const PageURLRequestMapLocation&, const String&, const double);
	ForensicInputEvent(PassRefPtr<JSONObject> obj);

	OwnPtr<blink::WebInputEvent> m_inputEvent;
	String m_domTextSnapshot;

	//Creates a full copy of the WebInputEvent
	PassOwnPtr<blink::WebInputEvent> copy(const blink::WebInputEvent&);

	PassOwnPtr<blink::WebInputEvent> createEvent(int type);
};


}

#endif /* ForensicInputEvent_h */
