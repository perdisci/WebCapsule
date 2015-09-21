/*
 * ForensicReplayDataStore.h
 *
 *  Created on: Jul 25, 2014
 *      Author: cjneasbi
 */

#ifndef ForensicReplayDataStore_h
#define ForensicReplayDataStore_h

// #include "core/inspector/WebCapsule.h"

#include "core/inspector/ForensicEvent.h"
#include "core/inspector/ForensicReplayData.h"
#include "core/inspector/ForensicRequestAssembler.h"

namespace WebCore {

class ForensicReplayDataStore :  public ForensicRequestAssembler, Serializable {
public:
	static PassRefPtr<ForensicReplayDataStore> create();

	virtual ~ForensicReplayDataStore();

	static const char* WEBCAPSULE_HEADER;

    double recordStartTime();
    double replayStartTime();
    virtual void setRecordStartTime(double t);
    void setReplayStartTime(double t);

    double recordStartTimeMonotonic();
    double replayStartTimeMonotonic();
    virtual void setRecordStartTimeMonotonic(double t);
    void setReplayStartTimeMonotonic(double t);

	void markPageURLRequestAsConsumed(String& pageURL);

	//virtual size_t sizePageURLRequests(const String&);
	PassRefPtr<PageURLRequest> getPageURLRequest(const PageURLRequestMapLocation&);

	// used to store the target element of mouse clicks
    bool containsKeyPressEventTargetRecordInfo(const String&);
    virtual void setKeyPressEventTargetRecordInfo(const String&, KeyPressEventTargetInfo);
    const KeyPressEventTargetInfo getKeyPressEventTargetRecordInfo(const String&);
    void setKeyPressEventTargetReplayInfo(const String&, KeyPressEventTargetInfo);
    bool containsMousePressEventTargetRecordInfo(const String&);
    virtual void setMousePressEventTargetRecordInfo(const String&, const MousePressEventTargetInfo);
    const MousePressEventTargetInfo getMousePressEventTargetRecordInfo(const String&);
    void setMousePressEventTargetReplayInfo(const String&, const MousePressEventTargetInfo);

    bool containsRequest(const blink::WebURLRequest&);
    PassRefPtr<RequestMapEntry> nextRequestEntry(const blink::WebURLRequest&);

    //virtual void setStackTraceForRequest(const String&, const V8StackTrace&);
    bool containsStackTraceForRequest(const String&);
    void clearStackTraceForRequest(const String&);
    V8StackTrace peekStackTraceForRequest(const String&);
    V8StackTrace getStackTraceForRequest(const String&); //clears from the map
    blink::WebURLRequest findMatchingRequest(
    		const String&,
    		double,
    		int = (V8StackTraceMatchingEntry::Terminal | V8StackTraceMatchingEntry::Redirect)); //should only be called during replay

    PassRefPtr<RedirectEntry> nextRedirectPair(const blink::WebURLRequest&);
    bool containsRedirect(const blink::WebURLRequest&);

    const Vector<RefPtr<ForensicEvent> >& forensicEvents();

    virtual void appendForensicEvent(PassRefPtr<ForensicEvent>);

    virtual void appendCurrentTime(double);
    double nextCurrentTime();
    double nextCurrentTimePreview();
    double currentTimeOffset();
    void setCurrentTimeOffset(double);

    virtual void appendMonotonicallyIncreasingTime(double);
    double nextMonotonicallyIncreasingTime();
    double nextMonotonicallyIncreasingTimePreview();
    double monotonicallyIncreasingTimeOffset();
    void setMonotonicallyIncreasingTimeOffset(double);

    virtual void appendUserAgent(PassOwnPtr<blink::WebString>);
    blink::WebString nextUserAgent();

    virtual void appendMemoryUsageMB(size_t);
    size_t nextMemoryUsageMB();

    virtual void appendActualMemoryUsageMB(size_t);
    size_t nextActualMemoryUsageMB();

    virtual void appendPhysicalMemoryMB(size_t);
    size_t nextPhysicalMemoryMB();

    virtual void appendNumberOfProcessors(size_t);
    size_t nextNumberOfProcessors();

    virtual void appendDefaultLocale(PassOwnPtr<blink::WebString>);
    blink::WebString nextDefaultLocale();

    virtual void appendCryptographicallyRandomValues(unsigned char*, size_t);
    bool nextCryptographicallyRandomValues(unsigned char*, size_t);

    virtual void appendIsLinkVisited(unsigned long long, bool);
    bool nextIsLinkVisited(unsigned long long);

    virtual void appendVisitedLinkHash(const char*, size_t, unsigned long long);
    unsigned long long nextVisitedLinkHash(const char*, size_t);

    virtual void appendSignedPublicKeyAndChallengeString(unsigned, const blink::WebString&, const blink::WebURL&, blink::WebString&);
    blink::WebString nextSignedPublicKeyAndChallengeString(unsigned, const blink::WebString&, const blink::WebURL&);

    virtual void appendClipboardSequenceNumber(blink::WebClipboard::Buffer, uint64);
    uint64 nextClipboardSequenceNumber(blink::WebClipboard::Buffer);
    //Modaldialog by Bo
    virtual void appendModaldialogConfirmResult(const String&, bool);
    bool nextModaldialogConfirmResult(const String&);

    virtual void appendModaldialogPromptResult(const String&,const String&);
    String nextModaldialogPromptResult(const String&);

    virtual void appendModaldialogAlertVals(const String&);


    virtual void appendClipboardIsFormatAvailable(blink::WebClipboard::Format, blink::WebClipboard::Buffer, bool);
    bool nextClipboardIsFormatAvailable(blink::WebClipboard::Format, blink::WebClipboard::Buffer);

    virtual void appendClipboardReadAvailableTypes(blink::WebClipboard::Buffer, bool*, blink::WebVector<blink::WebString>&);
    blink::WebVector<blink::WebString> nextClipboardReadAvailableTypes(blink::WebClipboard::Buffer, bool*);

    virtual void appendClipboardReadPlainText(blink::WebClipboard::Buffer, blink::WebString&);
    blink::WebString nextClipboardReadPlainText(blink::WebClipboard::Buffer);

    virtual void appendClipboardReadHTML(blink::WebClipboard::Buffer, blink::WebURL*, unsigned*, unsigned*, blink::WebString&);
    blink::WebString nextClipboardReadHTML(blink::WebClipboard::Buffer, blink::WebURL*, unsigned*, unsigned*);

    virtual void appendClipboardReadImage(blink::WebClipboard::Buffer, blink::WebData&);
    blink::WebData nextClipboardReadImage(blink::WebClipboard::Buffer);

    virtual void appendClipboardReadCustomData(blink::WebClipboard::Buffer, const blink::WebString&, blink::WebString&);
    blink::WebString nextClipboardReadCustomData(blink::WebClipboard::Buffer, const blink::WebString&);

    //Clears all stored data, should clear each data structure.
    virtual void clear();

    //Reset state of store for replay
    void reset();

    //Returns true if the size of any of the data structures is greater than zero, should test each data structure.
    bool containsData();

    //Returns data stored by the data store in JSON format
    PassRefPtr<JSONObject> serialize();
    PassRefPtr<JSONObject> serializeReplayInfo();

    void deserialize(PassRefPtr<JSONObject> json);

    virtual void stopRecording(){};

private:
    ForensicReplayDataStore();

    //This function should call all of the reset function below
    void resetMaps();

    //There should be one reset function for each HashMap;
    void resetKeyPressEventTargetReplayInfoMap();
    void resetMousePressEventTargetReplayInfoMap();
    void resetPageURLRequestsMap();
    void resetRequests();
    void resetRedirects();
    void resetStackTraces();
    void resetStackTraceMatchingMap();
    void resetCRV();
    void resetIsLinkVisitedMap();
    void resetVisitedLinkHashMap();
    void resetSignedPublicKeyAndChallengeStringMap();
    void resetClipboardSequenceNumberMap();
    void resetConfirmResultMap();
    void resetClipboardIsFormatAvailableMap();
    void resetClipboardReadAvailableTypesMap();
    void resetClipboardReadPlainTextMap();
    void resetClipboardReadHTMLMap();
    void resetClipboardReadImageMap();
    void resetClipboardReadCustomDataMap();

    //modaldialog by Bo
    void resetModaldialogConfirmResultMap();
    void resetModaldialogPromptResultMap();

    void resetVectors();
    void resetOffSets();
    void resetEventState();

    //There should be one serialize and one deserialize function for each HashMap except m_pendingMap
    void serializeRequests(PassRefPtr<JSONObject> json);
    void deserializeRequests(PassRefPtr<JSONObject> json);

    void serializeRedirects(PassRefPtr<JSONObject> json);
    void deserializeRedirects(PassRefPtr<JSONObject> json);

    void serializeKeyPressEventTargetMap(PassRefPtr<JSONObject> json, const KeyPressEventTargetInfoMap& map, const String& state);
    void deserializeKeyPressEventTargetRecordInfoMap(PassRefPtr<JSONObject> json);

    void serializeMousePressEventTargetMap(PassRefPtr<JSONObject> json, const MousePressEventTargetInfoMap& map, const String& state);
    void deserializeMousePressEventTargetRecordInfoMap(PassRefPtr<JSONObject> json);

    PassRefPtr<JSONArray> serializeStackTraceMatchingMap(); // for debugging

    void serializePageURLAliasMap(PassRefPtr<JSONObject> json);
    void deserializePageURLAliasMap(PassRefPtr<JSONObject> json);

    void serializedPageURLRequestsMap(PassRefPtr<JSONObject> json);
    void deserializedPageURLRequestsMap(PassRefPtr<JSONObject> json);

    void serializeCRV(PassRefPtr<JSONObject> json);
    void deserializeCRV(PassRefPtr<JSONObject> json);

    void serializeIsLinkVisitedMap(PassRefPtr<JSONObject> json);
    void deserializeIsLinkVisitedMap(PassRefPtr<JSONObject> json);

    void serializeVisitedLinkHashMap(PassRefPtr<JSONObject> json);
    void deserializeVisitedLinkHashMap(PassRefPtr<JSONObject> json);

    void serializeSignedPublicKeyAndChallengeStringMap(PassRefPtr<JSONObject> json);
    void deserializeSignedPublicKeyAndChallengeStringMap(PassRefPtr<JSONObject> json);

    void serializeClipboardSequenceNumberMap(PassRefPtr<JSONObject> json);
    void deserializeClipboardSequenceNumberMap(PassRefPtr<JSONObject> json);


    /*modified by Bo*/
    void serializeModaldialogConfirmResultMap(PassRefPtr<JSONObject> json);
    void deserializeModaldialogConfirmResultMap(PassRefPtr<JSONObject> json);
    void serializeModaldialogPromptResultMap(PassRefPtr<JSONObject> json);
    void deserializeModaldialogPromptResultMap(PassRefPtr<JSONObject> json);

    void serializeClipboardIsFormatAvailableMap(PassRefPtr<JSONObject> json);
    void deserializeClipboardIsFormatAvailableMap(PassRefPtr<JSONObject> json);

    void serializeClipboardReadAvailableTypesMap(PassRefPtr<JSONObject> json);
    void deserializeClipboardReadAvailableTypesMap(PassRefPtr<JSONObject> json);

    void serializeClipboardReadPlainTextMap(PassRefPtr<JSONObject> json);
    void deserializeClipboardReadPlainTextMap(PassRefPtr<JSONObject> json);

    void serializeClipboardReadHTMLMap(PassRefPtr<JSONObject> json);
    void deserializeClipboardReadHTMLMap(PassRefPtr<JSONObject> json);

    void serializeClipboardReadImageMap(PassRefPtr<JSONObject> json);
    void deserializeClipboardReadImageMap(PassRefPtr<JSONObject> json);

    void serializeClipboardReadCustomDataMap(PassRefPtr<JSONObject> json);
    void deserializeClipboardReadCustomDataMap(PassRefPtr<JSONObject> json);

    void serializeMaps(PassRefPtr<JSONObject> json);
    void serializeReplayInfoMaps(PassRefPtr<JSONObject> json);
    void deserializeMaps(PassRefPtr<JSONObject> json);

    void serializeVectors(PassRefPtr<JSONObject> json);
    void deserializeVectors(PassRefPtr<JSONObject> json);

    void deserializeVector(PassRefPtr<JSONObject> json, const String& key, SerializableVector& buf);
    PassRefPtr<ForensicEvent> deserializeForensicEvent(PassRefPtr<JSONObject> json);

    virtual void addFinishedRequest(PassRefPtr<PendingRequestMapEntry> entry);
    virtual void addFinishedRedirect(blink::WebURLRequest&, blink::WebURLRequest&, const blink::WebURLResponse&, const V8StackTrace&, double);

	// WebURLLoader data
    RequestMap m_requestMap;
    RedirectMap m_redirectMap;
    //PendingRequestMap m_pendingMap; //collects data from multiple async callbacks in ForensicWebURLLoader, never to be used during replay
    //StackTraceMap m_stackTraceMap; //collects stack traces associated with the creation of a request, should not be serialized
    StackTraceMatchingMap m_stackTraceMatchingMap; //computed from m_requestMap and m_redirectMap when reset is called, should not be serialized

    //PageURLAliasMap m_pageURLAliasMap;
    //PageURLRequestsMap m_pageURLRequestsMap;

	// WebClipboard data
	ClipboardSequenceNumberMap m_clipboardSequenceNumberMap;
	ClipboardIsFormatAvailableMap m_clipboardIsFormatAvailableMap;
	ClipboardReadAvailableTypesMap m_clipboardReadAvailableTypesMap;
	ClipboardReadPlainTextMap m_clipboardReadPlainTextMap;
	ClipboardReadHTMLMap m_clipboardReadHTMLMap;
	ClipboardReadImageMap m_clipboardReadImageMap;
	ClipboardReadCustomDataMap m_clipboardReadCustomDataMap;

	//Modal dialog confirm
	ModaldialogConfirmResultMap m_modaldialogConfirmResultMap;

	//Modal dialog prompt
	ModaldialogPromptResultMap m_modaldialogPromptResultMap;

    // Web API data
    Vector<RefPtr<ForensicEvent> > m_events;

    double m_recordStartTime; // to be serialized
    double m_replayStartTime; // no need to serialize
    double m_recordStartTimeMonotonic; // to be serialized
    double m_replayStartTimeMonotonic; // no need to serialize

    KeyPressEventTargetInfoMap m_KeyPressEventTargetRecordInfoMap;
    KeyPressEventTargetInfoMap m_KeyPressEventTargetReplayInfoMap;
    MousePressEventTargetInfoMap m_MousePressEventTargetRecordInfoMap;
    MousePressEventTargetInfoMap m_MousePressEventTargetReplayInfoMap;

    // Platform API data
	OwnPtr<DoubleVector> m_currentTimeVals;
	double m_currentTimeOffset;

	OwnPtr<DoubleVector> m_monotonicallyIncreasingTimeVals;
	double m_monotonicallyIncreasingTimeOffset;

	CRVMap m_cryptographicallyRandomValuesMap;

	OwnPtr<WebStringVector> m_userAgentVals;
	OwnPtr<SizeTypeVector> m_memoryUsageMBVals;
	OwnPtr<SizeTypeVector> m_actualMemoryUsageMBVals;
	OwnPtr<SizeTypeVector> m_physicalMemoryMBVals;
	OwnPtr<SizeTypeVector> m_numberOfProcessorsVals;
	OwnPtr<WebStringVector> m_defaultLocaleVals;
	OwnPtr<WebStringVector> m_modaldialogAlertVals; //modal dialog alert

	IsLinkVisitedMap m_isLinkVisitedMap;
	VisitedLinkHashMap m_visitedLinkHashMap;
	SignedPublicKeyAndChallengeStringMap m_signedPublicKeyAndChallengeStringMap;
};

} /* namespace WebCore */
#endif /* ForensicReplayDataStore_h */
