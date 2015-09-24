
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicInspectorStateWrapper_h
#define ForensicInspectorStateWrapper_h

#include "wtf/PassOwnPtr.h"
#include "core/inspector/InspectorState.h"

namespace WebCore {

/* This class wraps an InspectorState* with some convenience methods for record and replay. */
class ForensicInspectorStateWrapper {
public:
	static PassOwnPtr<ForensicInspectorStateWrapper> create(InspectorState*);
	static PassOwnPtr<ForensicInspectorStateWrapper> create(ForensicInspectorStateWrapper&);
	virtual ~ForensicInspectorStateWrapper();

	bool recording();
	bool recording(long);

	bool replaying();
	bool replaying(long);

	long stateId();

private:
	ForensicInspectorStateWrapper(InspectorState*);

	bool checkStateId(long);
	InspectorState* m_state;
};

} /* namespace WebCore */
#endif /* ForensicInspectorStateWrapper_h */
