/*
 * InspectorForensicsAgent.h
 *
 *  Created on: Feb 24, 2014
 *      Author: cjneasbi
 */

#ifndef InspectorForensicsAgent_h
#define InspectorForensicsAgent_h

#include "InspectorFrontend.h"
#include "core/inspector/InspectorBaseAgent.h"
#include "core/inspector/InspectorPageAgent.h"
#include "core/inspector/ForensicEvent.h"
#include "wtf/Noncopyable.h"
#include "wtf/OwnPtr.h"
#include "core/inspector/ForensicReplayDataStore.h"
#include "core/loader/FrameLoaderTypes.h"
#include "core/loader/HistoryItem.h"
#include "core/dom/Element.h"
#include "core/fetch/FetchInitiatorInfo.h"
#include "core/fetch/ResourceLoader.h"

namespace blink{

class WebInputEvent;
class WebThread;
class WebURLResponse;
class WebURLRequest;
class WebURLError;
class WebData;
class WebSize;

}

namespace WebCore {

class Page;
class Document;
class Event;
class InspectorClient;
class InstrumentingAgents;
class ForensicPlatformWrapper;
class ForensicReplayDataStore;
class KURL;
class LocalFrame;

namespace ForensicsAgentState {
static const char forensicsAgentRecording[] = "forensicsAgentRecording";
static const char forensicsAgentReplaying[] = "forensicsAgentReplaying";
static const char forensicsAgentStateID[] = "forensicsAgentStateID";
}

class InspectorForensicsAgent FINAL
	: public InspectorBaseAgent<InspectorForensicsAgent>
	, public InspectorBackendDispatcher::ForensicsCommandHandler {
    WTF_MAKE_NONCOPYABLE(InspectorForensicsAgent);
public:
    static PassOwnPtr<InspectorForensicsAgent> create(Page*, InspectorClient*, InspectorPageAgent*);

    virtual ~InspectorForensicsAgent();

    // InspectorInstrumentation Hooks
    void handleInputEvent(const blink::WebInputEvent&);
    void handleRenderMenuListValueChanged(unsigned int, bool, const Node*);
    void handleSetEditingValueEvent(const String& inputElementName, const String& value);
    void handleSetAutofilledEvent(const String& inputElementName, bool autofilled);
    void handleURLLoad(LocalFrame*, const blink::WebURLRequest&); //for url's entered into the address bar, WebFrameImpl.loadRequest
    void handleFrameLoad(LocalFrame*, const blink::WebURLRequest&, const Document*, const Event*); // used for informational purposes and for replay "self-healing"
    void handleUpdateForSameDocumentNavigation(LocalFrame*, const KURL&, const KURL&);
    void handleLoadHistoryItem(LocalFrame*, HistoryItem*, HistoryLoadType, ResourceRequestCachePolicy);
    void handlePageScroll(const blink::WebSize&, double);
    void handleResize(const blink::WebSize&);
    void willSendRequest(unsigned long identifier, DocumentLoader*, ResourceRequest&, const ResourceResponse& redirectResponse, const FetchInitiatorInfo&);

    //modaldialog by Bo
    bool handleModaldialogConfirmResultReplay(const String& message, bool& var);
    void handleModaldialogConfirmResultRecording(const String& message, bool result);
    bool handleModaldialogPromptResultReplay(const String& message, String& var);
    void handleModaldialogPromptResultRecording(const String& message, const String& result);
    void handleModaldialogAlertRecording(const String& message);
    bool handleModaldialogAlertReplay();


    // information only (do not replay!)
    void handleEventHandlerKeyEvent(const PlatformKeyboardEvent& keyEvent, const Node* targetNode);
    void handleEventHandlerMousePressEvent(const PlatformMouseEvent& mouseEvent, const Node* targetNode);

    // DOM snapshots
    static void takeDOMTextSnapshot(Page* page, String& domSnapshot, double timestamp);
    static void takeDOMPicture(Page* page, const String& baseDir, double timestamp);



    // Inspector Commands from ForensicsCommandHandler
    virtual void startReplay(ErrorString*, const double* in_speed) OVERRIDE;
    virtual void stopReplay(ErrorString*) OVERRIDE;
    virtual void startRecording(ErrorString*) OVERRIDE;
    virtual void stopRecording(ErrorString*) OVERRIDE;
    virtual void dumpRecording(ErrorString*, RefPtr<JSONObject>& out_recording) OVERRIDE;
    virtual void loadRecording(ErrorString*, const RefPtr<JSONObject>& in_recording) OVERRIDE;
    virtual void dumpReplayInfo(ErrorString*, RefPtr<JSONObject>& out_rplay_info) OVERRIDE;

    //Convenience wrappers for the inspector commands
    void startReplay();
    void stopReplay();
    void startRecording();
    void stopRecording();

    //Convenience wrappers for the inspector commands, implements state checking
    void startReplay(long);
    void stopReplay(long);
    void startRecording(long);
    void stopRecording(long);

    // Inspector Controller API
    virtual void init() OVERRIDE;
    virtual void setFrontend(InspectorFrontend*) OVERRIDE;
    virtual void clearFrontend() OVERRIDE;
    virtual void restore() OVERRIDE;

    void setDoTakeDOMTextSnapshot(bool);
    void setDoTakeDOMPicture(bool);
    void setDOMPictureBaseLogDir(const String&);

    bool replaying();
    bool recording();
    bool replaying(long);
    bool recording(long);
    long stateId();

    ForensicPlatformWrapper* platformWrapper();
    static InspectorFrontend::Forensics* forensicsFrontend();

private:
    InspectorForensicsAgent(Page*, InspectorClient*, InspectorPageAgent*);
    const char * typeString(const blink::WebInputEvent&);

    void setReplayState(bool);
    void setRecordingState(bool);
    void setStateId(long);
    bool checkStateId(long);
    void updateStateId();
    RefPtr<TypeBuilder::Forensics::Request> createRequest(blink::WebURLRequest&);
    const KURL& getCurrentURL();
    Page* getCurrentPage();
    const double getWrappedCurrentTime();
    const double getWrappedMonotonicallyIncreasingTime();
    void initPlatformWrapper();
    PageURLRequestMapLocation createMapLocation();

    Page* m_page;
    InspectorFrontend::Forensics* m_frontendForensics;
    InspectorClient* m_client;
    InspectorPageAgent* m_pageAgent;
    bool m_recording;
    bool m_replaying;
    long m_stateId; //state token, should be changed at every change of state

    bool m_doTakeDOMTextSnapshot;
    bool m_doTakeDOMPicture;
    String m_domPicBaseDir;

    // useful for agent tracking purposes
	unsigned int m_agentID;
	String m_firstPageURL;

	unsigned int agentID() { return m_agentID; }
	const String& firstPageURL() { return m_firstPageURL; }

    static void takeSkPicture(Page* page, String& dumpDir);
    static void frameContentAsPlainText(LocalFrame* frame, String& output);

    V8StackTrace getCurrentStackTrace();

    //TODO figure out how to persistently store events across navigations, i.e entering URL in address bar
    RefPtr<ForensicReplayDataStore> m_dataStore;
    RefPtr<ForensicReplayDataRecorder> m_dataRecorder;

    bool m_recordingOnStart;
    bool m_inMemoryRecording;

};

} // namespace WebCore

#endif /* InspectorForensicsAgent_h */
