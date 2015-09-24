
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicEventInjector_h
#define ForensicEventInjector_h

#include "core/inspector/ForensicEventVisitor.h"
#include "wtf/RefPtr.h"
#include "wtf/ThreadSafeRefCounted.h"
#include "core/inspector/WebCapsule.h"
#include "core/inspector/ForensicReplayDataStore.h"
#include "platform/weborigin/KURL.h"

namespace blink{
class WebViewImpl;
class WebURLLoader;
class WebURLLoaderClient;
}

namespace WebCore{
class Page;
class InspectorClient;
class InspectorPageAgent;
class InspectorForensicsAgent;
class ForensicPageEvent;
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
class ForensicSetEditingValueEvent;
class ForensicSetAutofilledEvent;
class ForensicRenderMenuListValueChangedEvent;

class ForensicEventInjector : public ForensicEventVisitor, public ThreadSafeRefCounted<ForensicEventInjector> {
public:
	static PassRefPtr<ForensicEventInjector> create(Page* = 0,
			InspectorClient* = 0,
			InspectorPageAgent* = 0,
			InspectorForensicsAgent* = 0,
			ForensicReplayDataStore* = 0,
			bool doCompareDOMTextSnapshot = false,
			blink::WebURLLoader* = 0,
			blink::WebURLLoaderClient* = 0);

	static PassRefPtr<ForensicEventInjector> create(blink::WebURLLoader*, blink::WebURLLoaderClient*);

	~ForensicEventInjector(){};
	//user input
	void visit(ForensicInputEvent&);

	//search input autofill
	void visit(ForensicSetEditingValueEvent&);
	void visit(ForensicSetAutofilledEvent&);

	//popup menu selection
	void visit(ForensicRenderMenuListValueChangedEvent&);

	//back/forward button events
	void visit(ForensicLoadHistoryItemEvent&);

	//async network
	void visit(ForensicReceiveCachedMetadataEvent&);
	void visit(ForensicDidFailEvent&);
	void visit(ForensicReceiveResponseEvent&);
	void visit(ForensicFinishLoadingEvent&);
	void visit(ForensicReceiveDataEvent& inputEvent);
	void visit(ForensicDownloadDataEvent& inputEvent);
	void visit(ForensicRedirectEvent& inputEvent);

	//cache
	void visit(ForensicClearCacheEvent& inputEvent);

	//page load
	void visit(ForensicLoadURLEvent& inputEvent);

	//page visual state
	void visit(ForensicPageScrollEvent& inputEvent);
	void visit(ForensicPageResizeEvent& inputEvent);

	//replay control
	void visit(ForensicReplayStopEvent& inputEvent);

	//informational, these should be no-ops
	void visit(ForensicRecordingStartEvent& inputEvent){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicRecordingStartEvent.");
	};

private:
	ForensicEventInjector(
			Page*,
			InspectorClient*,
			InspectorPageAgent*,
			InspectorForensicsAgent*,
			ForensicReplayDataStore*,
			bool doCompareDOMTextSnapshot,
			blink::WebURLLoader*,
			blink::WebURLLoaderClient*);

	Page* m_page;
	InspectorClient* m_client;
	InspectorPageAgent* m_pageAgent;
	InspectorForensicsAgent* m_forensicsAgent;
	ForensicReplayDataStore* m_dataStore;
	bool m_doCompareDOMTextSnapshot;
	blink::WebURLLoader* m_loader;
	blink::WebURLLoaderClient* m_loaderClient;
	
	bool hasPageEventReplayIncongruence(ForensicPageEvent& inputEvent);
	void attemptReplaySelfHealing(ForensicPageEvent& inputEvent);
	void doReplaySelfHealing(ForensicPageEvent& inputEvent);

	void forceLoadFrame(blink::WebViewImpl* webView, const KURL& url);
	void forceLoadFrame(blink::WebViewImpl* webView, const blink::WebURLRequest& request);

};

}


#endif /* ForensicEventInjector_h */
