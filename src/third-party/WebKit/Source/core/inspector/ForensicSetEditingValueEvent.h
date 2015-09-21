/*
 * ForensicSetEditingValueEvent.h
 *
 *  Created on: Dec 14, 2014
 *      Author: roberto.perdisci
 */

#ifndef ForensicSetEditingValueEvent_h
#define ForensicSetEditingValueEvent_h

#include "core/inspector/ForensicPageEvent.h"
#include "wtf/text/WTFString.h"


namespace WebCore {

class ForensicSetEditingValueEvent FINAL : public ForensicPageEvent {
public:
	static PassRefPtr<ForensicSetEditingValueEvent> create(const String, const String, const String&, const PageURLRequestMapLocation&, const double);
	static PassRefPtr<ForensicSetEditingValueEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicSetEditingValueEvent();
	
	virtual void accept(ForensicEventVisitor& visitor) OVERRIDE;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

	String inputElementName();
	String value();

private:
	ForensicSetEditingValueEvent(const String, const String, const String&, const PageURLRequestMapLocation&, const double);
	ForensicSetEditingValueEvent(PassRefPtr<JSONObject> obj);

	String m_inputElementName;
	String m_value;
};

} /* namespace WebCore */

#endif
