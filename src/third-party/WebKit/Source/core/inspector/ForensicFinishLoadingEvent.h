/*
 * ForensicFinishLoadingEvent.h
 *
 *  Created on: Jun 11, 2014
 *      Author: cjneasbi
 */

#ifndef ForensicFinishLoadingEvent_h
#define ForensicFinishLoadingEvent_h

#include "core/inspector/ForensicEvent.h"

namespace WebCore {
class ForensicEventVisitor;

//A call to WebURLLoaderClient::didFinishLoading
class ForensicFinishLoadingEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicFinishLoadingEvent> create(double, long, const double);
	static PassRefPtr<ForensicFinishLoadingEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicFinishLoadingEvent();

	double finishTime();
	long encodedDataLength();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	ForensicFinishLoadingEvent(double, long, const double);
	ForensicFinishLoadingEvent(PassRefPtr<JSONObject> obj);

	double m_finishTime;
	long m_encodedDataLength;
};

} /* namespace WebCore */
#endif /* ForensicFinishLoadingEvent_h */
