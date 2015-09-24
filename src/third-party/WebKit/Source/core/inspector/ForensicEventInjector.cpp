
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicEventInjector.h"
#include "web/WebViewImpl.h"
#include "platform/Logging.h"
#include "public/platform/Platform.h"
#include "public/platform/WebURLLoaderClient.h"
#include "public/platform/WebURLResponse.h"
#include "public/platform/WebURLRequest.h"
#include "public/platform/WebURLError.h"
#include "public/platform/WebString.h"
#include "public/web/WebElement.h"
#include "public/web/WebInputElement.h"
#include "core/dom/Element.h"
#include "core/html/HTMLSelectElement.h"
#include "core/fetch/MemoryCache.h"
#include "core/fetch/ResourceFetcher.h"
#include "core/frame/LocalFrame.h"
#include "core/inspector/ForensicPageEvent.h"
#include "core/inspector/ForensicInputEvent.h"
#include "core/inspector/ForensicReceiveCachedMetadataEvent.h"
#include "core/inspector/ForensicDidFailEvent.h"
#include "core/inspector/ForensicReceiveResponseEvent.h"
#include "core/inspector/ForensicFinishLoadingEvent.h"
#include "core/inspector/ForensicReceiveDataEvent.h"
#include "core/inspector/ForensicDownloadDataEvent.h"
#include "core/inspector/ForensicRedirectEvent.h"
#include "core/inspector/ForensicClearCacheEvent.h"
#include "core/inspector/ForensicLoadURLEvent.h"
#include "core/inspector/ForensicPageScrollEvent.h"
#include "core/inspector/ForensicPageResizeEvent.h"
#include "core/inspector/ForensicReplayStopEvent.h"
#include "core/inspector/ForensicRenderMenuListValueChangedEvent.h"
#include "core/inspector/ForensicSetEditingValueEvent.h"
#include "core/inspector/ForensicSetAutofilledEvent.h"
#include "core/inspector/ForensicLoadHistoryItemEvent.h"
#include "core/inspector/InspectorPageAgent.h"
#include "core/inspector/InspectorForensicsAgent.h"
#include "core/inspector/WebCapsule.h"
#include "platform/weborigin/KURL.h"
#include "web/tests/FrameTestHelpers.h"


namespace WebCore{

PassRefPtr<ForensicEventInjector> ForensicEventInjector::create(Page* page,
		InspectorClient* client,
		InspectorPageAgent* pageAgent,
		InspectorForensicsAgent* forensicsAgent,
		ForensicReplayDataStore* dataStore,
		bool doCompareDOMTextSnapshot,
		blink::WebURLLoader* loader,
		blink::WebURLLoaderClient* loaderClient) {
	return adoptRef(new ForensicEventInjector(page, client, pageAgent, forensicsAgent, dataStore, doCompareDOMTextSnapshot, loader, loaderClient));
}

PassRefPtr<ForensicEventInjector> ForensicEventInjector::create(blink::WebURLLoader* loader, blink::WebURLLoaderClient* loaderClient){
	return create(0, 0, 0, 0, 0, false, loader, loaderClient);
}

ForensicEventInjector::ForensicEventInjector(Page* page,
		InspectorClient* client,
		InspectorPageAgent* pageAgent,
		InspectorForensicsAgent* forensicsAgent,
		ForensicReplayDataStore* dataStore,
		bool doCompareDOMTextSnapshot,
		blink::WebURLLoader* loader,
		blink::WebURLLoaderClient* loaderClient)
	: m_page(page)
	, m_client(client)
	, m_pageAgent(pageAgent)
	, m_forensicsAgent(forensicsAgent)
	, m_dataStore(dataStore)
	, m_doCompareDOMTextSnapshot(doCompareDOMTextSnapshot)
	, m_loader(loader)
	, m_loaderClient(loaderClient)
{

}

void ForensicEventInjector::forceLoadFrame(blink::WebViewImpl* webView, const KURL& url) {

    // borrow code already available //src/third_party/WebKit/Source/web/tests/FrameTestHelpers.cpp
    blink::WebURLRequest urlRequest;
    urlRequest.initialize();
    urlRequest.setURL(url);
    forceLoadFrame(webView, urlRequest);

}

void ForensicEventInjector::forceLoadFrame(blink::WebViewImpl* webView, const blink::WebURLRequest& request) {

    webView->mainFrame()->loadRequest(request);

}

// private
bool ForensicEventInjector::hasPageEventReplayIncongruence(ForensicPageEvent& inputEvent) {

	ASSERT(m_page);
	const KURL& currentPageURL = m_page->mainFrame()->document()->loader()->url();


	if(currentPageURL.string() != inputEvent.pageURL()) {
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector::hasPageEventReplayIncongruence : POSSIBLE INCONGRUENCE : currentURL=%s != recordedURL=%s", currentPageURL.string().latin1().data(), inputEvent.pageURL().latin1().data());

		// FIXME: currently, we only support a single alias per URL
		// in theory, there may exist a chain of aliases; currently we do not support them
		// in practice, we have not encountered such alias chains, even in highly complex sites
		// as supporting them would introduce further complexity, and problems can occur only during replay, we leave this for future work
		ASSERT(m_dataStore);
		String urlRootAlias = m_dataStore->findPageURLAliasChainRoot(currentPageURL.string());
		if(urlRootAlias != currentPageURL.string()) {
				WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector::hasPageEventReplayIncongruence : URL ALIAS FOUND : currentURL=%s != recordedURL=%s", currentPageURL.string().latin1().data(), urlRootAlias.latin1().data());
		}

		// FIXME: in theory we should compare to all alias URLs in the alias chain, not just the root
		// but in practice this should suffice
		if(currentPageURL.string() != inputEvent.pageURL() && urlRootAlias != inputEvent.pageURL()) {
			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector::hasPageEventReplayIncongruence : REPLAY INCONGRUENCE : currentURL=%s != recordedURL=%s", currentPageURL.string().latin1().data(), inputEvent.pageURL().latin1().data());
			return true;
		}
	}

	return false;
}

// private
void ForensicEventInjector::attemptReplaySelfHealing(ForensicPageEvent& inputEvent) {
	ASSERT(m_page);
	blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(m_page);
	ASSERT(webView);

	RefPtr<PageURLRequest> pageURLRequest = m_dataStore->getPageURLRequest(inputEvent.pageURLRequestMapLocation());

	// ASSERT(!inputEvent.pageURLRequest()->isEmpty());
	// XXX: WORKAROUND (@Roberto) we should use the above ASSERT, instead (but we can't due to HTML5's strange features that allow replacing URL without (re)loading the page/frame
	if(!pageURLRequest->isEmpty() && !pageURLRequest->isConsumed()) {
		// XXX: WORKAROUND (@Roberto) force page load!
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector::attemptReplaySelfHealing : REPLAY INCONGRUENCE SELF-HEALING : calling forceLoadFrame - %s ", pageURLRequest->webURLRequest().url().string().latin1().data());
		pageURLRequest->setIsConsumed(true); // makes sure this is not used for another forceLoadFrame
        // TODO: notice that setting isConsumed for the pageURLRequest will prevent to perform self-healing for multiple replays
        // When we start a new replay (e.g., the second replay), we need to clear out all the isConsumed flags...

		forceLoadFrame(webView, pageURLRequest->webURLRequest()); // be careful here, about passing WebURLRequest& !!! Should we make a deep copy instead?

		// TODO ideally, we should somehow stop here and check/wait until the "forced" request has actually begun loading,
		// before we continue injecting events into the page (it probably requires relatively more complex logic)
	} else if(pageURLRequest->isConsumed()){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector::attemptReplaySelfHealing : REPLAY INCONGRUENCE SELF-HEALING : cannot attempt to resolve (already consumed PageURLRequest) : %s", pageURLRequest->webURLRequest().url().string().latin1().data());
	} else {
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector::attemptReplaySelfHealing : REPLAY INCONGRUENCE SELF-HEALING : cannot attempt to resolve (empty PageURLRequest)");
	}
}

void ForensicEventInjector::doReplaySelfHealing(ForensicPageEvent& inputEvent){
	if(!m_dataStore){
		return;
	}

	bool replayIncongruence = hasPageEventReplayIncongruence(inputEvent);
	if(replayIncongruence)
		attemptReplaySelfHealing(inputEvent); // attempt self-healing before injecting the next event
}

void ForensicEventInjector::visit(ForensicInputEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	ASSERT(m_page);
	blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(m_page);

	doReplaySelfHealing(inputEvent);

	if(webView){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting input event.");

		OwnPtr<blink::WebInputEvent> ie = inputEvent.inputEvent();

		/*
    	String keyID;
    	if(ie->type == blink::WebInputEvent::Char) {
    		OwnPtr<blink::WebInputEvent> ke = inputEvent.inputEvent();
    		const blink::WebKeyboardEvent& key = *static_cast<const blink::WebKeyboardEvent*>(ke.get());
    		keyID = key.keyIdentifier;
    		String keyText(key.text);
            WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"ForensicEventInjector::visit : keyID=%s keyText=%s", keyID.utf8().data(), keyText.utf8().data());
    	}

	    if(ie->type == blink::WebInputEvent::MouseDown || (ie->type == blink::WebInputEvent::Char && keyID == "Enter")) {

	        String domTextSnapshot;
            if(m_doCompareDOMTextSnapshot) {
            	domTextSnapshot.append("\n<!--- WEBCAPSULE : DOM_SNAPTHOT : BEGIN : STATE=REPLAY --->\n");
            	InspectorForensicsAgent::takeDOMTextSnapshot(m_page, domTextSnapshot, ie->timeStampSeconds);
            	domTextSnapshot.append("\n<!--- WEBCAPSULE : DOM_SNAPTHOT : END : STATE=REPLAY --->\n");
            }

            // TODO: we could use a DOM tree edit distance to compute how much the DOM snapshots differ
            // for example using the algorithm proposed in http://wwwconference.org/www2004/docs/1p502.pdf
            if(domTextSnapshot != inputEvent.domTextSnapshot()) {
                WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector::visit : REPLAY INCONGRUENCE : domTextSnapshot did not match! currentURL=%s", inputEvent.pageURL().utf8().data());
                WebCapsule::log(WebCapsule::DebugLogLevel,"\n\n\n%s\n\n\n%s\n\n\n", inputEvent.domTextSnapshot().utf8().data(), domTextSnapshot.utf8().data());
            }
            else {
                WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleInputEvent : domTextSnapshot matched perfectly! currentURL=%s", inputEvent.pageURL().utf8().data());
            }
	    }
	    */

		//Update the time of the event
		//Not strictly necessary from a replay POV, but good idea for correct performance analysis
		//ie.get()->timeStampSeconds = blink::Platform::current()->currentTime();
		bool retval = webView->handleInputEvent(*(ie.get()));
	    WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: handled input event: %d", retval);
	} else {
		inputEvent.failed();
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting input event failed.");
	}
}


void ForensicEventInjector::visit(ForensicRenderMenuListValueChangedEvent& inputEvent){
    if(inputEvent.state() != ForensicEvent::Ready) return;

    bool replayIncongruence = hasPageEventReplayIncongruence(inputEvent);
    if(replayIncongruence)
        attemptReplaySelfHealing(inputEvent); // attempt self-healing before injecting the next event

    //blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(inputEvent.page());
    ASSERT(m_page);
    blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(m_page);

    if(webView){

        WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector:injecting ForensicRenderMenuListValueChangedEvent");

        // How do we inject the event back into the page???
        // need to find the element first, and then call the setEditingValue function with the right arguments
        Element* element = webView->focusedElement();
        if(element && element->isFormControlElement()) {
            HTMLSelectElement* e = static_cast<HTMLSelectElement*>(element);
            e->optionSelectedByUser(inputEvent.listIndex(), inputEvent.fireOnChange(), false);
            WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector:injected ForensicRenderMenuListValueChangedEvent (found valid element)");
            inputEvent.injected();
        }
        else {
        	inputEvent.failed();
        }
    } else {
        inputEvent.failed();
    }
}



void ForensicEventInjector::visit(ForensicSetEditingValueEvent& inputEvent){
        if(inputEvent.state() != ForensicEvent::Ready) return;

        doReplaySelfHealing(inputEvent);

        //blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(inputEvent.page());
    	ASSERT(m_page);
    	blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(m_page);

        if(webView){

			WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicSetEditingValue. InputElementName: %s Value: %s",  inputEvent.inputElementName().latin1().data(), inputEvent.value().latin1().data());

			// How do we inject the event back into the page???
			// need to find the element first, and then call the setEditingValue function with the right arguments
			Element* element = webView->focusedElement();
			ASSERT(element);

			// We need to convert Element to a WebInputElement first...
			blink::WebElement webElement(element);
			blink::WebInputElement* inputElement = blink::toWebInputElement(&webElement);
			ASSERT(inputElement);

			blink::WebString afName(inputElement->nameForAutofill());
			String afNameString(afName.latin1().data(),afName.length());
			ASSERT(afNameString == inputEvent.inputElementName());

			// Now we can replay setEditingValue...
			inputElement->setEditingValue(blink::WebString(inputEvent.value()));

			inputEvent.injected();
        } else {
        	inputEvent.failed();
        }
}


void ForensicEventInjector::visit(ForensicSetAutofilledEvent& inputEvent){
        if(inputEvent.state() != ForensicEvent::Ready) return;

        doReplaySelfHealing(inputEvent);

        //blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(inputEvent.page());
    	ASSERT(m_page);
    	blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(m_page);

        if(webView){
                WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicSetAutofilled. InputElementName: %s Autofilled: %d",  inputEvent.inputElementName().latin1().data(), inputEvent.isAutofilled());

                // How do we inject the event back into the page???
                // need to find the element first, and then call the setEditingValue function with the right arguments
                Element* element = webView->focusedElement();

                ASSERT(element);
                /* 
                if(!element) { // XXX: WORKAROUND: we can use this if the ASSERT above breaks things and we want to move on
                        inputEvent.failed();
                        return;
                }
                */

                // We need to convert Element to a WebInputElement first...
                blink::WebElement webElement(element);
                blink::WebInputElement* inputElement = blink::toWebInputElement(&webElement);
                ASSERT(inputElement);
                /*
                if(!inputElement) { // XXX: WORKAROUND: we can use this if the ASSERT above breaks things and we want to move on
                        inputEvent.failed();
                        return;
                }
                */

                blink::WebString afName(inputElement->nameForAutofill());
                String afNameString(afName.latin1().data(),afName.length());
                ASSERT(afNameString == inputEvent.inputElementName());
                /*
                if(afNameString != inputEvent.inputElementName()) { // XXX: WORKAROUND: we can use this if the ASSERT above breaks things and we want to move on
                        inputEvent.failed();
                        return;
                }
                */

                // Now we can replay setAutofilled...
                inputElement->setAutofilled(inputEvent.isAutofilled());

                inputEvent.injected();
        } else {
        	inputEvent.failed();
        }
}


void ForensicEventInjector::visit(ForensicReceiveCachedMetadataEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicDidFailEvent");
	ASSERT(m_loader);
	ASSERT(m_loaderClient);

	m_loaderClient->didReceiveCachedMetadata(m_loader, inputEvent.data().data(),inputEvent.data().size());
	inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicDidFailEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicDidFailEvent, error %i", inputEvent.error().reason);
	ASSERT(m_loader);
	ASSERT(m_loaderClient);

	m_loaderClient->didFail(m_loader, inputEvent.error());
	inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicReceiveResponseEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicReceiveResponseEvent: mime-type: %s", inputEvent.response().mimeType().latin1().c_str());
	ASSERT(m_loader);
	ASSERT(m_loaderClient);

	m_loaderClient->didReceiveResponse(m_loader, inputEvent.response());
	inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicFinishLoadingEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicFinishLoadingEvent: Finish Time %f, Encoded Data Length, %li", inputEvent.finishTime(), inputEvent.encodedDataLength());
	ASSERT(m_loader);
	ASSERT(m_loaderClient);

	m_loaderClient->didFinishLoading(m_loader, inputEvent.finishTime(), inputEvent.encodedDataLength());
	inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicReceiveDataEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicReceiveDataEvent");
	ASSERT(m_loader);
	ASSERT(m_loaderClient);

	m_loaderClient->didReceiveData(m_loader, inputEvent.data().data(), inputEvent.data().size(), inputEvent.encodedDataLength());
	inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicDownloadDataEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicDownloadDataEvent");
	ASSERT(m_loader);
	ASSERT(m_loaderClient);

	m_loaderClient->didDownloadData(m_loader, inputEvent.dataLength(), inputEvent.encodedDataLength());
	inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicRedirectEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicRedirectEvent");
	ASSERT(m_loader);
	ASSERT(m_loaderClient);

	m_loaderClient->willSendRequest(m_loader, inputEvent.request(), inputEvent.response());
	inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicClearCacheEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicClearCacheEvent");
	ASSERT(m_pageAgent);
	ASSERT(m_client);

	memoryCache()->evictResources();
	for (LocalFrame* frame = m_pageAgent->mainFrame(); frame; frame = frame->tree().traverseNext())
	        frame->document()->fetcher()->garbageCollectDocumentResources();
	m_client->clearBrowserCache();

	inputEvent.injected();
}

//back/forward button events
void ForensicEventInjector::visit(ForensicLoadHistoryItemEvent& inputEvent){

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector::visit(ForensicLoadHistoryItemEvent)");
	ASSERT(m_page);

	m_page->mainFrame()->loader().loadHistoryItem(inputEvent.item(), inputEvent.historyLoadType(), inputEvent.cachePolicy());
    inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicLoadURLEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicLoadURLEvent");
	ASSERT(m_pageAgent);

	ErrorString string;
	m_pageAgent->navigate(&string, inputEvent.url());

	inputEvent.injected();
}

void ForensicEventInjector::visit(ForensicPageScrollEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	doReplaySelfHealing(inputEvent);

	//blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(inputEvent.page());
	ASSERT(m_page);
	blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(m_page);

	if(webView){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicPageScrollEvent. Height: %i Width: %i",  inputEvent.webSize().height, inputEvent.webSize().width);
		webView->applyScrollAndScale(inputEvent.webSize(), inputEvent.pageScaleDelta());
		inputEvent.injected();
	} else {
		inputEvent.failed();
	}
}

void ForensicEventInjector::visit(ForensicPageResizeEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	doReplaySelfHealing(inputEvent);

	//blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(inputEvent.page());
	ASSERT(m_page);
	blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(m_page);

	if(webView){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicPageResizeEvent. Height: %i Width: %i",  inputEvent.webSize().height, inputEvent.webSize().width);

		//TODO resize outside browser window if possible
		//only way found so far is https://developer.chrome.com/extensions/windows#method-update used by chromedriver
		/* Doesn't work, setWindowRect is ignored by the browser
		FloatRect current(inputEvent.page()->chrome().windowRect());
		WebCapsule::log(WebCapsule::Debug,"ForensicEventInjector: current window size Height: %f Width: %f",  current.height(), current.width());
		current.setWidth(inputEvent.size().width);
		current.setHeight(inputEvent.size().height);
		WebCapsule::log(WebCapsule::Debug,"ForensicEventInjector: new window size Height: %f Width: %f",  current.height(), current.width());
		inputEvent.page()->chrome().setWindowRect(current);
		*/

		webView->resize(inputEvent.webSize());
		webView->layout();
		inputEvent.injected();
	} else {
		inputEvent.failed();
	}
}

void ForensicEventInjector::visit(ForensicReplayStopEvent& inputEvent){
	if(inputEvent.state() != ForensicEvent::Ready) return;

	//InspectorForensicsAgent* agent = inputEvent.agent();
	if(m_forensicsAgent){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjector: injecting ForensicReplayStopEvent.");
		m_forensicsAgent->stopReplay();
		inputEvent.injected();
	} else {
		inputEvent.failed();
	}
}



} // namespace WebCore
