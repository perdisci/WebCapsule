/*
 * ForensicRedirectEvent.h
 *
 *  Created on: Jul 14, 2014
 *      Author: cjneasbi
 */

#ifndef ForensicRedirectEvent_h
#define ForensicRedirectEvent_h

#include "core/inspector/ForensicEvent.h"
#include "platform/exported/WrappedResourceResponse.h"
#include "platform/exported/WrappedResourceRequest.h"
#include "wtf/OwnPtr.h"
#include "wtf/PassOwnPtr.h"
#include "platform/network/ResourceRequest.h"
#include "platform/network/ResourceResponse.h"

namespace WebCore {
class ForensicEventVisitor;

class ForensicRedirectEvent : public ForensicEvent {
public:
	static PassRefPtr<ForensicRedirectEvent> create(ResourceRequest& request,
			ResourceResponse& response,
			const double timestamp);
	static PassRefPtr<ForensicRedirectEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicRedirectEvent();

	blink::WebURLRequest& request();
	blink::WebURLResponse& response();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;
private:
	ForensicRedirectEvent(ResourceRequest& request,
			ResourceResponse& response,
			const double timestamp);
	ForensicRedirectEvent(PassRefPtr<JSONObject> obj);

	OwnPtr<ResourceRequest> m_request;
	OwnPtr<ResourceResponse> m_response;

	blink::WrappedResourceRequest* m_wrappedRequest;
	blink::WrappedResourceResponse* m_wrappedResponse;
};

} /* namespace WebCore */
#endif /* ForensicRedirectEvent_h */
