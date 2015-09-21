/*
 * ForensicReceiveResponseEvent.h
 *
 *  Created on: Jun 11, 2014
 *      Author: cjneasbi
 */

#ifndef ForensicReceiveResponseEvent_h
#define ForensicReceiveResponseEvent_h

#include "core/inspector/ForensicEvent.h"

namespace blink {
class WebURLResponse;
}

namespace WebCore {
class ForensicEventVisitor;

//A call to WebURLLoaderClient::didReceiveResponse
class ForensicReceiveResponseEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicReceiveResponseEvent> create(blink::WebURLResponse&, const double);
	static PassRefPtr<ForensicReceiveResponseEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicReceiveResponseEvent();

	blink::WebURLResponse& response();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;
private:
	ForensicReceiveResponseEvent(blink::WebURLResponse&, const double);
	ForensicReceiveResponseEvent(PassRefPtr<JSONObject> obj);

	blink::WebURLResponse m_response;
};

} /* namespace WebCore */
#endif /* ForensicReceiveResponseEvent_h */
