
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicPageResizeEvent_h
#define ForensicPageResizeEvent_h

#include "core/inspector/ForensicPageEvent.h"
#include "public/platform/WebSize.h"

namespace WebCore {

class ForensicPageResizeEvent : public ForensicPageEvent {
public:
	static PassRefPtr<ForensicPageResizeEvent> create(const blink::WebSize&, const String&, const PageURLRequestMapLocation&, const double);
	static PassRefPtr<ForensicPageResizeEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicPageResizeEvent();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

	const blink::WebSize& webSize();

protected:
	ForensicPageResizeEvent();
	ForensicPageResizeEvent(const blink::WebSize&, const String&, const PageURLRequestMapLocation&, const double);
	ForensicPageResizeEvent(PassRefPtr<JSONObject> obj);

	PassRefPtr<JSONObject> serialize(const String& type) OVERRIDE;

private:
	blink::WebSize m_size;
};

} /* namespace WebCore */
#endif /* ForensicPageResizeEvent_h */
