/*
 * InspectorForensicsAgent.cpp
 *
 *  Created on: Feb 24, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include <v8.h>
#include "core/inspector/ForensicSerialization.h"
#include "core/inspector/ForensicInputEvent.h"
#include "core/inspector/ForensicClearCacheEvent.h"
#include "core/inspector/ForensicLoadURLEvent.h"
#include "core/inspector/ForensicPageResizeEvent.h"
#include "core/inspector/ForensicPageScrollEvent.h"
#include "core/inspector/ForensicReplayStopEvent.h"
#include "core/inspector/ForensicRecordingStartEvent.h"
#include "core/inspector/ForensicEventInjectorTask.h"
#include "core/inspector/InspectorForensicsAgent.h"
#include "core/inspector/InspectorState.h"
#include "core/inspector/InstrumentingAgents.h"
#include "core/inspector/ForensicPlatformWrapper.h"
#include "core/inspector/ForensicInspectorStateWrapper.h"
#include "core/page/Page.h"
#include "platform/Logging.h"
#include "public/web/WebInputEvent.h"
#include "public/platform/Platform.h"
#include "public/platform/WebString.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefPtr.h"
#include "core/frame/LocalFrame.h"
#include "core/dom/Document.h"
#include "core/dom/Element.h"
#include "core/events/Event.h"
#include "core/loader/DocumentLoader.h"
#include "core/fetch/ResourceFetcher.h"
#include "platform/weborigin/KURL.h"
#include "core/inspector/ForensicReplayDataStore.h"
#include "core/inspector/ForensicOffloadingRecorder.h"
#include "core/inspector/WebCapsule.h"
#include "core/inspector/ForensicRenderMenuListValueChangedEvent.h"
#include "core/inspector/ForensicSetEditingValueEvent.h"
#include "core/inspector/ForensicSetAutofilledEvent.h"
#include "core/inspector/ForensicLoadHistoryItemEvent.h"
#include "core/inspector/WebURLRequestTraits.h"
#include "web/WebViewImpl.h"
#include "bindings/v8/ScriptController.h"
#include "../public/web/WebScriptSource.h"
#include "core/dom/Range.h"
#include "core/rendering/RenderView.h"
#include "core/editing/TextIterator.h"
#include "core/frame/Frame.h"
#include "core/html/HTMLElement.h"
#include "core/html/HTMLHeadElement.h"
#include "core/loader/UniqueIdentifier.h"
#include "core/html/PluginDocument.h"
#include "platform/PlatformKeyboardEvent.h"
#include "platform/PlatformMouseEvent.h"

#define NUMBER_TO_STRING_DECIMAL_DIGITS 6



namespace WebCore {

static ForensicPlatformWrapper* s_platformWrapper = 0;
static unsigned int s_forensicAgentID = 0;
static InspectorFrontend::Forensics* s_frontendForensics = 0;

InspectorForensicsAgent::InspectorForensicsAgent(Page* page, InspectorClient* client, InspectorPageAgent* pageAgent)
    : InspectorBaseAgent<InspectorForensicsAgent>("Forensics")
    , m_page(page)
    , m_frontendForensics(0)
    , m_client(client)
    , m_pageAgent(pageAgent)
    , m_recording(false)
    , m_replaying(false)
    , m_stateId(0)
    , m_doTakeDOMTextSnapshot(false)
	, m_doTakeDOMPicture(false)
	, m_recordingOnStart(false)
	, m_inMemoryRecording(true)
    //, m_urlLoader(0)
{
	TRACE_EVENT0("blink", "InspectorForensicsAgent::InspectorForensicsAgent : Total");

	//can't set m_state in constructor, i guess because InspectorBaseAgent hasn't really been called yet

	// useful to track when new agents are created
	m_agentID = ++s_forensicAgentID;
	WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::InspectorForensicsAgent : created new agent : agentID=%d");
}

InspectorForensicsAgent::~InspectorForensicsAgent()
{
	/*if(m_urlLoader){
		delete m_urlLoader;
	}*/

	WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::InspectorForensicsAgent : instantiating empty agent");
}

PassOwnPtr<InspectorForensicsAgent> InspectorForensicsAgent::create(Page* page, InspectorClient* client, InspectorPageAgent* pageAgent)
{
	TRACE_EVENT0("blink", "InspectorForensicsAgent::create : Total");

	WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent: Creating new agent...");
	return adoptPtr(new InspectorForensicsAgent(page, client, pageAgent));
}

void InspectorForensicsAgent::init()
{
	//m_instrumentingAgents->setInspectorForensicsAgent(this);

	// we make sure that at least all network and platform calls are recorded from the beginning,
	// so that we can collect TRACE_EVENT info without bias;
	// but for now, we need to make sure we don't inject any page event, to avoid some "empty"
	// TRACE_EVENT instrumentation measurements
	if(m_recordingOnStart){
		setRecordingState(true);
		initPlatformWrapper(); // immediately starts using the wrapped platform
	}
}

void InspectorForensicsAgent::setFrontend(InspectorFrontend* frontend)
{
	ASSERT(!m_frontendForensics);
    m_frontendForensics = frontend->forensics();
    s_frontendForensics = m_frontendForensics;
    m_instrumentingAgents->setInspectorForensicsAgent(this);
}

//TODO turn the agent off
void InspectorForensicsAgent::clearFrontend()
{
	ASSERT(m_frontendForensics);
    m_frontendForensics = 0;
    s_frontendForensics = 0;
    m_instrumentingAgents->setInspectorForensicsAgent(0);
}

void InspectorForensicsAgent::restore()
{

	WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent: Restoring agent state.");
	m_recording = m_state->getBoolean(ForensicsAgentState::forensicsAgentRecording);
	m_replaying = m_state->getBoolean(ForensicsAgentState::forensicsAgentReplaying);

	long stateId = m_state->getLong(ForensicsAgentState::forensicsAgentStateID, -1);
	if(stateId != -1){
		setStateId(stateId);
	} else {
		setStateId(0);
	}
}

void InspectorForensicsAgent::startReplay(ErrorString*, const double* in_speed)
{
	//if no data, then nothing to replay
	if(!m_recording && !m_replaying && m_dataStore && m_dataStore->containsData()){

		//Setting the state of the data store for replay
		m_dataStore->reset();
		//Index 0 should always be a ForensicRecordingStartEvent
		ASSERT(s_platformWrapper);
		ForensicRecordingStartEvent* startEvent = static_cast<ForensicRecordingStartEvent*>(m_dataStore->forensicEvents().at(0).get());
		m_dataStore->setCurrentTimeOffset(s_platformWrapper->wrappedPlatform()->currentTime() - startEvent->timestamp());
		m_dataStore->setMonotonicallyIncreasingTimeOffset(
				s_platformWrapper->wrappedPlatform()->monotonicallyIncreasingTime() - startEvent->monotonicallyIncreasingTimestamp());

		//Setting the platform replay state
		setReplayState(true);

		//Injecting web api events
		WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::startReplay : Starting Replay, Total Events %li", (long)m_dataStore->forensicEvents().size()); //cast to long is for android compatibility


        // start replay
        m_dataStore->setReplayStartTime(getWrappedCurrentTime());
        m_dataStore->setReplayStartTimeMonotonic(getWrappedMonotonicallyIncreasingTime());

        // initialize V8 replay
		v8::V8::StartPlatformInstReplaying(m_dataStore->recordStartTime(), m_dataStore->replayStartTime());

		if(in_speed){
			blink::Platform::current()->currentThread()->postTask(
					new ForensicEventInjectorTask(m_dataStore->forensicEvents(),
							s_platformWrapper->wrappedPlatform(),
							ForensicInspectorStateWrapper::create(m_state),
							ForensicEventInjector::create(m_page, m_client, m_pageAgent, this, m_dataStore.get(), m_doTakeDOMTextSnapshot),
                            m_dataStore->recordStartTime(),
                            m_dataStore->replayStartTime(),
							false,
							*in_speed));
		} else {
			blink::Platform::current()->currentThread()->postTask(
			        new ForensicEventInjectorTask(m_dataStore->forensicEvents(),
					        s_platformWrapper->wrappedPlatform(),
				            ForensicInspectorStateWrapper::create(m_state),
				            ForensicEventInjector::create(m_page, m_client, m_pageAgent, this, m_dataStore.get(), m_doTakeDOMTextSnapshot),
                            m_dataStore->recordStartTime(),
                            m_dataStore->replayStartTime()));
		}
	}
}

void InspectorForensicsAgent::stopReplay(ErrorString*)
{
	if(!m_recording && m_replaying){
		WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent: Stopping Replay");
		setReplayState(false);
		v8::V8::StopPlatformInst();
		if(m_frontendForensics){
			m_frontendForensics->replayStopped(getWrappedCurrentTime());
		}
	}
}

bool InspectorForensicsAgent::replaying()
{
	return m_replaying;
}

bool InspectorForensicsAgent::recording()
{
	return m_recording;
}

bool InspectorForensicsAgent::replaying(long stateId)
{
	return checkStateId(stateId) && m_replaying;
}

bool InspectorForensicsAgent::recording(long stateId)
{
	return checkStateId(stateId) && m_recording;
}

long InspectorForensicsAgent::stateId()
{
	return m_stateId;
}

void InspectorForensicsAgent::startReplay(){
	ErrorString error;
	double d = 1.0;
	startReplay(&error, &d);
}

void InspectorForensicsAgent::stopReplay(){
	ErrorString error;
	stopReplay(&error);
}

void InspectorForensicsAgent::startRecording(){
	ErrorString error;
	startRecording(&error);
}

void InspectorForensicsAgent::stopRecording(){
	ErrorString error;
	stopRecording(&error);
}

void InspectorForensicsAgent::startReplay(long stateId){
	if(checkStateId(stateId)){
		startReplay();
	}
}

void InspectorForensicsAgent::stopReplay(long stateId){
	//WebCapsule::log(WebCapsule::Debug,"InspectorForensicsAgent: Stop replay with stateId: %li current state id: %li", stateId, m_stateId);
	if(checkStateId(stateId)){
		stopReplay();
	}
}

void InspectorForensicsAgent::startRecording(long stateId){
	if(checkStateId(stateId)){
		startRecording();
	}
}

void InspectorForensicsAgent::stopRecording(long stateId){
	if(checkStateId(stateId)){
		stopRecording();
	}
}

void InspectorForensicsAgent::setReplayState(bool replayState){
	m_replaying = replayState;
	m_state->setBoolean(ForensicsAgentState::forensicsAgentReplaying, m_replaying);
	updateStateId();
}

void InspectorForensicsAgent::startRecording(ErrorString*)
{
	// if(!m_replaying && !m_recording){
	if(!m_replaying) {
		// useful for tracking the agent
		// NOTE: we could not put this in the constructor, because in some cases at those early stages the Document seems not to exist!
		m_firstPageURL = String(""); // to make sure that latin1().data() does not return null (it may be unnecessary...)
		if(m_page->mainFrame()) {
			//m_firstPageURL = String("frame"); // only for debugging purposes
			//WebCapsule::log(WebCapsule::Debug,"InspectorForensicsAgent::startRecording : agentID=%d | firstPageURL=%s", m_agentID, m_firstPageURL.latin1().data());
			if(m_page->mainFrame()->document()) {
				//m_firstPageURL = String("doc"); // only for debugging purposes
				//WebCapsule::log(WebCapsule::Debug,"InspectorForensicsAgent::startRecording : agentID=%d | firstPageURL=%s", m_agentID, m_firstPageURL.latin1().data());
				if(!m_page->mainFrame()->document()->url().isEmpty()) {
					// m_firstPageURL = String("url"); // only for debugging purposes
					// WebCapsule::log(WebCapsule::Debug,"InspectorForensicsAgent::startRecording : agentID=%d | firstPageURL=%s", m_agentID, m_firstPageURL.latin1().data());
					m_firstPageURL = m_page->mainFrame()->document()->url().string();
					WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::startRecording : agentID=%d | firstPageURL=%s", m_agentID, m_firstPageURL.latin1().data());
				}
			}
		}


		// FIXME : these parameters should be setting according to a configuration file, and passed to the agent by devtools_client.py
		m_doTakeDOMTextSnapshot = true;
		m_doTakeDOMPicture = false;
		m_domPicBaseDir = String("/tmp/skpics/");
		/****************************************/


		RefPtr<ForensicEventInjector> injector = ForensicEventInjector::create(m_page, m_client, m_pageAgent, this, 0, m_doTakeDOMTextSnapshot);
		setRecordingState(true);
		v8::V8::StartPlatformInstRecording();

		initPlatformWrapper();

		ASSERT(m_dataRecorder);
		m_dataRecorder->clear();

		ASSERT(s_platformWrapper);
		PassRefPtr<ForensicRecordingStartEvent> recStart = ForensicRecordingStartEvent::create(getWrappedCurrentTime(), s_platformWrapper->wrappedPlatform()->monotonicallyIncreasingTime());
		PassRefPtr<ForensicClearCacheEvent> clearCache = ForensicClearCacheEvent::create(getWrappedCurrentTime());
		PassRefPtr<ForensicLoadURLEvent> aboutBlank = ForensicLoadURLEvent::create(String("about:blank"), getWrappedCurrentTime());
		PassRefPtr<ForensicLoadURLEvent> loadURL = ForensicLoadURLEvent::create(getCurrentURL().string(), getWrappedCurrentTime());

		blink::WebViewImpl* webView = blink::WebViewImpl::fromPage(m_page);
		PassRefPtr<ForensicPageResizeEvent> pageSize = ForensicPageResizeEvent::create(webView->size(), getCurrentURL().string(), PageURLRequestMapLocation(), getWrappedCurrentTime());

		WebCapsule::log(WebCapsule::DebugLogLevel,"Current Page URL: %s", loadURL->url().latin1().data());

        //start recording
		m_dataRecorder->setRecordStartTime(getWrappedCurrentTime());
		m_dataRecorder->setRecordStartTimeMonotonic(getWrappedMonotonicallyIncreasingTime());

		//clear the cache, reload the current page
		injector->visit(*clearCache);
        injector->visit(*aboutBlank);
		injector->visit(*loadURL);

		//store these actions to be replayed
		m_dataRecorder->appendForensicEvent(recStart);
		m_dataRecorder->appendForensicEvent(clearCache);
		m_dataRecorder->appendForensicEvent(aboutBlank);
		m_dataRecorder->appendForensicEvent(pageSize);
		m_dataRecorder->appendForensicEvent(loadURL);
	}
}

void InspectorForensicsAgent::stopRecording(ErrorString*)
{
	if(!m_replaying && m_recording){
		setRecordingState(false);
		v8::V8::StopPlatformInst();

		//finish up recording tasks
		m_dataRecorder->stopRecording();

		//After this timestamp we have recorded no more data that can be replayed
		m_dataRecorder->appendForensicEvent(ForensicReplayStopEvent::create(s_platformWrapper->wrappedPlatform()->currentTime()));
	}
}

void InspectorForensicsAgent::dumpRecording(ErrorString*, RefPtr<JSONObject>& out_recording){
	RefPtr<JSONObject> data = JSONObject::create();
	if(!m_replaying && !m_recording && m_inMemoryRecording && m_dataStore){
		data->setObject("blink", m_dataStore->serialize());
		v8::PlatformInstrumentationData v8Data = v8::V8::DumpPlatformInstRecording();
		data->setObject("v8", SerializationUtils::serialize(v8Data));
		WebCapsule::log(WebCapsule::DebugLogLevel,"Dumped Recording");
	}
	out_recording.swap(data);
}

void InspectorForensicsAgent::dumpReplayInfo(ErrorString*, RefPtr<JSONObject>& out_rplay_info){
	RefPtr<JSONObject> data = JSONObject::create();
	if(!m_replaying && !m_recording && m_inMemoryRecording && m_dataStore){
		data->setObject("blink", m_dataStore->serializeReplayInfo());
		WebCapsule::log(WebCapsule::DebugLogLevel,"Dumped Replay Info");
	}
	out_rplay_info.swap(data);
}

void InspectorForensicsAgent::loadRecording(ErrorString*, const RefPtr<JSONObject>& in_recording){
	if(!m_dataStore){
		initPlatformWrapper();
	}
	ASSERT(m_dataStore);

	if(m_recording) { // this is useful because we could start recording at startup
		stopRecording();
		m_dataRecorder->clear();
	}

	if(!m_replaying && !m_recording){
		RefPtr<JSONObject> blinkData = in_recording->getObject("blink");
		ASSERT(blinkData); //there should always be some data to deserialize within the blink domain
		m_dataStore->deserialize(blinkData);

		RefPtr<JSONObject> v8Data = in_recording->getObject("v8");
		if (v8Data){ //its possible thata we did not record any data withing the v8 domain
			v8::PlatformInstrumentationData data;
			SerializationUtils::deserialize(v8Data, data);
			v8::V8::LoadPlatformInstRecording(data);
		}

		//ErrorString string;
		//m_pageAgent->navigate(&string, "about:blank"); //makes sure that the tab has been fully initialized.

		WebCapsule::log(WebCapsule::DebugLogLevel, "Loaded Recording");
	}
}

void InspectorForensicsAgent::setRecordingState(bool recordingState){
	m_recording = recordingState;
	m_state->setBoolean(ForensicsAgentState::forensicsAgentRecording, m_recording);
	updateStateId();
}

void InspectorForensicsAgent::setStateId(long stateId){
	m_stateId = stateId;
	m_state->setLong(ForensicsAgentState::forensicsAgentStateID, m_stateId);
}

void InspectorForensicsAgent::updateStateId(){
	setStateId(m_stateId + 1);
}

bool InspectorForensicsAgent::checkStateId(long stateId)
{
	return m_stateId == stateId;
}

RefPtr<TypeBuilder::Forensics::Request> InspectorForensicsAgent::createRequest(blink::WebURLRequest& requestIn){
	return TypeBuilder::Forensics::Request::create()
		.setHash(WebURLRequestHash::hash(requestIn))
		.setUrl(String::fromUTF8(requestIn.url().string().utf8().c_str()));
}


void InspectorForensicsAgent::handleInputEvent(const blink::WebInputEvent& inputEvent)
{
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleInputEvent");

	//Print the input event to the log
	WebCapsule::log(WebCapsule::VeryVerboseLogLevel,"InspectorForensicsAgent::handleInputEvent : input_event_type: %s, time: %f", typeString(inputEvent), inputEvent.timeStampSeconds);

	if(m_recording) {
		//Sends the captured event to the front end
		m_frontendForensics->inputEventFired(typeString(inputEvent), inputEvent.timeStampSeconds);
        WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleInputEvent : Recording Input Event : type=%s", typeString(inputEvent));

		//Records the captured event
		ASSERT(s_platformWrapper);
        WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleInputEvent : currentURL=%s", getCurrentURL().string().utf8().data());
        m_dataRecorder->appendForensicEvent(ForensicInputEvent::create(inputEvent, getCurrentURL().string(), createMapLocation(), getWrappedCurrentTime()));
	}
}

/*
void InspectorForensicsAgent::handleEventTargetElement(LocalFrame *frame, const blink::WebInputEvent& inputEvent) {

	Element* target = eventTargetElement(frame->document()); // frame needs to point to the current frame!
    WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleInputEvent : Event Target : %s", target->outerHTML().utf8().data());

}
*/

// information only (do not replay!)
void InspectorForensicsAgent::handleEventHandlerKeyEvent(const PlatformKeyboardEvent& keyEvent, const Node* targetNode) {
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerKeyEvent");

	if(!targetNode)
		return;
	if(!targetNode->isHTMLElement())
		return;
	if(!keyEvent.type() == PlatformEvent::Char)
		return;

	String id;
	id.append("T:");
	id.append(String::numberToStringFixedWidth(keyEvent.timestamp(),NUMBER_TO_STRING_DECIMAL_DIGITS));
	id.append("-I:");
	id.append(keyEvent.keyIdentifier());
	id.append("-K:");
	id.append(keyEvent.text());

    const Element* targetElement = toElement(targetNode);
    const Element* parentElement = targetElement->parentElement();

    WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerKeyEvent : ID=%s, targetElement=%s, parentElement=%s ", id.utf8().data(), targetElement->outerHTML().utf8().data(), parentElement->outerHTML().utf8().data());


    String domTextSnapshot;
    if(keyEvent.type() == PlatformEvent::Char && keyEvent.keyIdentifier()=="Enter") {

        if(m_doTakeDOMTextSnapshot) {
        	TRACE_EVENT_BEGIN0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerKeyEvent : TakeDOMTextSnapshot");
        	domTextSnapshot.append("\n<!--- WEBCAPSULE : DOM_SNAPTHOT : BEGIN : STATE=RECORDING --->\n");
        	takeDOMTextSnapshot(m_page, domTextSnapshot, keyEvent.timestamp());
        	domTextSnapshot.append("\n<!--- WEBCAPSULE : DOM_SNAPTHOT : END : STATE=RECORDING --->\n");
            WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerKeyEvent : taken domTextSnapshot (ID=%s)", String::numberToStringFixedWidth(keyEvent.timestamp(),NUMBER_TO_STRING_DECIMAL_DIGITS).utf8().data());
            TRACE_EVENT_END0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerKeyEvent : TakeDOMTextSnapshot");
        }

        if(m_doTakeDOMPicture) {
        	TRACE_EVENT_BEGIN0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerKeyEvent : TakeDOMPicture");
        	takeDOMPicture(m_page, m_domPicBaseDir, keyEvent.timestamp());
            WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerKeyEvent : taken domPicture (ID=%s)", String::numberToStringFixedWidth(keyEvent.timestamp(),NUMBER_TO_STRING_DECIMAL_DIGITS).utf8().data());
            TRACE_EVENT_END0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerKeyEvent : TakeDOMPicture");
        }
    }


    KeyPressEventTargetInfo eventInfo(keyEvent.timestamp(), keyEvent.keyIdentifier(), keyEvent.text(), targetElement->outerHTML(), parentElement->outerHTML(), domTextSnapshot);

    if(m_recording) {
    	m_dataRecorder->setKeyPressEventTargetRecordInfo(id, eventInfo);
    }
    else if(m_replaying) {
    	m_dataStore->setKeyPressEventTargetReplayInfo(id, eventInfo);

    	KeyPressEventTargetInfo recEventInfo;
    	if(m_dataStore->containsKeyPressEventTargetRecordInfo(id))
    		recEventInfo = m_dataStore->getKeyPressEventTargetRecordInfo(id);

    	if(recEventInfo.m_targetElementHTML == eventInfo.m_targetElementHTML) {
    	    WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerKeyEvent : Record and Replay event targets matched perfectly! ");
    	}
    	else {
    	    WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerKeyEvent : Record and Replay event targets differ! \nRECORDING = %s\nREPLAYING = %s", recEventInfo.m_targetElementHTML.utf8().data(), eventInfo.m_targetElementHTML.utf8().data());
    	}
    }

}


// information only (do not replay!)
void InspectorForensicsAgent::handleEventHandlerMousePressEvent(const PlatformMouseEvent& mouseEvent, const Node* targetNode) {
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerMousePressEvent");

	if(!targetNode)
		return;
	if(!targetNode->isHTMLElement())
		return;

	const int x = mouseEvent.position().x();
	const int y = mouseEvent.position().y();

	String id;
	id.append("T:");
	id.append(String::numberToStringFixedWidth(mouseEvent.timestamp(),NUMBER_TO_STRING_DECIMAL_DIGITS));
	id.append("-X:");
	id.append(String::number(x));
	id.append("-Y:");
	id.append(String::number(y));

    const Element* targetElement = toElement(targetNode);
    const Element* parentElement = targetElement->parentElement();

    WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerMousePressEvent : ID=%s, targetElement=%s, parentElement=%s ", id.utf8().data(), targetElement->outerHTML().utf8().data(), parentElement->outerHTML().utf8().data());


    String domTextSnapshot;
    if(mouseEvent.type() == PlatformEvent::MousePressed) {

        if(m_doTakeDOMTextSnapshot) {
        	TRACE_EVENT_BEGIN0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerMousePressEvent : TakeDOMTextSnapshot");
        	domTextSnapshot.append("\n<!--- WEBCAPSULE : DOM_SNAPTHOT : BEGIN : STATE=RECORDING --->\n");
        	takeDOMTextSnapshot(m_page, domTextSnapshot, mouseEvent.timestamp());
        	domTextSnapshot.append("\n<!--- WEBCAPSULE : DOM_SNAPTHOT : END : STATE=RECORDING --->\n");
            WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerMousePressEvent : taken domTextSnapshot (ID=%s)", String::numberToStringFixedWidth(mouseEvent.timestamp(),NUMBER_TO_STRING_DECIMAL_DIGITS).utf8().data());
            TRACE_EVENT_END0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerMousePressEvent : TakeDOMTextSnapshot");
        }

        if(m_doTakeDOMPicture) {
        	TRACE_EVENT_BEGIN0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerMousePressEvent : TakeDOMPicture");
        	takeDOMPicture(m_page, m_domPicBaseDir, mouseEvent.timestamp());
            WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerMousePressEvent : taken domPicture (ID=%s)", String::numberToStringFixedWidth(mouseEvent.timestamp(),NUMBER_TO_STRING_DECIMAL_DIGITS).utf8().data());
            TRACE_EVENT_END0("blink", "WebCapsule::InspectorForensicsAgent::handleEventHandlerMousePressEvent : TakeDOMPicture");
        }
    }


    MousePressEventTargetInfo eventInfo(mouseEvent.timestamp(), x, y, targetElement->outerHTML(), parentElement->outerHTML(), domTextSnapshot);

    if(m_recording) {
    	m_dataRecorder->setMousePressEventTargetRecordInfo(id, eventInfo);
    }
    else if(m_replaying) {
    	m_dataStore->setMousePressEventTargetReplayInfo(id, eventInfo);

    	MousePressEventTargetInfo recEventInfo;
    	if(m_dataStore->containsMousePressEventTargetRecordInfo(id))
    		recEventInfo = m_dataStore->getMousePressEventTargetRecordInfo(id);

    	if(recEventInfo.m_targetElementHTML == eventInfo.m_targetElementHTML) {
    	    WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerMousePressEvent : Record and Replay event targets matched perfectly! ");
    	}
    	else {
    	    WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleEventHandlerMousePressEvent : Record and Replay event targets differ! \nRECORDING = %s\nREPLAYING = %s", recEventInfo.m_targetElementHTML.utf8().data(), eventInfo.m_targetElementHTML.utf8().data());
    	}
    }
}

void InspectorForensicsAgent::setDoTakeDOMTextSnapshot(bool v) {
	m_doTakeDOMTextSnapshot = v;
}

void InspectorForensicsAgent::setDoTakeDOMPicture(bool v) {
	m_doTakeDOMPicture = v;
}

void InspectorForensicsAgent::setDOMPictureBaseLogDir(const String& dir) {
	m_domPicBaseDir = dir;
}

void InspectorForensicsAgent::takeDOMTextSnapshot(Page* page, String& domSnapshot, double timestamp) {
	domSnapshot.append("\n<!--- WEBCAPSULE : DOM_SNAPTHOT : BEGIN : ID=");
	domSnapshot.append(String::numberToStringFixedWidth(timestamp,NUMBER_TO_STRING_DECIMAL_DIGITS));
	domSnapshot.append(" --->\n");

    frameContentAsPlainText(page->mainFrame(), domSnapshot);

    domSnapshot.append("\n<!--- WEBCAPSULE : DOM_SNAPTHOT : END : ID=");
	domSnapshot.append(String::numberToStringFixedWidth(timestamp,NUMBER_TO_STRING_DECIMAL_DIGITS));
	domSnapshot.append(" --->\n");
}

void InspectorForensicsAgent::takeDOMPicture(Page* page, const String& baseDir, double timestamp) {
	String dumpDir;
	dumpDir.append(baseDir);
	dumpDir.append(String::numberToStringFixedWidth(timestamp,NUMBER_TO_STRING_DECIMAL_DIGITS));
    takeSkPicture(page, dumpDir);
}


void InspectorForensicsAgent::handleRenderMenuListValueChanged(unsigned int listIndex, bool fireOnChange, const Node* targetNode)
{
		TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleRenderMenuListValueChanged");

	    const Element* targetElement = toElement(targetNode);
	    const Element* parentElement = targetElement->parentElement();

        //Print the event to the log
        WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleRenderMenuListValueChanged (listIndex=%d, fireOnChange=%d, time=%f)", listIndex, fireOnChange, getWrappedCurrentTime());
        if(m_recording){
                //Records the captured event
                ASSERT(s_platformWrapper);
                m_dataRecorder->appendForensicEvent(ForensicRenderMenuListValueChangedEvent::create(listIndex, fireOnChange, targetElement->outerHTML(), parentElement->outerHTML(), getCurrentURL().string(), createMapLocation(), getWrappedCurrentTime()));
        }
}


// inputElementName: is the name of the element on the page, and can be derived from WebFormControlElement::nameForAutofill()
// value: the value to be set
// timestampSeconds: the time at which WebInputElement::setEditingValue was called
void InspectorForensicsAgent::handleSetEditingValueEvent(const String& inputElementName, const String& value)
{
		TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleSetEditingValueEvent");

        //Print the event to the log
        WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleSetEditingValueEvent (inputElementName=%s, value=%s, time=%f)", inputElementName.latin1().data(), value.latin1().data(), getWrappedCurrentTime());
        if(m_recording){
                //Records the captured event
                ASSERT(s_platformWrapper);
                m_dataRecorder->appendForensicEvent(ForensicSetEditingValueEvent::create(inputElementName, value, getCurrentURL().string(), createMapLocation(), getWrappedCurrentTime()));
        }
}


void InspectorForensicsAgent::handleSetAutofilledEvent(const String& inputElementName, bool autofilled)
{
		TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleSetAutofilledEvent");

        //Print the event to the log
        WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleSetAutofilledEvent (autofilled=%d, time=%f)", autofilled);

        if(m_recording){
                //Records the captured event
                ASSERT(s_platformWrapper);
                m_dataRecorder->appendForensicEvent(ForensicSetAutofilledEvent::create(/*m_page,*/ inputElementName, autofilled, getCurrentURL().string(), createMapLocation(), getWrappedCurrentTime()));

        }
}

//modaldialog by Bo
bool InspectorForensicsAgent::handleModaldialogConfirmResultReplay(const String& message, bool& var)
{
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleModaldialogConfirmResultReplay");
	if(m_replaying)
	{
				var = m_dataStore->nextModaldialogConfirmResult(message);
				WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleModaldialogConfirmResultReplay (message=%s, var=%d)", message.utf8().data(), var);
				return true;
	}
	return false;
}
void InspectorForensicsAgent::handleModaldialogConfirmResultRecording(const String& message, bool result)
{
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleModaldialogConfirmResultRecording");
	if(m_recording){
		m_dataRecorder->appendModaldialogConfirmResult(message, result);
	}
}



bool InspectorForensicsAgent::handleModaldialogPromptResultReplay(const String& message, String& var)
{
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleModaldialogPromptResultReplay");
	if(m_replaying)
	{
		var = m_dataStore->nextModaldialogPromptResult(message);
		WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleModaldialogPromptResultReplay (message=%s, var=%s)", message.utf8().data(), var.utf8().data());
		return true;
	}
	return false;
}
void InspectorForensicsAgent::handleModaldialogPromptResultRecording(const String& message, const String& result)
{
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleModaldialogPromptResultRecording");
	if(m_recording){
		WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleModaldialogPromptResultRecording (message=%s, result=%s)", message.utf8().data(), result.utf8().data());
		m_dataRecorder->appendModaldialogPromptResult(message, result);
	}
}

bool InspectorForensicsAgent::handleModaldialogAlertReplay()
{
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleModaldialogAlertValsReplay");
	if(m_replaying)
	{
		return true;
	}
	return false;
}

void InspectorForensicsAgent::handleModaldialogAlertRecording(const String& message)
{
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleModaldialogAlertRecording");
	if(m_recording){
		WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleModaldialogAlertRecording (message=%s)", message.utf8().data());
		m_dataRecorder->appendModaldialogAlertVals(message);
	}
}

//TODO handle special protocols like swappedout://
//FIXME we should re-enable the recording of URL load events, so that we can synchronize to such events during replay
void InspectorForensicsAgent::handleURLLoad(LocalFrame* frame, const blink::WebURLRequest& request) {
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleURLLoad");

	if(m_recording && m_page->mainFrame() == frame){
		//WebCapsule::log(WebCapsule::Debug,"InspectorForensicsAgent: Handling url load: %s", request.url().spec().data());
		//m_events.append(new ForensicLoadURLEvent(String::format("%s", request.url().spec().data()), m_pageAgent, blink::Platform::current()->currentTime()));

	}
}

// This is used to enable a better self-healing
// It handles a "strange" HTML5 feature that allows for replacing the page URL without actually (re)loading the frame
// See for example this: http://spoiledmilk.com/blog/html5-changing-the-browser-url-without-refreshing-page/
void InspectorForensicsAgent::handleUpdateForSameDocumentNavigation(LocalFrame* frame, const KURL& oldURL, const KURL& newURL) {
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleUpdateForSameDocumentNavigation");

	if(m_recording && frame->isMainFrame()){

		String oldURLStr(""); oldURLStr.append(oldURL.string()); // just for convenience, to make sure URL strings are not null when we write the log (it may very well be unnecessary)
		String newURLStr(""); newURLStr.append(newURL.string());
		WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleUpdateForSameDocumentNavigation oldURL=%s | newURL=%s", oldURLStr.latin1().data(), newURLStr.latin1().data());

		if(newURLStr.length() > 0 && newURL!=oldURL) {
			m_dataRecorder->setURLAlias(newURL, oldURL);
		}
	}
}

// This is used to record the user pressing the back/forward button on the browser toolbar
void InspectorForensicsAgent::handleLoadHistoryItem(LocalFrame* frame, HistoryItem* item, HistoryLoadType historyLoadType, ResourceRequestCachePolicy cachePolicy) {
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleLoadHistoryItem");

	if(!frame || !item) return;

	if(item->url().string() == String("about:blank")) // we should make sure that we store only items with valid URLs
		return;

	/*
	 * TODO we can't store a frame pointer inside of the event as it is not serializable
	 * We need a way to identify the frame in the event without storing a pointer
	 */

	if(frame != m_page->mainFrame())
		return;

	//Print the event to the log
	WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleLoadHistoryItem : history_url=%s", item->url().string().latin1().data());

	if(m_recording){
		//Records the captured event
		ASSERT(s_platformWrapper);
		m_dataRecorder->appendForensicEvent(ForensicLoadHistoryItemEvent::create(/*frame,*/ item, historyLoadType, cachePolicy, s_platformWrapper->wrappedPlatform()->currentTime()));
	    WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleLoadHistoryItem : recorded history item");
	} else if(m_replaying) {
        WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleLoadHistoryItem : replaying history item");
    }
}

// used to enable self-healing
// it also provides very useful information about the events that cause page transitions
void InspectorForensicsAgent::handleFrameLoad(LocalFrame* frame, const blink::WebURLRequest& request, const Document* originDocument, const Event* event){
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleFrameLoad");

	if(!frame->isMainFrame()) return; // FIXME: currently we only support self-healing for main frame replay incongruence; in the future we might need to generalize to all frames

	if(m_recording || m_replaying){ // currently we only record main frame (i.e., page-level) URL changes to assist self-healing during replay

		String originUrlStr("");
		if(originDocument) // the good thing is that originDocument contains the "ideal" referrer, even if the referrer url is not carried in the sbusequent HTTP request
				originUrlStr = originDocument->url().string();

		// computes properties of the event that triggered the load
		String eventTypeStr("");
		if(event && !event->type().isNull())
				eventTypeStr = event->type().string();

		Node* node = NULL;
		if(event && event->target() && event->target()->toNode())
				node = event->target()->toNode();

		String nodeNameStr(""); // this is probably not interesting, given that we are able to log nodeHTML
		if(node && node->isHTMLElement() && !node->nodeName().isNull())
				nodeNameStr = node->nodeName();

		String parentHTML("");
		if(node && node->isHTMLElement() && node->parentElement())
				parentHTML = node->parentElement()->outerHTML();

		String nodeHTML("");
		if(node && node->isHTMLElement())
				nodeHTML = toElement(node)->outerHTML();

		WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleFrameLoad : frame_name=%s | main_frame=%d | event_type=%s | node_name=%s | nodeHTML=%s | parentHTML=%s | url=%s | origin=%s", frame->tree().name().latin1().data(), frame->isMainFrame(), eventTypeStr.latin1().data(), nodeNameStr.latin1().data(), nodeHTML.latin1().data(), parentHTML.latin1().data(), request.url().string().latin1().data(), originUrlStr.latin1().data());

		String requestURLStr(request.url().string().latin1().data());

		if(m_recording) {
			if(!requestURLStr.isEmpty()){
				m_dataRecorder->addToPageURLRequestsMap(requestURLStr, PageURLRequest::create(request, frame->tree().name().string(), frame->isMainFrame(), originUrlStr, eventTypeStr, nodeNameStr, nodeHTML, parentHTML));
			}
			WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleFrameLoad : stored PageURLRequest (url=%s, v_size=%d)", request.url().string().latin1().data(), m_dataStore->sizePageURLRequests(requestURLStr));
		}
		else if(m_replaying) {
			WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent::handleFrameLoad : marking PageURLRequest as consumed (url=%s)", request.url().string().latin1().data());
			m_dataStore->markPageURLRequestAsConsumed(requestURLStr);
		}
	}
}

void InspectorForensicsAgent::handlePageScroll(const blink::WebSize& size, double pageScaleDelta){
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handlePageScroll");

	if(m_recording){
		ASSERT(s_platformWrapper);
		m_dataRecorder->appendForensicEvent(ForensicPageScrollEvent::create(size, pageScaleDelta, getCurrentURL().string(), createMapLocation(), getWrappedCurrentTime()));
	}
}

void InspectorForensicsAgent::handleResize(const blink::WebSize& size){
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::handleResize");

	if(m_recording){
		WebCapsule::log(WebCapsule::DebugLogLevel,"InspectorForensicsAgent: Handling resize: Width: %i Height: %i", size.width, size.height);
		ASSERT(s_platformWrapper);
		m_dataRecorder->appendForensicEvent(ForensicPageResizeEvent::create(size, getCurrentURL().string(), createMapLocation(), getWrappedCurrentTime()));
	}
}

void InspectorForensicsAgent::willSendRequest(unsigned long identifier, DocumentLoader* loader, ResourceRequest& request, const ResourceResponse& redirectResponse, const FetchInitiatorInfo& info){
	TRACE_EVENT0("blink", "WebCapsule::InspectorForensicsAgent::willSendRequest");
	if(redirectResponse.isNull()){
		if(m_recording || m_replaying){
			unsigned long id = createUniqueIdentifier();
			request.setHTTPHeaderField(ForensicReplayDataStore::WEBCAPSULE_HEADER, AtomicString::number(id));
			if(m_recording){
				ASSERT(m_dataRecorder);
				m_dataRecorder->setStackTraceForRequest(String::number(id), getCurrentStackTrace());
			} else {
				ASSERT(m_dataStore);
				m_dataStore->setStackTraceForRequest(String::number(id), getCurrentStackTrace());
			}

		}
	}
}

V8StackTrace InspectorForensicsAgent::getCurrentStackTrace(){
	V8StackTrace retval;

	v8::Isolate* isolate = v8::Isolate::GetCurrent();
	if (isolate->InContext()){
		v8::HandleScope handleScope(isolate);
		v8::Handle<v8::StackTrace> stackTrace(v8::StackTrace::CurrentStackTrace(isolate, 200));

		int frameCount = stackTrace->GetFrameCount();
		for(int i = 0; i < frameCount; ++i){
			retval.append(V8StackFrame(v8::Handle<v8::StackFrame>(stackTrace->GetFrame(i))));
		}
	}

	return retval;
}

void InspectorForensicsAgent::initPlatformWrapper(){
	if(!s_platformWrapper){
		m_dataStore = ForensicReplayDataStore::create();
		if(m_inMemoryRecording){
			m_dataRecorder = m_dataStore;
		} else {
			m_dataRecorder = ForensicOffloadingRecorder::create(blink::Platform::current());
		}
		s_platformWrapper = new ForensicPlatformWrapper(blink::Platform::current(), ForensicInspectorStateWrapper::create(m_state), m_dataStore, m_dataRecorder);
		blink::Platform::initialize(s_platformWrapper);
	} else {
		m_dataStore = s_platformWrapper->dataStore();
		m_dataRecorder = s_platformWrapper->dataRecorder();
	}
}

Page* InspectorForensicsAgent::getCurrentPage() {
	return m_page;
}

const double InspectorForensicsAgent::getWrappedCurrentTime() {
	ASSERT(s_platformWrapper);
	return s_platformWrapper->wrappedPlatform()->currentTime();
}

const double InspectorForensicsAgent::getWrappedMonotonicallyIncreasingTime() {
	ASSERT(s_platformWrapper);
	return s_platformWrapper->wrappedPlatform()->monotonicallyIncreasingTime();
}

const KURL& InspectorForensicsAgent::getCurrentURL(){
	ASSERT(m_page);
	return m_page->mainFrame()->document()->url();
}

ForensicPlatformWrapper* InspectorForensicsAgent::platformWrapper(){
	return s_platformWrapper;
}

InspectorFrontend::Forensics* InspectorForensicsAgent::forensicsFrontend(){
	return s_frontendForensics;
}

PageURLRequestMapLocation InspectorForensicsAgent::createMapLocation(){
	PageURLRequestMapLocation retval;

    // URL alias tracking is used to support redirects and HTML5 silent URL changes
    // (see for example this: http://spoiledmilk.com/blog/html5-changing-the-browser-url-without-refreshing-page/)
	retval.currentUrlRootAlias = m_dataRecorder->findPageURLAliasChainRoot(getCurrentURL().string());

	// notice that we store the request related to the root URL alias with the inputEvent
	// notice also that .last() on PageURLRequestsMap's Vector assumes that we are always dealing with one page (or one thread) at a time per each InspectorForensicAgent
	retval.pos = m_dataRecorder->sizePageURLRequests(retval.currentUrlRootAlias) - 1;
	return retval;
}

//Converts type enum to string for printing
const char * InspectorForensicsAgent::typeString(const blink::WebInputEvent& inputEvent){
	switch (inputEvent.type) {
    case blink::WebInputEvent::MouseMove:
    	return "MouseMove";
    case blink::WebInputEvent::MouseLeave:
    	return "MouseLeave";
    case blink::WebInputEvent::MouseDown:
    	return "MouseDown";
    case blink::WebInputEvent::MouseUp:
    	return "MouseUp";
    case blink::WebInputEvent::MouseWheel:
    	return "MouseWheel";
    case blink::WebInputEvent::RawKeyDown:
    	return "RawKeyDown";
    case blink::WebInputEvent::KeyDown:
    	return "KeyDown";
    case blink::WebInputEvent::KeyUp:
    	return "KeyUp";
    case blink::WebInputEvent::Char:
    	return "Char";
    case blink::WebInputEvent::GestureScrollBegin:
    	return "GestureScrollBegin";
    case blink::WebInputEvent::GestureScrollEnd:
    	return "GestureScrollEnd";
    case blink::WebInputEvent::GestureScrollUpdate:
    	return "GestureScrollUpdate";
    case blink::WebInputEvent::GestureScrollUpdateWithoutPropagation:
    	return "GestureScrollUpdateWithoutPropagation";
    case blink::WebInputEvent::GestureFlingStart:
    	return "GestureFlingStart";
    case blink::WebInputEvent::GestureFlingCancel:
    	return "GestureFlingCancel";
    case blink::WebInputEvent::GestureTap:
    	return "GestureTap";
    case blink::WebInputEvent::GestureTapUnconfirmed:
    	return "GestureTapUnconfirmed";
    case blink::WebInputEvent::GestureTapDown:
    	return "GestureTapDown";
    case blink::WebInputEvent::GestureShowPress:
    	return "GestureShowPress";
    case blink::WebInputEvent::GestureTapCancel:
    	return "GestureTapCancel";
    case blink::WebInputEvent::GestureDoubleTap:
    	return "GestureDoubleTap";
    case blink::WebInputEvent::GestureTwoFingerTap:
    	return "GestureTwoFingerTap";
    case blink::WebInputEvent::GestureLongPress:
    	return "GestureLongPress";
    case blink::WebInputEvent::GestureLongTap:
    	return "GestureLongTap";
    case blink::WebInputEvent::TouchStart:
    	return "TouchStart";
    case blink::WebInputEvent::TouchMove:
    	return "TouchMove";
    case blink::WebInputEvent::TouchEnd:
    	return "TouchEnd";
    case blink::WebInputEvent::TouchCancel:
    	return "TouchCancel";
    case blink::WebInputEvent::GesturePinchBegin:
    	return "GesturePinchBegin";
    case blink::WebInputEvent::GesturePinchEnd:
    	return "GesturePinchEnd";
    case blink::WebInputEvent::GesturePinchUpdate:
    	return "GesturePinchUpdate";
    default:
        return "Unknown";
	}
}


// static
void InspectorForensicsAgent::takeSkPicture(Page* page, String& dumpDir) {

    String jsCall("chrome.gpuBenchmarking.printToSkPicture");
    jsCall.append("(\"");
    jsCall.append(dumpDir);
    jsCall.append("\");");


    // takes a picture of the page
    blink::WebScriptSource source(blink::WebString::fromUTF8(jsCall.latin1().data()));
    // from web/WebFrameImpl.cpp executeScript
    TextPosition position(OrdinalNumber::fromOneBasedInt(source.startLine), OrdinalNumber::first());
    page->mainFrame()->script().executeScriptInMainWorld(ScriptSourceCode(source.code, source.url, position));
    // m_page->mainFrame()->view()->layout();
    // runPendingTasks();
}


// code borrowed from web/WebFrameImpl.cpp
// we made it a member of InspectorForensicsAgent so that we can make changes, if needed...
// static
void InspectorForensicsAgent::frameContentAsPlainText(LocalFrame* frame, String& output)
{
    Document* document = frame->document();
    if (!document)
        return;

    if (!frame->view())
        return;

    // TextIterator iterates over the visual representation of the DOM. As such,
    // it requires you to do a layout before using it (otherwise it'll crash).
    document->updateLayout();

    output.append("\n<!--- WEBCAPSULE : FRAME URL = ");
    output.append(document->url().string());
    output.append(" --->\n");

    output.append(document->head()->outerHTML());
    output.append(document->body()->outerHTML());

    // Recursively walk the children.
    const FrameTree& frameTree = frame->tree();
    for (LocalFrame* curChild = frameTree.firstChild(); curChild; curChild = curChild->tree().nextSibling()) {

        String visibleStr;
        RenderView* contentRenderer = curChild->contentRenderer();
        RenderPart* ownerRenderer = curChild->ownerRenderer();
        if (!contentRenderer || !contentRenderer->width() || !contentRenderer->height()
            || (contentRenderer->x() + contentRenderer->width() <= 0) || (contentRenderer->y() + contentRenderer->height() <= 0)
            || (ownerRenderer && ownerRenderer->style() && ownerRenderer->style()->visibility() != VISIBLE)) {
            visibleStr = "\n<!--- WEBCAPSULE : FRAME VISIBILITY = INVISIBLE --->\n";
            // continue; // Ignore the text of non-visible frames.
        }
        else {
            visibleStr = "\n<!--- WEBCAPSULE : FRAME VISIBILITY = VISIBLE --->\n";
        }

        output.append("\n<!--- WEBCAPSULE : FRAME_SNAPTHOT : BEGIN --->\n");
        output.append(visibleStr);
        frameContentAsPlainText(curChild, output);
        output.append("\n<!--- WEBCAPSULE : FRAME_SNAPTHOT : END --->\n");
    }
}


}



