
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicEvent_h
#define ForensicEvent_h

#include "wtf/ThreadSafeRefCounted.h"
#include "wtf/RefPtr.h"
#include "platform/JSONValues.h"
#include "core/inspector/ForensicSerialization.h"

namespace WebCore{
class ForensicEventVisitor;

class ForensicEvent : public ThreadSafeRefCounted<ForensicEvent> , public Serializable {
public:
	enum State {
		Ready,
		Injected,
		Cancelled,
		Diverged,
		Failed
	};

	virtual ~ForensicEvent() {};

	double timestamp() FINAL;

	virtual void accept(ForensicEventVisitor&) = 0;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize();
	virtual void deserialize(PassRefPtr<JSONObject> json);

	void cancel() FINAL;
	void injected() FINAL;
	void ready() FINAL;
	void diverged() FINAL;
	void failed() FINAL;
	State state() FINAL;

protected:
	ForensicEvent();
	ForensicEvent(double timestamp);
	ForensicEvent(PassRefPtr<JSONObject> obj);

	PassRefPtr<JSONObject> serialize(const String& type);

private:
	void setState(State) FINAL;

	double m_timestamp;
	State m_state;
};

}

#endif /* ForensicEvent_h */
