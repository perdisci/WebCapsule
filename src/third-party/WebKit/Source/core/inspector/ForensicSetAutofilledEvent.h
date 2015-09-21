/*
 * ForensicSetAutofilledEvent.h
 *
 *  Created on: Dec 14, 2014
 *      Author: roberto.perdisci
 */

#ifndef ForensicSetAutofilledEvent_h
#define ForensicSetAutofilledEvent_h

#include "core/inspector/ForensicPageEvent.h"
#include "wtf/text/WTFString.h"


namespace WebCore {

class ForensicSetAutofilledEvent FINAL : public ForensicPageEvent {
public:
	static PassRefPtr<ForensicSetAutofilledEvent> create(const String, bool, const String&, const PageURLRequestMapLocation&, const double);
	static PassRefPtr<ForensicSetAutofilledEvent> create(PassRefPtr<JSONObject> obj);
	virtual ~ForensicSetAutofilledEvent();
	
	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;


	String inputElementName();
	bool isAutofilled();

private:
	ForensicSetAutofilledEvent(const String, bool, const String&, const PageURLRequestMapLocation&, const double);
	ForensicSetAutofilledEvent(PassRefPtr<JSONObject> obj);

	String m_inputElementName;
	bool m_autofilled;
};

} /* namespace WebCore */

#endif
