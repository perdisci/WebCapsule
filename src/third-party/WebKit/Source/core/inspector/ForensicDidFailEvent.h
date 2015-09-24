
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicDidFailEvent_h
#define ForensicDidFailEvent_h

#include "core/inspector/ForensicEvent.h"
#include "public/platform/WebURLError.h"

namespace WebCore {

//A call to WebURLLoaderClient::didFail
class ForensicDidFailEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicDidFailEvent> create(blink::WebURLError& error, const double);
	static PassRefPtr<ForensicDidFailEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicDidFailEvent();

	blink::WebURLError& error();

	void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	ForensicDidFailEvent(PassRefPtr<JSONObject> obj);
	ForensicDidFailEvent(blink::WebURLError&, const double);

	blink::WebURLError m_error;
};

} /* namespace WebCore */
#endif /* ForensicReceiveCachedMetadataEvent_h */
