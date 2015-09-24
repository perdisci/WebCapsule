
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicRenderMenuListValueChangedEvent_h
#define ForensicRenderMenuListValueChangedEvent_h

#include "core/inspector/ForensicPageEvent.h"
#include "wtf/text/WTFString.h"


namespace WebCore {

class ForensicRenderMenuListValueChangedEvent FINAL : public ForensicPageEvent {
public:
	static PassRefPtr<ForensicRenderMenuListValueChangedEvent> create(
			unsigned listIndex, bool fireOnChange, const String& te, const String& pe,
			const String& pageURL,
			const PageURLRequestMapLocation& mapLocation,
			const double timestamp);
	static PassRefPtr<ForensicRenderMenuListValueChangedEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicRenderMenuListValueChangedEvent();
	
	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

	unsigned listIndex();
	bool fireOnChange();
	const String& targetElementStr();
	const String& parentElementStr();

private:
	ForensicRenderMenuListValueChangedEvent(
			unsigned listIndex, bool fireOnChange, const String& te, const String& pe,
			const String& pageURL,
			const PageURLRequestMapLocation& mapLocation,
			const double timestamp);
	ForensicRenderMenuListValueChangedEvent(PassRefPtr<JSONObject> obj);

	unsigned m_listIndex;
	bool m_fireOnChange;
	String m_targetElementStr;
	String m_parentElementStr;
};

} /* namespace WebCore */

#endif
