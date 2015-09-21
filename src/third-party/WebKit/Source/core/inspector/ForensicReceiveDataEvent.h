/*
 * ForensicReceiveDataEvent.h
 *
 *  Created on: Jun 12, 2014
 *      Author: cjneasbi
 */

#ifndef ForensicReceiveDataEvent_h
#define ForensicReceiveDataEvent_h

#include "core/inspector/ForensicEvent.h"
#include "wtf/Vector.h"

namespace WebCore {
class ForensicEventVisitor;

class ForensicReceiveDataEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicReceiveDataEvent> create(const char*,
			int,
			int,
			const double);
	static PassRefPtr<ForensicReceiveDataEvent> create(PassRefPtr<JSONObject> obj);


	virtual ~ForensicReceiveDataEvent();

	const Vector<char>& data();
	int encodedDataLength();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	ForensicReceiveDataEvent(const char*,
			int,
			int,
			const double);
	ForensicReceiveDataEvent(PassRefPtr<JSONObject> obj);

	Vector<char> m_data;
	int m_encodedDataLength;

};

} /* namespace WebCore */
#endif /* FORENSICRECEIVEDATAEVENT_H_ */
