
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicLoadHistoryItemEvent_h
#define ForensicLoadHistoryItemEvent_h

#include "core/inspector/ForensicEvent.h"
#include "wtf/PassRefPtr.h"
#include "wtf/text/WTFString.h"
#include "core/loader/HistoryItem.h"
#include "core/loader/FrameLoaderTypes.h"
#include "core/frame/LocalFrame.h"
#include "platform/network/ResourceRequest.h"


namespace WebCore {

class ForensicLoadHistoryItemEvent FINAL : public ForensicEvent {
public:
	static PassRefPtr<ForensicLoadHistoryItemEvent> create(/*LocalFrame* frame,*/ HistoryItem* item, HistoryLoadType historyLoadType, ResourceRequestCachePolicy cachePolicy, double timestamp);
	static PassRefPtr<ForensicLoadHistoryItemEvent> create(PassRefPtr<JSONObject> obj);

	virtual ~ForensicLoadHistoryItemEvent();
	
	void accept(ForensicEventVisitor& visitor);
	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

	//LocalFrame* frame();
	HistoryItem* item();
	HistoryLoadType historyLoadType();
	ResourceRequestCachePolicy cachePolicy();

private:
	ForensicLoadHistoryItemEvent(/*LocalFrame* frame,*/ HistoryItem* item, HistoryLoadType historyLoadType, ResourceRequestCachePolicy cachePolicy, double timestamp);
	ForensicLoadHistoryItemEvent(PassRefPtr<JSONObject> obj);

    void copyHistoryItem(HistoryItem* item);

	//LocalFrame* m_frame;
	RefPtr<HistoryItem> m_item;
	HistoryLoadType m_historyLoadType;
	ResourceRequestCachePolicy m_cachePolicy;
};

} /* namespace WebCore */

#endif
