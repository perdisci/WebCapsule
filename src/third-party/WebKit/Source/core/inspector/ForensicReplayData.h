/*
 * ForensicReplayData.h
 *
 *  Created on: Oct 8, 2014
 *      Author: cjneasbi
 */

#ifndef ForensicReplayData_h
#define ForensicReplayData_h

#include "core/inspector/ForensicSerialization.h"
#include "core/inspector/WebURLRequestTraits.h"
#include "wtf/PassOwnPtr.h"
#include "wtf/OwnPtr.h"
#include "wtf/Vector.h"
#include "wtf/text/Base64.h"
#include "wtf/text/StringBuilder.h"
#include "public/platform/WebData.h"
#include "public/platform/WebVector.h"
#include "public/platform/WebClipboard.h"
#include "public/platform/WebURLRequest.h"
#include "public/platform/WebURLResponse.h"
#include "public/platform/WebURLError.h"
#include "public/platform/WebURLLoaderClient.h"
#include "platform/JSONValues.h"
#include "platform/CrossThreadCopier.h"
#include "platform/exported/WrappedResourceRequest.h"
#include "platform/exported/WrappedResourceResponse.h"
#include "platform/network/ResourceRequest.h"
#include "platform/network/ResourceResponse.h"
#include "platform/weborigin/KURL.h"
#include "core/inspector/WebCapsule.h"

/*
 * The following are class primarily used by ForensicReplayDataStore
 */
namespace WebCore {

class V8StackFrame : public Serializable {
public:
	V8StackFrame(const v8::Handle<v8::StackFrame>&);
	V8StackFrame(PassRefPtr<JSONObject> json);
	virtual ~V8StackFrame();

	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize();
	virtual void deserialize(PassRefPtr<JSONObject> json);

	int line;
	int col;
	String functionName;
	String scriptName;
};

class V8StackTrace : public SerializableVector, public Vector<V8StackFrame> {
public:
	virtual ~V8StackTrace();

	virtual PassRefPtr<JSONArray> serialize();
	virtual void deserialize(PassRefPtr<JSONArray> json);
};

class V8StackTraceMatchingEntry : public Serializable {
public:
	//bit masking
	enum Source {
		Terminal = 1,
		Redirect = 1 << 1
	};

	V8StackTraceMatchingEntry(const blink::WebURLRequest*, double, Source);
	virtual ~V8StackTraceMatchingEntry();

	const blink::WebURLRequest* request() const;
	double startTime() const;
	Source source() const;
	bool operator<(const V8StackTraceMatchingEntry& other) const;

	virtual PassRefPtr<JSONObject> serialize(); //for debugging
	virtual void deserialize(PassRefPtr<JSONObject> json){};

private:
	const blink::WebURLRequest* m_request;
	double m_startTime;
	Source m_source;
};

class RequestMapEntry : public ThreadSafeRefCounted<RequestMapEntry>, Serializable{
public:
	static PassRefPtr<RequestMapEntry> create();
	static PassRefPtr<RequestMapEntry> create(blink::WebURLResponse& response,
			blink::WebURLError& error,
			blink::WebData& data,
			V8StackTrace& stackTrace,
			double finishTime = 0,
			double endTime = 0,
			double startTime = 0,
			int64 totalEncodedDataLength = 0);
	virtual ~RequestMapEntry();

	void setResponse(const blink::WebURLResponse& response);
	double finishTime();
	void setFinishTime(double finishTime);
	double endTime();
	void setEndTime(double endTime); // this should be set to the currentTime() take when didFinishLoading is called

	double startTime();
	void setStartTime(double startTime);

	void setTotalEncodedDataLength(int64 length);
	V8StackTrace* stackTrace();
	void setStackTrace(const V8StackTrace&);
	void clearStackTrace();

	virtual size_t size();

	virtual PassRefPtr<JSONObject> serialize();
	virtual void deserialize(PassRefPtr<JSONObject> json);

	blink::WebURLResponse m_response;
	blink::WebURLError m_error;
	Vector<char> m_data;
	int64 m_totalEncodedDataLength;
	V8StackTrace m_stackTrace;


private:
	RequestMapEntry();
	RequestMapEntry(blink::WebURLResponse& response,
			blink::WebURLError& error,
			blink::WebData& data,
			V8StackTrace& stackTrace,
			double finishTime,
			double endTime,
			double startTime,
			int64 totalEncodedDataLength);

	double m_startTime; //the time when request loading began
	double m_finishTime;
	double m_endTime;
};

class RequestMapValue : public SerializableVector {
public:
	static PassOwnPtr<RequestMapValue> create();
	static PassOwnPtr<RequestMapValue> create(blink::WebURLResponse& response,
			blink::WebURLError& error,
			blink::WebData& data,
			V8StackTrace& stackTrace,
			double finishTime,
			double endTime,
			double startTime,
			int64 totalEncodedDataLength);
	virtual ~RequestMapValue();

	void add(blink::WebURLResponse& response,
			blink::WebURLError& error,
			blink::WebData& data,
			V8StackTrace& stackTrace,
			double finishTime,
			double endTime,
			double startTime,
			int64 totalEncodedDataLength);
	PassRefPtr<RequestMapEntry> next();
	PassRefPtr<RequestMapEntry> current();  //does not advance m_index
	void reset();
	size_t size();
	Vector<RefPtr<RequestMapEntry> >& values();

	virtual PassRefPtr<JSONArray> serialize() OVERRIDE ;
	virtual void deserialize(PassRefPtr<JSONArray> json) OVERRIDE ;

private:
	RequestMapValue();
	RequestMapValue(blink::WebURLResponse& response,
			blink::WebURLError& error,
			blink::WebData& data,
			V8StackTrace& stackTrace,
			double finishTime,
			double endTime,
			double startTime,
			int64 totalEncodedDataLength);

	Vector<RefPtr<RequestMapEntry> > m_values;
	size_t m_index;
};

class PendingRequestMapEntry : public RefCounted<PendingRequestMapEntry> {
public:
	static PassRefPtr<PendingRequestMapEntry> create();
	static PassRefPtr<PendingRequestMapEntry> create(blink::WebURLRequest& request);
	static PassRefPtr<PendingRequestMapEntry> create(blink::WebURLRequest& request,
			blink::WebURLResponse& response,
			blink::WebURLError& error,
			blink::WebData& data,
			V8StackTrace& stackTrace);
	virtual ~PendingRequestMapEntry();

	virtual size_t size();

	void setRequest(const blink::WebURLRequest& request);

	blink::WebURLRequest m_request;
	RefPtr<RequestMapEntry> m_mapEntry;

private:
    PendingRequestMapEntry();
	PendingRequestMapEntry(blink::WebURLRequest& request);
	PendingRequestMapEntry(blink::WebURLRequest& request,
			blink::WebURLResponse& response,
			blink::WebURLError& error,
			blink::WebData& data,
			V8StackTrace& stackTrace);

};

class RedirectEntry : public ThreadSafeRefCounted<RedirectEntry>, Serializable {
public:
	static PassRefPtr<RedirectEntry> create();
	static PassRefPtr<RedirectEntry> create(
			blink::WebURLRequest& request,
			const blink::WebURLResponse& response,
			const V8StackTrace& stackTrace,
			double startTime = 0);
	virtual ~RedirectEntry();

	virtual size_t size();

	void setRequest(const blink::WebURLRequest& request);
	void setResponse(const blink::WebURLResponse& response);
	V8StackTrace* stackTrace();
	void setStackTrace(const V8StackTrace& stackTrace);
	void clearStackTrace();
	double startTime();
	void setStartTime(double startTime);

	virtual PassRefPtr<JSONObject> serialize();
	virtual void deserialize(PassRefPtr<JSONObject> json);

	OwnPtr<ResourceRequest> m_request;
	OwnPtr<ResourceResponse> m_response;


private:
	RedirectEntry();
	RedirectEntry(
			const blink::WebURLRequest& request,
			const blink::WebURLResponse& response,
			const V8StackTrace& stackTrace,
			double startTime);

	V8StackTrace m_stackTrace; //stack trace for the original request
	double m_startTime; //the time when request loading began

};

class RedirectMapValue : public SerializableVector{
public:
	static PassOwnPtr<RedirectMapValue> create();
	static PassOwnPtr<RedirectMapValue> create(
			blink::WebURLRequest& request,
			const blink::WebURLResponse& response,
			const V8StackTrace& stackTrace,
			double startTime = 0);
	~RedirectMapValue();

	void add(blink::WebURLRequest& request, const blink::WebURLResponse& response, const V8StackTrace& stackTrace, double startTime = 0);
	PassRefPtr<RedirectEntry> next();
	PassRefPtr<RedirectEntry> current(); //does not advance m_index
	void reset();
	size_t size();
	Vector<RefPtr<RedirectEntry> >& values();

	virtual PassRefPtr<JSONArray> serialize() OVERRIDE ;
	virtual void deserialize(PassRefPtr<JSONArray> json) OVERRIDE ;
private:
	RedirectMapValue();
	RedirectMapValue(blink::WebURLRequest& request, const blink::WebURLResponse& response, const V8StackTrace& stackTrace, double startTime);

	Vector<RefPtr<RedirectEntry> > m_values;
	size_t m_index;
};

// PageURLRequest is used for self-healing replay
// XXX: Currently, self-healing is only partially supported because we do not deal with local frames; we only deal with main frame incongruence
// To support local frames we would also need to record the path from the root of the FrameTree to the current frame, plus the current frame URL, besides the page (i.e., main frame) URL
// The page URL, frame tree path and frame URL would also need to be stored with each ForensicInputEvent, likely adding a good bit of overhead!
// TODO serialize
class PageURLRequest : public RefCounted<PageURLRequest>, Serializable {
public:
    static PassRefPtr<PageURLRequest> create(
    		const blink::WebURLRequest& request,
    		const String& frameName,
    		const bool isMainFrame,
    		const String& originURL,
    		const String& eventType,
    		const String& nodeName,
    		const String& nodeHTML,
    		const String& parentHTML);
    static PassRefPtr<PageURLRequest> createRedirect(const blink::WebURLRequest& request, const String& originURL);
    static PassRefPtr<PageURLRequest> createEmpty();
    virtual ~PageURLRequest();

    PageURLRequest& operator=(const PageURLRequest& r);
    bool isEmpty();
    bool isConsumed();
    void setIsConsumed(bool consumed);
    blink::WebURLRequest& webURLRequest();
    size_t size();

	virtual PassRefPtr<JSONObject> serialize();
	virtual void deserialize(PassRefPtr<JSONObject> json);

private:
    PageURLRequest();
    PageURLRequest(
    		const blink::WebURLRequest& request,
    		const String& frameName,
    		const bool isMainFrame,
    		const bool isRedirectRequest,
    		const String& originURL,
    		const String& eventType,
    		const String& nodeName,
    		const String& nodeHTML,
    		const String& parentHTML);

    void copyWebURLRequest(const blink::WebURLRequest& urlRequest);

    blink::WebURLRequest m_webURLRequest; // request to be loaded into the frame
    String m_frameName;  // frame where the URL request will be loaded
    bool m_isMainFrame;
    bool m_isRedirectRequest; // true if this is a redirection for a previous frame load request
    String m_originURL;  // URL of the page where the event that caused the page transition occurred
    String m_eventType;  // type of triggering event (e.g., "click"); see //src/third_party/WebKit/Source/core/events/EventTypeNames.in
    String m_nodeName;   // for HTML elements, this is the tag name; probably not useful, given the presence of m_nodeHTML
    String m_nodeHTML;   // HTML of the element "activated" by the triggering event
    String m_parentHTML; // HTML of the parent node
    bool m_isEmpty;		// XXX: what is the purpose of this element
    bool m_isConsumed;   // if the PageURLRequest is used, this should be set to true
};

struct PageURLRequestMapLocation {
	String currentUrlRootAlias;
	size_t pos;
};

class GenericKey : public WTF::GenericHashTraits<GenericKey>, Serializable{
public:
	static const bool hasIsEmptyValueFunction = true;
	static const bool safeToCompareToEmptyOrDeleted = false;

	GenericKey(bool empty);
	GenericKey();
	virtual ~GenericKey();

	virtual size_t size();

	bool operator==(const GenericKey& other) const;
	void del();
	bool deleted() const;
	bool isEmpty() const;

	//Needs to be overridded
	virtual unsigned hashVal() const;

	//Needs to be overridded
	virtual PassRefPtr<JSONObject> serialize();

	//Needs to be overridded
	virtual void deserialize(PassRefPtr<JSONObject> json);

    static GenericKey emptyValue();
    static bool isEmptyValue(const GenericKey& value);
    static void constructDeletedValue(GenericKey& value);
    static bool isDeletedValue(const GenericKey& value);
	static unsigned hash(const GenericKey& object);
	static bool equal(const GenericKey& a, const GenericKey& b);

private:
	bool m_deleted;
	bool m_empty;
};

class IsFormatAvailableKey : public GenericKey {
public:
	IsFormatAvailableKey();
	IsFormatAvailableKey(blink::WebClipboard::Format format, blink::WebClipboard::Buffer buffer);
	virtual ~IsFormatAvailableKey();

	virtual size_t size();

	virtual unsigned hashVal() const OVERRIDE;
	virtual PassRefPtr<JSONObject> serialize() OVERRIDE ;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE ;

private:
	blink::WebClipboard::Format m_format;
	blink::WebClipboard::Buffer m_buffer;
};

class ReadAvailableTypesKey : public GenericKey {
public:
	ReadAvailableTypesKey();
	ReadAvailableTypesKey(blink::WebClipboard::Buffer buffer, bool containsFilenames);
	virtual ~ReadAvailableTypesKey();

	virtual size_t size();

	virtual unsigned hashVal() const OVERRIDE;
	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	blink::WebClipboard::Buffer m_buffer;
	bool m_containsFilenames;
};

class ReadHTMLKey : public GenericKey  {
public:
	ReadHTMLKey();
	ReadHTMLKey(blink::WebClipboard::Buffer buffer, blink::WebURL sourceUrl, unsigned fragmentStart, unsigned fragmentEnd);
	virtual ~ReadHTMLKey();

	virtual size_t size();

	virtual unsigned hashVal() const OVERRIDE;
	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	blink::WebClipboard::Buffer m_buffer;
	blink::WebURL m_sourceUrl;
	unsigned m_fragmentStart;
	unsigned m_fragmentEnd;
};

class CustomDataKey : public GenericKey {
public:
	CustomDataKey();
	CustomDataKey(blink::WebClipboard::Buffer buffer, blink::WebString string);
	virtual ~CustomDataKey();

	virtual size_t size();

	virtual unsigned hashVal() const OVERRIDE;
	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	blink::WebClipboard::Buffer m_buffer;
	blink::WebString m_string;
};

class PublicKeyAndChallengeStringKey : public GenericKey {
public:
	PublicKeyAndChallengeStringKey();
	PublicKeyAndChallengeStringKey(unsigned keySizeIndex, blink::WebString challenge, blink::WebURL url);
	virtual ~PublicKeyAndChallengeStringKey();

	virtual size_t size();

	virtual unsigned hashVal() const OVERRIDE;
	virtual PassRefPtr<JSONObject> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONObject> json) OVERRIDE;

private:
	unsigned m_keySizeIndex;
	blink::WebString m_challenge;
	blink::WebURL m_url;
};


class KeyPressEventTargetInfo {
public:

	KeyPressEventTargetInfo();
	KeyPressEventTargetInfo(double t, const String& keyID, const String& keyText, const String& te, const String& pe, const String& dom);
	KeyPressEventTargetInfo(const KeyPressEventTargetInfo& info);

	KeyPressEventTargetInfo& operator=(const KeyPressEventTargetInfo& info);
	bool operator==(const KeyPressEventTargetInfo& info);

	size_t size();

	PassRefPtr<JSONObject> serialize();
	void deserialize(PassRefPtr<JSONObject> json);

	double m_timestamp;
	String m_keyID;
	String m_keyText;
	String m_targetElementHTML;
	String m_parentElementHTML;
	String m_domTextSnapshot;
};



class MousePressEventTargetInfo {
public:

	MousePressEventTargetInfo();
	MousePressEventTargetInfo(double t, int x, int y, const String& te, const String& pe, const String& dom);
	MousePressEventTargetInfo(const MousePressEventTargetInfo& info);

	MousePressEventTargetInfo& operator=(const MousePressEventTargetInfo& info);
	bool operator==(const MousePressEventTargetInfo& info);

	size_t size();

	PassRefPtr<JSONObject> serialize();
	void deserialize(PassRefPtr<JSONObject> json);

	double m_timestamp;
	int m_x;
	int m_y;
	String m_targetElementHTML;
	String m_parentElementHTML;
	String m_domTextSnapshot;
};


template<class T>
class ObjectVector : public SerializableVector {
public:
	static PassOwnPtr<ObjectVector> create (){
		return adoptPtr(new ObjectVector());
	}

	static PassOwnPtr<ObjectVector> create (PassOwnPtr<T> buffer){
				return adoptPtr(new ObjectVector(buffer));
	}

	virtual ~ObjectVector(){}

	void add(PassOwnPtr<T> buffer){
		m_buffers.append(buffer);
	}

	T* next(){
		if( m_index < m_buffers.size()){
			T* retval = m_buffers.at(m_index++).get();
			return retval;
		}
		return 0;
	}

	T* last(){
		return m_buffers.at(m_buffers.size() - 1);
	}

	void clear(){
		m_buffers.clear();
		reset();
	}

	void reset(){
		m_index = 0;
	}

	bool isEmpty(){
		return m_buffers.isEmpty();
	}

	size_t size(){
		return m_buffers.size();
	}

	typename Vector< OwnPtr<T> >::iterator begin(){
		return m_buffers.begin();
	}

	typename Vector< OwnPtr<T> >::iterator end(){
		return m_buffers.end();
	}

	virtual PassRefPtr<JSONArray> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONArray> json) OVERRIDE;


protected:
	ObjectVector(){
		reset();
	}
	ObjectVector(PassOwnPtr<T> buffer){
		reset();
		m_buffers.append(buffer);
	}
	size_t m_index;
	Vector< OwnPtr<T> > m_buffers;
};

template<typename T>
class PrimativeVector : public SerializableVector {
public:
	static PassOwnPtr<PrimativeVector> create (){
		return adoptPtr(new PrimativeVector());
	}

	static PassOwnPtr<PrimativeVector> create (T num){
	    		return adoptPtr(new PrimativeVector(num));
	}

	virtual ~PrimativeVector(){}

	void add(T num){
		m_buffers.append(num);

	}

	T* next(){
		if( m_index < m_buffers.size()){
			return &(m_buffers.at(m_index++));
		}
		return 0;
	}

	T* nextPreview(){ // does not advance index
		if( m_index < m_buffers.size()){
			return &(m_buffers.at(m_index));
		}
		return 0;
	}

	T* last(){
		return &(m_buffers.at(m_buffers.size() - 1));
	}

	void clear(){
		m_buffers.clear();
		reset();
	}

	void reset(){
		m_index = 0;
	}

	bool isEmpty(){
		return m_buffers.isEmpty();
	}

	size_t size(){
		return m_buffers.size();
	}

	typename Vector<T>::iterator begin(){
		return m_buffers.begin();
	}

	typename Vector<T>::iterator end(){
		return m_buffers.end();
	}

	virtual PassRefPtr<JSONArray> serialize() OVERRIDE;
	virtual void deserialize(PassRefPtr<JSONArray> json) OVERRIDE;

protected:
	PrimativeVector(){
		reset();
	}

	PrimativeVector(T num){
		reset();
		m_buffers.append(num);
	}

	size_t m_index;
	Vector<T> m_buffers;
};

typedef PrimativeVector<bool> BoolVector;
typedef PrimativeVector<double> DoubleVector;
typedef PrimativeVector<size_t> SizeTypeVector;
typedef PrimativeVector<uint64> UInt64Vector;
typedef PrimativeVector<unsigned long long> ULLVector;

//modaldialog by Bo
//typedef ObjectVector<String> StringVector;

typedef ObjectVector<blink::WebString> WebStringVector;
typedef ObjectVector<blink::WebData> WebDataVector;
typedef ObjectVector<blink::WebVector<blink::WebString> > AvailableTypesVector;

typedef HashMap<String, KeyPressEventTargetInfo> KeyPressEventTargetInfoMap;
typedef HashMap<String, MousePressEventTargetInfo> MousePressEventTargetInfoMap;

typedef HashMap<unsigned long long, OwnPtr<BoolVector> > IsLinkVisitedMap;
typedef HashMap<String, OwnPtr<ULLVector> > VisitedLinkHashMap;
typedef HashMap<PublicKeyAndChallengeStringKey, OwnPtr<WebStringVector>, PublicKeyAndChallengeStringKey, PublicKeyAndChallengeStringKey> SignedPublicKeyAndChallengeStringMap;

//WebURLLoader data
typedef HashMap<blink::WebURLRequest, OwnPtr<RequestMapValue>, WebURLRequestHash, WebURLRequestHashTraits> RequestMap;
typedef HashMap<blink::WebURLRequest, OwnPtr<RedirectMapValue>, WebURLRequestHash, WebURLRequestHashTraits> RedirectMap;
typedef HashMap<blink::WebURLLoaderClient*, RefPtr<PendingRequestMapEntry> > PendingRequestMap;
typedef HashMap<String, V8StackTrace > StackTraceMap;
typedef HashMap<String, OwnPtr<Vector<V8StackTraceMatchingEntry> > > StackTraceMatchingMap;

//Platform::cryptographicallyRandomValues data
typedef HashMap<size_t, OwnPtr<WebDataVector> > CRVMap;

//WebClipboard data
typedef HashMap<int, OwnPtr<UInt64Vector> > ClipboardSequenceNumberMap;
//modaldialog by Bo
typedef HashMap<String, OwnPtr<BoolVector> > ModaldialogConfirmResultMap;
typedef HashMap<String, OwnPtr<WebStringVector> > ModaldialogPromptResultMap;


typedef HashMap<IsFormatAvailableKey, OwnPtr<BoolVector>, IsFormatAvailableKey, IsFormatAvailableKey> ClipboardIsFormatAvailableMap;
typedef HashMap<ReadAvailableTypesKey, OwnPtr<AvailableTypesVector>, ReadAvailableTypesKey, ReadAvailableTypesKey> ClipboardReadAvailableTypesMap;
typedef HashMap<int, OwnPtr<WebStringVector> > ClipboardReadPlainTextMap;
typedef HashMap<ReadHTMLKey, OwnPtr<WebStringVector>, ReadHTMLKey, ReadHTMLKey> ClipboardReadHTMLMap;
typedef HashMap<int, OwnPtr<WebDataVector> > ClipboardReadImageMap;
typedef HashMap<CustomDataKey, OwnPtr<WebStringVector>, CustomDataKey, CustomDataKey> ClipboardReadCustomDataMap;

//self-healing data; used to recover from replay incongruence (using "forced load")
 // FIXME: this is a very simple way of tracking redirect chains and HTML5's "silent" URL substitutions
 // (see for example this: http://spoiledmilk.com/blog/html5-changing-the-browser-url-without-refreshing-page/)
 // it does not support complex cases multiple alias chains that lead to the same final URL
 typedef HashMap<String, String> PageURLAliasMap;
 typedef HashMap<String, Vector<RefPtr<PageURLRequest> > > PageURLRequestsMap; // maps a page URL to the event and request that triggered the page to be loaded

} /* namespace WebCore */
#endif /* ForensicReplayData_h */
