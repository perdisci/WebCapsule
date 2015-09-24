
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicEventVisitor_h
#define ForensicEventVisitor_h

namespace WebCore{
class ForensicInputEvent;
class ForensicReceiveCachedMetadataEvent;
class ForensicDidFailEvent;
class ForensicReceiveResponseEvent;
class ForensicFinishLoadingEvent;
class ForensicReceiveDataEvent;
class ForensicDownloadDataEvent;
class ForensicRedirectEvent;
class ForensicClearCacheEvent;
class ForensicLoadURLEvent;
class ForensicPageScrollEvent;
class ForensicPageResizeEvent;
class ForensicReplayStopEvent;
class ForensicRecordingStartEvent;
class ForensicSetEditingValueEvent;
class ForensicSetAutofilledEvent;
class ForensicLoadHistoryItemEvent;
class ForensicRenderMenuListValueChangedEvent;

class ForensicEventVisitor{
public:
	virtual ~ForensicEventVisitor(){};
	//user input
	virtual void visit(ForensicInputEvent& inputEvent) = 0;

	//search input autofill
	virtual void visit(ForensicSetEditingValueEvent& inputEvent) = 0;
	virtual void visit(ForensicSetAutofilledEvent& inputEvent) = 0;

	//popup menu selection
	virtual void visit(ForensicRenderMenuListValueChangedEvent&) = 0;

	//async network
	virtual void visit(ForensicReceiveCachedMetadataEvent& inputEvent) = 0;
	virtual void visit(ForensicDidFailEvent& inputEvent) = 0;
	virtual void visit(ForensicReceiveResponseEvent& inputEvent) = 0;
	virtual void visit(ForensicFinishLoadingEvent& inputEvent) = 0;
	virtual void visit(ForensicReceiveDataEvent& inputEvent) = 0;
	virtual void visit(ForensicDownloadDataEvent& inputEvent) = 0;
	virtual void visit(ForensicRedirectEvent& inputEvent) = 0;

	//cache
	virtual void visit(ForensicClearCacheEvent& inputEvent) = 0;

	//page load
	virtual void visit(ForensicLoadURLEvent& inputEvent) = 0;

	//back/forward button
	virtual void visit(ForensicLoadHistoryItemEvent& inputEvent) = 0;

	//page visual state
	virtual void visit(ForensicPageScrollEvent& inputEvent) = 0;
	virtual void visit(ForensicPageResizeEvent& inputEvent) = 0;

	//replay control
	virtual void visit(ForensicReplayStopEvent& inputEvent) = 0;

	//informational
	virtual void visit(ForensicRecordingStartEvent& inputEvent) = 0;
};


}

#endif /* ForensicEventVisitor_h */
