
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicPageScrollEvent_h
#define ForensicPageScrollEvent_h

#include "core/inspector/ForensicPageResizeEvent.h"
#include "public/platform/WebSize.h"

namespace WebCore {
class ForensicEventVisitor;

class ForensicPageScrollEvent FINAL : public ForensicPageResizeEvent
{
public:
	static PassRefPtr<ForensicPageScrollEvent> create(const blink::WebSize&, double, const String&, const PageURLRequestMapLocation&, const double);
	static PassRefPtr<ForensicPageScrollEvent> create(PassRefPtr<JSONObject> obj);
	virtual ~ForensicPageScrollEvent();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;


	float pageScaleDelta();
private:
	ForensicPageScrollEvent(const blink::WebSize&, double, const String&, const PageURLRequestMapLocation&, const double);
	ForensicPageScrollEvent(PassRefPtr<JSONObject> obj);

	float m_pageScaleDelta;

};

} /* namespace WebCore */
#endif /* ForensicPageScrollEvent_h */
