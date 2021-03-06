
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicClearCacheEvent_h
#define ForensicClearCacheEvent_h

#include "core/inspector/ForensicEvent.h"

namespace WebCore {

class ForensicClearCacheEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicClearCacheEvent> create(const double);
	static PassRefPtr<ForensicClearCacheEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicClearCacheEvent();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	ForensicClearCacheEvent(const double);
	ForensicClearCacheEvent(PassRefPtr<JSONObject> obj);
};

} /* namespace WebCore */
#endif /* ForensicClearCacheEvent_h*/
