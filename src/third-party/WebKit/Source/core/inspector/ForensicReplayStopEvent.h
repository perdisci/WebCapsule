
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicReplayStopEvent_h
#define ForensicReplayStopEvent_h

#include "core/inspector/ForensicEvent.h"

namespace WebCore {

class ForensicReplayStopEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicReplayStopEvent> create(const double);
	static PassRefPtr<ForensicReplayStopEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicReplayStopEvent();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	ForensicReplayStopEvent(const double);
	ForensicReplayStopEvent(PassRefPtr<JSONObject> obj);
};

} /* namespace WebCore */
#endif /* ForensicReplayStopEvent_h */
