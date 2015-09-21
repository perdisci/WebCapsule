/*
 * ForensicPageEvent.h
 *
 *  Created on: Jul 14, 2014
 *      Author: cjneasbi
 */

#ifndef ForensicPageEvent_h
#define ForensicPageEvent_h

#include "core/inspector/ForensicEvent.h"
#include "wtf/PassRefPtr.h"
#include "core/inspector/ForensicReplayDataStore.h"

namespace WebCore {

class Page;

class ForensicPageEvent : public ForensicEvent {
public:
	virtual ~ForensicPageEvent();

	virtual void accept(ForensicEventVisitor&) = 0;
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize();
	virtual void deserialize(PassRefPtr<JSONObject> json);

	Page* page();

	const String& pageURL(); // returns the URL or the page where the event was recorded
	const PageURLRequestMapLocation& pageURLRequestMapLocation();

protected:
    ForensicPageEvent();
	ForensicPageEvent(const String&, const PageURLRequestMapLocation&, const double);
	PassRefPtr<JSONObject> serialize(const String& type);

private:
	String m_pageURL;
	PageURLRequestMapLocation m_mapLocation;
};

} /* namespace WebCore */
#endif /* ForensicPageEvent_h */
