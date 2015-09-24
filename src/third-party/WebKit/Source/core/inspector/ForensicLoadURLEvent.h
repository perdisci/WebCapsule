
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicLoadURLEvent_h
#define ForensicLoadURLEvent_h

#include "core/inspector/ForensicEvent.h"
#include "wtf/text/WTFString.h"

namespace WebCore {

class InspectorPageAgent;

class ForensicLoadURLEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicLoadURLEvent> create(String url, const double);
	static PassRefPtr<ForensicLoadURLEvent> create(PassRefPtr<JSONObject> obj);

	~ForensicLoadURLEvent();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

	String& url();
private:
	ForensicLoadURLEvent(String url, const double);
	ForensicLoadURLEvent(PassRefPtr<JSONObject> obj);

	String m_url;
};

} /* namespace WebCore */
#endif /* ForensicLoadURLEvent_h */
