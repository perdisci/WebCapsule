
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicRecordingStartEvent_h
#define ForensicRecordingStartEvent_h

#include "core/inspector/ForensicEvent.h"

namespace WebCore {

class ForensicRecordingStartEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicRecordingStartEvent> create(const double, const double);
	static PassRefPtr<ForensicRecordingStartEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicRecordingStartEvent();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

	double monotonicallyIncreasingTimestamp();
private:
	ForensicRecordingStartEvent(const double, const double);
	ForensicRecordingStartEvent(PassRefPtr<JSONObject> obj);

	double m_monotonicallyIncreasingTimestamp;
};

} /* namespace WebCore */
#endif /* ForensicRecordingStartEvent_h */
