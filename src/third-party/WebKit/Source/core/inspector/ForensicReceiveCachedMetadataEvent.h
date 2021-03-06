
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicReceiveCachedMetadataEvent_h
#define ForensicReceiveCachedMetadataEvent_h

#include "core/inspector/ForensicEvent.h"
#include "wtf/Vector.h"

namespace WebCore {
class ForensicEventVisitor;

class ForensicReceiveCachedMetadataEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicReceiveCachedMetadataEvent> create(const char*,
			int,
			const double);
	static PassRefPtr<ForensicReceiveCachedMetadataEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicReceiveCachedMetadataEvent();

	const Vector<char>& data();

	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	ForensicReceiveCachedMetadataEvent(PassRefPtr<JSONObject> obj);
	ForensicReceiveCachedMetadataEvent(const char*,
			int,
			const double);

	Vector<char> m_data;
};

} /* namespace WebCore */
#endif /* ForensicReceiveCachedMetadataEvent_h */
