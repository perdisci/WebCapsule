/*
 * ForensicLoadURLEvent.h
 *
 *  Created on: Jun 25, 2014
 *      Author: cjneasbi
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
