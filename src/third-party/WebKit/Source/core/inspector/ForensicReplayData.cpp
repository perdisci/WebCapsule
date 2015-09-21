/*
 * ForensicReplayData.cpp
 *
 *  Created on: Feb 6, 2015
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicReplayData.h"

namespace WebCore {

V8StackFrame::V8StackFrame(const v8::Handle<v8::StackFrame>& frame)
	: line(frame->GetLineNumber())
	, col(frame->GetColumn())
{
	v8::String::Utf8Value fName(frame->GetFunctionName());
	v8::String::Utf8Value sName(frame->GetScriptName());

	functionName = String(*fName);
	scriptName = String(*sName);

}

V8StackFrame::V8StackFrame(PassRefPtr<JSONObject> json){
	deserialize(json);
}

V8StackFrame::~V8StackFrame(){}

size_t V8StackFrame::size(){
	return sizeof(line) + sizeof(col) + functionName.sizeInBytes() + scriptName.sizeInBytes();
}

PassRefPtr<JSONObject> V8StackFrame::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setNumber("line", line);
	retval->setNumber("col", col);
	retval->setString("functionName", functionName);
	retval->setString("scriptName", scriptName);
	return retval;
}

void V8StackFrame::deserialize(PassRefPtr<JSONObject> json){
	bool result = json->getNumber("line", &line);
	ASSERT(result);

	result = json->getNumber("col", &col);
	ASSERT(result);

	result = json->getString("functionName", &functionName);
	ASSERT(result);

	result = json->getString("scriptName", &scriptName);
	ASSERT(result);
}

V8StackTrace::~V8StackTrace(){};

PassRefPtr<JSONArray> V8StackTrace::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	for(size_t i = 0; i < size(); i++){
		retval->pushObject(at(i).serialize());
	}
	return retval;
}

void V8StackTrace::deserialize(PassRefPtr<JSONArray> json){
	clear();
	for(unsigned int i = 0; i < json->length(); i++){
		RefPtr<JSONObject> stackFrameJSON = json->get(i)->asObject();
		ASSERT(stackFrameJSON);
		append(V8StackFrame(stackFrameJSON));
	}
}


V8StackTraceMatchingEntry::V8StackTraceMatchingEntry(const blink::WebURLRequest* request, double startTime, Source source)
	: m_request(request)
	, m_startTime(startTime)
	, m_source(source)
{

}

V8StackTraceMatchingEntry::~V8StackTraceMatchingEntry(){};

const blink::WebURLRequest* V8StackTraceMatchingEntry::request() const{
	return m_request;
}

double V8StackTraceMatchingEntry::startTime() const{
	return m_startTime;
}

V8StackTraceMatchingEntry::Source V8StackTraceMatchingEntry::source() const{
	return m_source;
}

bool V8StackTraceMatchingEntry::operator<(const V8StackTraceMatchingEntry& other) const{
	return startTime() < other.startTime();
}

PassRefPtr<JSONObject> V8StackTraceMatchingEntry::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("request", String(m_request->url().string()));
	retval->setNumber("startTime", m_startTime);
	retval->setString("source", m_source == Terminal ? "Terminal" : "Redirect");
	return retval;
}

PassRefPtr<RequestMapEntry> RequestMapEntry::create(){
	return adoptRef(new RequestMapEntry());
}

PassRefPtr<RequestMapEntry> RequestMapEntry::create(blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data,
		V8StackTrace& stackTrace,
		double finishTime,
		double endTime,
		double startTime,
		int64 totalEncodedDataLength){
	return adoptRef(new RequestMapEntry(response, error, data, stackTrace, finishTime, endTime, startTime, totalEncodedDataLength));
}

RequestMapEntry::RequestMapEntry()
{
	m_response.initialize();
	m_finishTime = 0;
	m_endTime = 0;
	m_startTime = 0;
	m_totalEncodedDataLength = 0;
}

RequestMapEntry::RequestMapEntry(blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data,
		V8StackTrace& stackTrace,
		double finishTime,
		double endTime,
		double startTime,
		int64 totalEncodedDataLength)
{
	setResponse(response);
	m_error = error;
	m_data.append(data.data(), data.size());
	setStackTrace(stackTrace);
	m_finishTime = finishTime;
	m_endTime = endTime;
	m_startTime = startTime;
	m_totalEncodedDataLength = totalEncodedDataLength;
}

RequestMapEntry::~RequestMapEntry(){}

void RequestMapEntry::setResponse(const blink::WebURLResponse& response){
	m_response.assign(response);
}

double RequestMapEntry::finishTime() {
	return m_finishTime;
}

void RequestMapEntry::setFinishTime(double finishTime){
	m_finishTime = finishTime;
}

double RequestMapEntry::endTime() {
	return m_endTime;
}

void RequestMapEntry::setEndTime(double endTime){ // this should be set to the currentTime() take when didFinishLoading is called
	m_endTime = endTime;
}

double RequestMapEntry::startTime(){
	return m_startTime;
}

void RequestMapEntry::setStartTime(double startTime){
	m_startTime = startTime;
}


void RequestMapEntry::setTotalEncodedDataLength(int64 length){
	m_totalEncodedDataLength = length;
}

V8StackTrace* RequestMapEntry::stackTrace(){
	if(!m_stackTrace.isEmpty()){
		return &m_stackTrace;
	}
	return 0;
}

void RequestMapEntry::setStackTrace(const V8StackTrace& stackTrace){
	m_stackTrace.clear();
	m_stackTrace.appendVector(stackTrace);
}

void RequestMapEntry::clearStackTrace(){
	m_stackTrace.clear();
}

size_t RequestMapEntry::size(){
	size_t retval = sizeof(m_response)
			+ Utils::size(m_error)
			+ m_data.size()
			+ sizeof(m_totalEncodedDataLength)
			+ sizeof(m_startTime)
			+ sizeof(m_finishTime)
			+ sizeof(m_endTime);
	for(size_t i = 0; i < m_stackTrace.size(); i++){
		retval += m_stackTrace[i].size();
	}
	return retval;
}

PassRefPtr<JSONObject> RequestMapEntry::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setObject("response", SerializationUtils::serialize(m_response));
	retval->setObject("error", SerializationUtils::serialize(m_error));
	retval->setString("data", base64Encode(m_data));
	retval->setNumber("finishTime", m_finishTime);
	retval->setNumber("endTime", m_endTime);
	retval->setNumber("startTime", m_startTime);
	retval->setNumber("totalEncodedDataLength", m_totalEncodedDataLength);
	retval->setArray("stackTrace", m_stackTrace.serialize());

	return retval;
}

void RequestMapEntry::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> responseJSON = json->getObject("response");
	ASSERT(responseJSON);
	m_response = blink::WrappedResourceResponse(*(SerializationUtils::deserializeResponse(responseJSON).leakPtr()));

	RefPtr<JSONObject> errorJSON = json->getObject("error");
	ASSERT(errorJSON);
	SerializationUtils::deserialize(errorJSON, m_error);

	String strTemp;
	bool result = json->getString("data", &strTemp);
	ASSERT(result);
	base64Decode(strTemp, m_data);

	result = json->getNumber("finishTime", &m_finishTime);
	ASSERT(result);

	result = json->getNumber("endTime", &m_endTime);
	ASSERT(result);

	double dTemp;
	result = json->getNumber("totalEncodedDataLength", &dTemp);
	ASSERT(result);
	m_totalEncodedDataLength = dTemp;

	RefPtr<JSONArray> stackTraceJSON = json->getArray("stackTrace");
	ASSERT(stackTraceJSON);
	m_stackTrace.deserialize(stackTraceJSON);

	result = json->getNumber("startTime", &m_startTime);
	ASSERT(result);

}

PassOwnPtr<RequestMapValue> RequestMapValue::create(){
	return adoptPtr(new RequestMapValue());
}

PassOwnPtr<RequestMapValue> RequestMapValue::create(blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data,
		V8StackTrace& stackTrace,
		double finishTime,
		double endTime,
		double startTime,
		int64 totalEncodedDataLength){
	return adoptPtr(new RequestMapValue(response, error, data, stackTrace, finishTime, endTime, startTime, totalEncodedDataLength));
}

RequestMapValue::RequestMapValue(){
    reset();
}

RequestMapValue::RequestMapValue(blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data,
		V8StackTrace& stackTrace,
		double finishTime,
		double endTime,
		double startTime,
		int64 totalEncodedDataLength){
	add(response, error, data, stackTrace, finishTime, endTime, startTime, totalEncodedDataLength);
	reset();
}

RequestMapValue::~RequestMapValue(){}

void RequestMapValue::add(blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data,
		V8StackTrace& stackTrace,
		double finishTime,
		double endTime,
		double startTime,
		int64 totalEncodedDataLength){
	m_values.append(RequestMapEntry::create(response, error, data, stackTrace, finishTime, endTime, startTime, totalEncodedDataLength));
}

PassRefPtr<RequestMapEntry> RequestMapValue::next(){
	if(m_index < m_values.size()){
		PassRefPtr<RequestMapEntry> retval = m_values.at(m_index++);
		return retval;
	}
	return PassRefPtr<RequestMapEntry>();
}

PassRefPtr<RequestMapEntry> RequestMapValue::current(){
	if(m_index < m_values.size()){
		PassRefPtr<RequestMapEntry> retval = m_values.at(m_index);
		return retval;
	}
	return PassRefPtr<RequestMapEntry>();
}

void RequestMapValue::reset(){
	m_index = 0;
}

size_t RequestMapValue::size(){
	return m_values.size();
}

Vector<RefPtr<RequestMapEntry> >& RequestMapValue::values(){
	return m_values;
}

PassRefPtr<JSONArray> RequestMapValue::serialize() OVERRIDE {
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<RefPtr<RequestMapEntry> >::iterator it;
	for(it = m_values.begin(); it != m_values.end(); ++it){
		retval->pushObject((*it)->serialize());
	}
	return retval;
}

void RequestMapValue::deserialize(PassRefPtr<JSONArray> json) OVERRIDE {
	if(!m_values.isEmpty()){
		m_values.clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		RefPtr<JSONObject> requestJSON = json->get(i)->asObject();
		ASSERT(requestJSON);
		RefPtr<RequestMapEntry> entry = RequestMapEntry::create();
		entry->deserialize(requestJSON);
		m_values.append(entry);
	}
}


PassRefPtr<PendingRequestMapEntry> PendingRequestMapEntry::create(){
	return adoptRef(new PendingRequestMapEntry());
}

PassRefPtr<PendingRequestMapEntry> PendingRequestMapEntry::create(blink::WebURLRequest& request){
	return adoptRef(new PendingRequestMapEntry(request));
}

PassRefPtr<PendingRequestMapEntry> PendingRequestMapEntry::create(blink::WebURLRequest& request,
		blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data,
		V8StackTrace& stackTrace){
	return adoptRef(new PendingRequestMapEntry(request, response, error, data, stackTrace));
}

PendingRequestMapEntry::PendingRequestMapEntry(){
	m_request.initialize();
	m_mapEntry = RequestMapEntry::create();
}

PendingRequestMapEntry::PendingRequestMapEntry(blink::WebURLRequest& request){
	setRequest(request);
	m_mapEntry = RequestMapEntry::create();
}

PendingRequestMapEntry::PendingRequestMapEntry(blink::WebURLRequest& request,
		blink::WebURLResponse& response,
		blink::WebURLError& error,
		blink::WebData& data,
		V8StackTrace& stackTrace){
	m_mapEntry = RequestMapEntry::create(response, error, data, stackTrace);
	setRequest(request);
	//m_request = request;
}

PendingRequestMapEntry::~PendingRequestMapEntry(){}

size_t PendingRequestMapEntry::size(){
	return sizeof(m_request) + m_mapEntry->size();
}

void PendingRequestMapEntry::setRequest(const blink::WebURLRequest& request){
	m_request.assign(request);
}

PassRefPtr<RedirectEntry> RedirectEntry::create(){
	return adoptRef(new RedirectEntry());
}

PassRefPtr<RedirectEntry> RedirectEntry::create(
		blink::WebURLRequest& request,
		const blink::WebURLResponse& response,
		const V8StackTrace& stackTrace,
		double startTime){
	return adoptRef(new RedirectEntry(request, response, stackTrace, startTime));
}

RedirectEntry::RedirectEntry()
	:  m_startTime(0)
{
}

RedirectEntry::RedirectEntry(
		const blink::WebURLRequest& request,
		const blink::WebURLResponse& response,
		const V8StackTrace& stackTrace,
		double startTime){
	setRequest(request);
	setResponse(response);
	setStackTrace(stackTrace);
	setStartTime(startTime);
}

RedirectEntry::~RedirectEntry(){}

size_t RedirectEntry::size(){
	size_t retval = sizeof(*(m_request.get())) + sizeof(*(m_response.get())) + sizeof(m_startTime);
	for(size_t i = 0; i < m_stackTrace.size(); i++){
		retval += m_stackTrace[i].size();
	}
	return retval;
}

void RedirectEntry::setRequest(const blink::WebURLRequest& request){
	m_request = ResourceRequest::adopt(CrossThreadCopier<ResourceRequest>::copy(request.toResourceRequest()));
	//m_request.assign(request);
}
void RedirectEntry::setResponse(const blink::WebURLResponse& response){
	m_response = ResourceResponse::adopt(CrossThreadCopier<ResourceResponse>::copy(response.toResourceResponse()));
	//m_response.assign(response);
}

V8StackTrace* RedirectEntry::stackTrace(){
	if(m_stackTrace.size() > 0){
		return &m_stackTrace;
	}
	return 0;
}

void RedirectEntry::setStackTrace(const V8StackTrace& stackTrace){
	m_stackTrace.clear();
	m_stackTrace.appendVector(stackTrace);
}

void RedirectEntry::clearStackTrace(){
	m_stackTrace.clear();
}

double RedirectEntry::startTime(){
	return m_startTime;
}

void RedirectEntry::setStartTime(double startTime){
	m_startTime = startTime;
}

PassRefPtr<JSONObject> RedirectEntry::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setObject("request", SerializationUtils::serialize(*(m_request.get())));
	retval->setObject("response", SerializationUtils::serialize(*(m_response.get())));
	retval->setArray("stackTrace", m_stackTrace.serialize());
	retval->setNumber("startTime", m_startTime);

	return retval;
}

void RedirectEntry::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> responseJSON = json->getObject("response");
	ASSERT(responseJSON);
	m_response = SerializationUtils::deserializeResponse(responseJSON);

	RefPtr<JSONObject> requestJSON = json->getObject("request");
	ASSERT(requestJSON);
	m_request = SerializationUtils::deserializeRequest(requestJSON);

	RefPtr<JSONArray> stackTraceJSON = json->getArray("stackTrace");
	ASSERT(stackTraceJSON);
	m_stackTrace.deserialize(stackTraceJSON);

	bool result = json->getNumber("startTime", &m_startTime);
	ASSERT(result);
}


PassOwnPtr<RedirectMapValue> RedirectMapValue::create(){
	return adoptPtr(new RedirectMapValue());
}

PassOwnPtr<RedirectMapValue> RedirectMapValue::create(
		blink::WebURLRequest& request,
		const blink::WebURLResponse& response,
		const V8StackTrace& stackTrace,
		double startTime){
	return adoptPtr(new RedirectMapValue(request, response, stackTrace, startTime));
}

RedirectMapValue::RedirectMapValue(){
    reset();
}

RedirectMapValue::RedirectMapValue(blink::WebURLRequest& request, const blink::WebURLResponse& response, const V8StackTrace& stackTrace, double startTime){
	add(request, response, stackTrace, startTime);
	reset();
}

RedirectMapValue::~RedirectMapValue(){}

void RedirectMapValue::add(blink::WebURLRequest& request, const blink::WebURLResponse& response, const V8StackTrace& stackTrace, double startTime){
	m_values.append(RedirectEntry::create(request, response, stackTrace, startTime));
}

PassRefPtr<RedirectEntry> RedirectMapValue::next(){
	if(m_index < m_values.size()){
		PassRefPtr<RedirectEntry> retval = m_values.at(m_index++);
		return retval;
	}
	return PassRefPtr<RedirectEntry>();
}

PassRefPtr<RedirectEntry> RedirectMapValue::current(){
	if(m_index < m_values.size()){
		PassRefPtr<RedirectEntry> retval = m_values.at(m_index);
		return retval;
	}
	return PassRefPtr<RedirectEntry>();
}

void RedirectMapValue::reset(){
	m_index = 0;
}

size_t RedirectMapValue::size(){
	return m_values.size();
}

Vector<RefPtr<RedirectEntry> >& RedirectMapValue::values(){
	return m_values;
}

PassRefPtr<JSONArray> RedirectMapValue::serialize() OVERRIDE {
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<RefPtr<RedirectEntry> >::iterator it;
	for(it = m_values.begin(); it != m_values.end(); ++it){
		retval->pushObject((*it)->serialize());
	}
	return retval;
}

void RedirectMapValue::deserialize(PassRefPtr<JSONArray> json) OVERRIDE {
	if(!m_values.isEmpty()){
		m_values.clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		RefPtr<JSONObject> redirectJSON = json->get(i)->asObject();
		ASSERT(redirectJSON);
		RefPtr<RedirectEntry> pair = RedirectEntry::create();
		pair->deserialize(redirectJSON);
		m_values.append(pair);
	}
}

PassRefPtr<PageURLRequest> PageURLRequest::create(
		const blink::WebURLRequest& request,
		const String& frameName,
		const bool isMainFrame,
		const String& originURL,
		const String& eventType,
		const String& nodeName,
		const String& nodeHTML,
		const String& parentHTML) {
    return adoptRef(new PageURLRequest(request, frameName, isMainFrame, false, originURL, eventType, nodeName, nodeHTML, parentHTML));
}

PassRefPtr<PageURLRequest> PageURLRequest::createRedirect(const blink::WebURLRequest& request, const String& originURL) {
    return adoptRef(new PageURLRequest(request, String(""), false, true, originURL, String(""), String(""), String(""), String("")));
}

PassRefPtr<PageURLRequest> PageURLRequest::createEmpty() {
    return adoptRef(new PageURLRequest());
}

PageURLRequest::PageURLRequest() {
	m_isEmpty = true;
	m_isConsumed = false;
	String emptyStr = String("");
	m_frameName = emptyStr;
	m_isMainFrame = false;
	m_isRedirectRequest = false;
	m_originURL = emptyStr;
	m_eventType = emptyStr;
	m_nodeName = emptyStr;
	m_nodeHTML = emptyStr;
	m_parentHTML = emptyStr;
	m_webURLRequest.initialize();
}

PageURLRequest::PageURLRequest(
		const blink::WebURLRequest& request,
		const String& frameName,
		const bool isMainFrame,
		const bool isRedirectRequest,
		const String& originURL,
		const String& eventType,
		const String& nodeName,
		const String& nodeHTML,
		const String& parentHTML)
    : m_frameName(frameName)
    , m_isMainFrame(isMainFrame)
	, m_isRedirectRequest(isRedirectRequest)
    , m_originURL(originURL)
    , m_eventType(eventType)
    , m_nodeName(nodeName)
    , m_nodeHTML(nodeHTML)
    , m_parentHTML(parentHTML)
	, m_isEmpty(false)
	, m_isConsumed(false)
{
    copyWebURLRequest(request);
}

PageURLRequest::~PageURLRequest() {}

PageURLRequest& PageURLRequest::operator=(const PageURLRequest& r) {
    m_frameName = r.m_frameName;
    m_isMainFrame = r.m_isMainFrame;
    m_isRedirectRequest = r.m_isRedirectRequest;
    m_originURL = r.m_originURL;
    m_eventType = r.m_eventType;
    m_nodeName = r.m_nodeName;
    m_nodeHTML = r.m_nodeHTML;
    m_parentHTML = r.m_parentHTML;
    m_webURLRequest = r.m_webURLRequest; // FIXME: it may be better to deep-copy the entire request, to be safe...
    m_isEmpty = r.m_isEmpty;
    m_isConsumed = r.m_isConsumed;

    return(*this);
}

bool PageURLRequest::isEmpty() {
	return m_isEmpty;
}

bool PageURLRequest::isConsumed() {
	return m_isConsumed;
}

void PageURLRequest::setIsConsumed(bool consumed) {
	m_isConsumed = consumed;
}

blink::WebURLRequest& PageURLRequest::webURLRequest() {
    return m_webURLRequest;
}

/*
	blink::WebURLRequest m_webURLRequest; // request to be loaded into the frame

 */

size_t PageURLRequest::size(){
	size_t retval = m_frameName.sizeInBytes()
			+ sizeof(m_isMainFrame)
			+ sizeof(m_isRedirectRequest)
			+ m_originURL.sizeInBytes()
			+ m_eventType.sizeInBytes()
			+ m_nodeName.sizeInBytes()
			+ m_nodeHTML.sizeInBytes()
			+ m_parentHTML.sizeInBytes()
			+ sizeof(m_isEmpty)
			+ sizeof(m_isConsumed)
			+ sizeof(m_webURLRequest); //TODO replace with a more accurate measurement

	return retval;
}

PassRefPtr<JSONObject> PageURLRequest::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setObject("webURLRequest", SerializationUtils::serialize(m_webURLRequest));
	retval->setString("frameName", m_frameName);
	retval->setBoolean("isMainFrame", m_isMainFrame);
	retval->setBoolean("isRedirectRequest", m_isRedirectRequest);
	retval->setString("originURL", m_originURL);
	retval->setString("eventType", m_eventType);
	retval->setString("nodeName", m_nodeName);
	retval->setString("nodeHTML", m_nodeHTML);
	retval->setString("parentHTML", m_parentHTML);
	retval->setBoolean("isEmpty", m_isEmpty);

	return retval;
}

void PageURLRequest::deserialize(PassRefPtr<JSONObject> json){
	bool result;
	RefPtr<JSONObject> requestJSON = json->getObject("webURLRequest");
	ASSERT(requestJSON);
	m_webURLRequest = blink::WrappedResourceRequest(*(SerializationUtils::deserializeRequest(requestJSON).leakPtr()));

	result = json->getString("frameName", &m_frameName);
	ASSERT(result);

	result = json->getBoolean("isMainFrame", &m_isMainFrame);
	ASSERT(result);

	result = json->getBoolean("isRedirectRequest", &m_isRedirectRequest);
	ASSERT(result);

	result = json->getString("originURL", &m_originURL);
	ASSERT(result);

	result = json->getString("eventType", &m_eventType);
	ASSERT(result);

	result = json->getString("nodeName", &m_nodeName);
	ASSERT(result);

	result = json->getString("nodeHTML", &m_nodeHTML);
	ASSERT(result);

	result = json->getString("parentHTML", &m_parentHTML);
	ASSERT(result);

	result = json->getBoolean("isEmpty", &m_isEmpty);
	ASSERT(result);
}

void PageURLRequest::copyWebURLRequest(const blink::WebURLRequest& urlRequest) {

    m_webURLRequest.initialize();

    // first, we copy the entire ResourceRequest
    ResourceRequest& rr = m_webURLRequest.toMutableResourceRequest();
    const ResourceRequest& request = urlRequest.toResourceRequest();

    // OwnPtr<ResourceRequest> resourceRequest = ResourceRequest::adopt(request.toResourceRequest().copyData());
    // need to find a way to copy resourceRequest into rr!!!
    // since we cannot find an easy way to move the entire ResourceRequest data into rr, we need to do it "manually"

    rr.setURL(request.url().copy());
    rr.setCachePolicy(request.cachePolicy());
    rr.setTimeoutInterval(request.timeoutInterval());
    rr.setFirstPartyForCookies(request.firstPartyForCookies().copy());
    rr.setHTTPMethod(AtomicString(request.httpMethod().string().isolatedCopy()));

    // copy the headers from request
    const HTTPHeaderMap& headers = request.httpHeaderFields();
    HTTPHeaderMap::const_iterator endIt = headers.end();
    for (HTTPHeaderMap::const_iterator it = headers.begin(); it != endIt; ++it) {
    	WebCapsule::log(WebCapsule::DebugLogLevel, "ADDING REQUEST HEADER %s", it->key.string().latin1().data());
        rr.addHTTPHeaderField(AtomicString(it->key.string().isolatedCopy()), AtomicString(it->value.string().isolatedCopy()));
    }

    rr.setPriority(request.priority());

    if (request.httpBody())
        rr.setHTTPBody(request.httpBody()->deepCopy());

    rr.setAllowStoredCredentials(request.allowStoredCredentials());
    rr.setReportUploadProgress(request.reportUploadProgress());
    rr.setHasUserGesture(request.hasUserGesture());
    rr.setDownloadToFile(request.downloadToFile());
    rr.setRequestorID(request.requestorID());
    rr.setRequestorProcessID(request.requestorProcessID());
    rr.setAppCacheHostID(request.appCacheHostID());
    rr.setTargetType(request.targetType());

    // the referrer string should already be set by copying the HTTPHeaderMap, but we also need to set the referrerPolicy
    // and it seems there is no way to do it independently through the public interface of ResourceRequest
    rr.setHTTPReferrer(Referrer(request.httpReferrer().string().isolatedCopy(),request.referrerPolicy()));

    // there are other parameters of ResourceRequest that could be set
    // but it seems those would be unnecessary for replay purposes (at least for this version or our code)

}



KeyPressEventTargetInfo::KeyPressEventTargetInfo()
	: m_timestamp(0) { }

KeyPressEventTargetInfo::KeyPressEventTargetInfo(double t, const String& keyID, const String& keyText, const String& te, const String& pe, const String& dom)
	: m_timestamp(t)
	, m_keyID(keyID)
	, m_keyText(keyText)
	, m_targetElementHTML(te)
	, m_parentElementHTML(pe)
	, m_domTextSnapshot(dom) { }

KeyPressEventTargetInfo::KeyPressEventTargetInfo(const KeyPressEventTargetInfo& info) {
	m_timestamp = info.m_timestamp;
	m_keyID = info.m_keyID;
	m_keyText = info.m_keyText;
	m_targetElementHTML = info.m_targetElementHTML;
	m_parentElementHTML = info.m_parentElementHTML;
	m_domTextSnapshot = info.m_domTextSnapshot;
}

KeyPressEventTargetInfo& KeyPressEventTargetInfo::operator=(const KeyPressEventTargetInfo& info) {
	m_timestamp = info.m_timestamp;
	m_keyID = info.m_keyID;
	m_keyText = info.m_keyText;
	m_targetElementHTML = info.m_targetElementHTML;
	m_parentElementHTML = info.m_parentElementHTML;
	m_domTextSnapshot = info.m_domTextSnapshot;

	return *this;
}

size_t KeyPressEventTargetInfo::size(){
	size_t retval = sizeof(m_timestamp)
			+ m_keyID.sizeInBytes()
			+ m_keyText.sizeInBytes()
			+ m_targetElementHTML.sizeInBytes()
			+ m_parentElementHTML.sizeInBytes()
			+ m_domTextSnapshot.sizeInBytes();

	return retval;
}

bool KeyPressEventTargetInfo::operator==(const KeyPressEventTargetInfo& info) {
	if(m_timestamp != info.m_timestamp)
		return false;
	if(m_keyID != info.m_keyID)
		return false;
	if(m_keyText != info.m_keyText)
		return false;
	if(m_targetElementHTML != info.m_targetElementHTML)
		return false;
	if(m_parentElementHTML != info.m_parentElementHTML)
		return false;
	if(m_domTextSnapshot != info.m_domTextSnapshot)
		return false;

	return true;
}

PassRefPtr<JSONObject> KeyPressEventTargetInfo::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setNumber("timestamp", m_timestamp);
	retval->setString("keyID", m_keyID);
	retval->setString("keyText", m_keyText);
	retval->setString("targetElementHTML", m_targetElementHTML);
	retval->setString("parentElementHTML", m_parentElementHTML);
	retval->setString("domTextSnapshot", m_domTextSnapshot);
	return retval;
}

void KeyPressEventTargetInfo::deserialize(PassRefPtr<JSONObject> json){
	bool result = false;

	result = json->getNumber("timestamp", &m_timestamp);
	ASSERT(result);
	result = json->getString("keyID", &m_keyID);
	ASSERT(result);
	result = json->getString("keyText", &m_keyText);
	ASSERT(result);
	result = json->getString("targetElementHTML", &m_targetElementHTML);
	ASSERT(result);
	result = json->getString("parentElementHTML", &m_parentElementHTML);
	ASSERT(result);
	result = json->getString("domTextSnapshot", &m_domTextSnapshot);
}



MousePressEventTargetInfo::MousePressEventTargetInfo()
	: m_timestamp(0)
	, m_x(0)
	, m_y(0) { }

MousePressEventTargetInfo::MousePressEventTargetInfo(double t, int x, int y, const String& te, const String& pe, const String& dom)
	: m_timestamp(t)
	, m_x(x)
	, m_y(y)
	, m_targetElementHTML(te)
	, m_parentElementHTML(pe)
	, m_domTextSnapshot(dom) { }

MousePressEventTargetInfo::MousePressEventTargetInfo(const MousePressEventTargetInfo& info) {
	m_timestamp = info.m_timestamp;
	m_x = info.m_x;
	m_y = info.m_y;
	m_targetElementHTML = info.m_targetElementHTML;
	m_parentElementHTML = info.m_parentElementHTML;
	m_domTextSnapshot = info.m_domTextSnapshot;
}

MousePressEventTargetInfo& MousePressEventTargetInfo::operator=(const MousePressEventTargetInfo& info) {
	m_timestamp = info.m_timestamp;
	m_x = info.m_x;
	m_y = info.m_y;
	m_targetElementHTML = info.m_targetElementHTML;
	m_parentElementHTML = info.m_parentElementHTML;
	m_domTextSnapshot = info.m_domTextSnapshot;

	return *this;
}

bool MousePressEventTargetInfo::operator==(const MousePressEventTargetInfo& info) {
	if(m_timestamp != info.m_timestamp)
		return false;
	if(m_x != info.m_x)
		return false;
	if(m_y != info.m_y)
		return false;
	if(m_targetElementHTML != info.m_targetElementHTML)
		return false;
	if(m_parentElementHTML != info.m_parentElementHTML)
		return false;
	if(m_domTextSnapshot != m_domTextSnapshot)
		return false;

	return true;
}

size_t MousePressEventTargetInfo::size(){
	size_t retval = sizeof(m_timestamp)
			+ sizeof(m_x)
			+ sizeof(m_y)
			+ m_targetElementHTML.sizeInBytes()
			+ m_parentElementHTML.sizeInBytes()
			+ m_domTextSnapshot.sizeInBytes();

	return retval;
}

PassRefPtr<JSONObject> MousePressEventTargetInfo::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setNumber("timestamp", m_timestamp);
	retval->setNumber("x", m_x);
	retval->setNumber("y", m_y);
	retval->setString("targetElementHTML", m_targetElementHTML);
	retval->setString("parentElementHTML", m_parentElementHTML);
	retval->setString("domTextSnapshot", m_domTextSnapshot);
	return retval;
}

void MousePressEventTargetInfo::deserialize(PassRefPtr<JSONObject> json){
	bool result = false;

	result = json->getNumber("timestamp", &m_timestamp);
	ASSERT(result);
	result = json->getNumber("x", &m_x);
	ASSERT(result);
	result = json->getNumber("y", &m_y);
	ASSERT(result);
	result = json->getString("targetElementHTML", &m_targetElementHTML);
	ASSERT(result);
	result = json->getString("parentElementHTML", &m_parentElementHTML);
	ASSERT(result);
	result = json->getString("domTextSnapshot", &m_domTextSnapshot);
	ASSERT(result);
}



template <class T>
PassRefPtr<JSONArray> ObjectVector<T>::serialize(){
	ASSERT(false);
	return JSONArray::create();
}

template <>
PassRefPtr<JSONArray> ObjectVector<blink::WebString>::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<OwnPtr<blink::WebString> >::iterator it;
	for(it = begin(); it != end(); ++it){
		retval->pushString(String((*it)->latin1().data()));
	}
	return retval;
}

template <>
PassRefPtr<JSONArray> ObjectVector<blink::WebData>::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<OwnPtr<blink::WebData> >::iterator it2;
	for(it2 = begin(); it2 != end(); ++it2){
		retval->pushString(base64Encode((*it2)->data(), (*it2)->size()));
	}
	return retval;
}

template <>
PassRefPtr<JSONArray> ObjectVector<blink::WebVector<blink::WebString> >::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<OwnPtr<blink::WebVector<blink::WebString> > >::iterator it2;
	for(it2 = begin(); it2 != end(); ++it2){
		blink::WebVector<blink::WebString>* value = (*it2).get();
		RefPtr<JSONArray> valueJSON = JSONArray::create();
		for (size_t i = 0; i < value->size(); ++i){
			valueJSON->pushString(String((*value)[i].latin1().c_str()));
		}
		retval->pushArray(valueJSON);
	}
	return retval;
}

template <class T>
void ObjectVector<T>::deserialize(PassRefPtr<JSONArray> json){
	ASSERT(false);
}

template <>
void ObjectVector<blink::WebString>::deserialize(PassRefPtr<JSONArray> json){
	if(!isEmpty()){
		clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		String temp;
		bool result = json->get(i)->asString(&temp);
		ASSERT(result);
		add(adoptPtr(new blink::WebString(temp)));
	}
}

template <>
void ObjectVector<blink::WebData>::deserialize(PassRefPtr<JSONArray> json){
	if(!isEmpty()){
		clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		String temp;
		Vector<char> buf;
		bool result = json->get(i)->asString(&temp);
		ASSERT(result);

		result = base64Decode(temp, buf);
		ASSERT(result);

		add(adoptPtr(new blink::WebData(buf.data(), buf.size())));
	}
}

template <>
void ObjectVector<blink::WebVector<blink::WebString> >::deserialize(PassRefPtr<JSONArray> json){
	if(!isEmpty()){
		clear();
	}

	String temp;
	bool result;
	for(unsigned i = 0; i < json->length(); i++){
		RefPtr<JSONArray> arrJSON = json->get(i)->asArray();
		size_t size = arrJSON->length();
		blink::WebVector<blink::WebString>* value = new blink::WebVector<blink::WebString>(size);
		for(unsigned j = 0; j < arrJSON->length(); j++){
			result = arrJSON->get(j)->asString(&temp);
			ASSERT(result);
			(*value)[j] = blink::WebString(temp);
		}
		add(adoptPtr(value));
	}
}

template <typename T>
PassRefPtr<JSONArray> PrimativeVector<T>::serialize(){
	ASSERT(false);
	return JSONArray::create();
}

template <>
PassRefPtr<JSONArray> PrimativeVector<bool>::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<bool>::iterator it2;
	for(it2 = begin(); it2 != end(); ++it2){
		retval->pushBoolean(*it2);
	}
	return retval;
}

template <>
PassRefPtr<JSONArray> PrimativeVector<double>::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<double>::iterator it2;
	for(it2 = begin(); it2 != end(); ++it2){
		retval->pushNumber(*it2);
	}
	return retval;
}

template <>
PassRefPtr<JSONArray> PrimativeVector<long unsigned int>::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<long unsigned int>::iterator it2;
	for(it2 = begin(); it2 != end(); ++it2){
		retval->pushNumber(*it2);
	}
	return retval;
}

template <>
PassRefPtr<JSONArray> PrimativeVector<unsigned int>::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<unsigned int>::iterator it2;
	for(it2 = begin(); it2 != end(); ++it2){
		retval->pushNumber(*it2);
	}
	return retval;
}

template <>
PassRefPtr<JSONArray> PrimativeVector<unsigned long long>::serialize(){
	RefPtr<JSONArray> retval = JSONArray::create();
	Vector<unsigned long long>::iterator it2;
	for(it2 = begin(); it2 != end(); ++it2){
		retval->pushNumber(*it2);
	}
	return retval;
}

template <typename T>
void PrimativeVector<T>::deserialize(PassRefPtr<JSONArray> json){
	ASSERT(false);
}

template <>
void PrimativeVector<bool>::deserialize(PassRefPtr<JSONArray> json){
	if(!isEmpty()){
		clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		bool temp;
		bool result = json->get(i)->asBoolean(&temp);
		ASSERT(result);
		add(temp);
	}
}

template <>
void PrimativeVector<double>::deserialize(PassRefPtr<JSONArray> json){
	if(!isEmpty()){
		clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		double temp;
		bool result = json->get(i)->asNumber(&temp);
		ASSERT(result);
		add(temp);
	}
}

template <>
void PrimativeVector<long unsigned int>::deserialize(PassRefPtr<JSONArray> json){
	if(!isEmpty()){
		clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		long unsigned int temp;
		bool result = json->get(i)->asNumber(&temp);
		ASSERT(result);
		add(temp);
	}
}

template <>
void PrimativeVector<unsigned int>::deserialize(PassRefPtr<JSONArray> json){
	if(!isEmpty()){
		clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		unsigned int temp;
		bool result = json->get(i)->asNumber(&temp);
		ASSERT(result);
		add(temp);
	}
}

template <>
void PrimativeVector<unsigned long long>::deserialize(PassRefPtr<JSONArray> json){
	if(!isEmpty()){
		clear();
	}

	for(size_t i = 0; i < json->length(); i++){
		double temp;
		bool result = json->get(i)->asNumber(&temp);
		ASSERT(result);
		add(temp);
	}
}

GenericKey::GenericKey(bool empty)
	: m_deleted(false)
	, m_empty(empty)
{

}

GenericKey::GenericKey()
	: m_deleted(false)
	, m_empty(false)
{

}

GenericKey::~GenericKey(){}

size_t GenericKey::size(){
	return sizeof(m_deleted) + sizeof(m_empty);
}

bool GenericKey::operator==(const GenericKey& other) const {
	return hashVal() == other.hashVal();
}

void GenericKey::del(){
	m_deleted = true;
}

bool GenericKey::deleted() const{
	return m_deleted;
}

bool GenericKey::isEmpty() const{
	return m_empty;
}

unsigned GenericKey::hashVal() const{
	return 0;
}

PassRefPtr<JSONObject> GenericKey::serialize(){
	return JSONObject::create();
}

void GenericKey::deserialize(PassRefPtr<JSONObject> json){}

GenericKey GenericKey::emptyValue()
{
	return GenericKey(true);
}

bool GenericKey::isEmptyValue(const GenericKey& value)
{
	return value.isEmpty();
}

void GenericKey::constructDeletedValue(GenericKey& value)
{
	value.del();
}

bool GenericKey::isDeletedValue(const GenericKey& value)
{
	return value.deleted();
}

unsigned GenericKey::hash(const GenericKey& object) {
	return object.hashVal();
}

bool GenericKey::equal(const GenericKey& a, const GenericKey& b) {
	return hash(a) == hash(b);
}

IsFormatAvailableKey::IsFormatAvailableKey()
	: m_format(blink::WebClipboard::FormatPlainText)
	, m_buffer(blink::WebClipboard::BufferStandard)
{

}

IsFormatAvailableKey::IsFormatAvailableKey(blink::WebClipboard::Format format, blink::WebClipboard::Buffer buffer)
	: GenericKey()
	, m_format(format)
	, m_buffer(buffer)
{

}

IsFormatAvailableKey::~IsFormatAvailableKey(){}

size_t IsFormatAvailableKey::size(){
	return GenericKey::size() + sizeof(m_format) + sizeof(m_buffer);
}

unsigned IsFormatAvailableKey::hashVal() const{
	StringBuilder builder;
	builder.appendNumber(m_format);
	builder.append(','); //Append delimiter to avoid accidental collisions
	builder.appendNumber(m_buffer);

	return StringHash::hash(builder.toString());
}

PassRefPtr<JSONObject> IsFormatAvailableKey::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("type", "IsFormatAvailableKey");
	retval->setNumber("format", m_format);
	retval->setNumber("buffer", m_buffer);
	return retval;
}

void IsFormatAvailableKey::deserialize(PassRefPtr<JSONObject> json){
	int temp;
	bool result = json->getNumber("format", &temp);
	ASSERT(result);
	m_format = static_cast<blink::WebClipboard::Format>(temp);

	result = json->getNumber("buffer", &temp);
	ASSERT(result);
	m_buffer = static_cast<blink::WebClipboard::Buffer>(temp);
}

ReadAvailableTypesKey::ReadAvailableTypesKey()
	: m_buffer(blink::WebClipboard::BufferStandard)
	, m_containsFilenames(false)
{

}

ReadAvailableTypesKey::ReadAvailableTypesKey(blink::WebClipboard::Buffer buffer, bool containsFilenames)
	: GenericKey()
	, m_buffer(buffer)
	, m_containsFilenames(containsFilenames)
{

}

ReadAvailableTypesKey::~ReadAvailableTypesKey(){}

size_t ReadAvailableTypesKey::size(){
	return GenericKey::size() + sizeof(m_buffer) + sizeof(m_containsFilenames);
}

unsigned ReadAvailableTypesKey::hashVal() const{
	StringBuilder builder;
	builder.appendNumber(m_buffer);
	builder.append(',');
	builder.appendNumber(m_containsFilenames ? 1 : 0);

	return StringHash::hash(builder.toString());
}

PassRefPtr<JSONObject> ReadAvailableTypesKey::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("type", "ReadAvailableTypesKey");
	retval->setBoolean("containsFilesnames", m_containsFilenames);
	retval->setNumber("buffer", m_buffer);
	return retval;
}

void ReadAvailableTypesKey::deserialize(PassRefPtr<JSONObject> json){
	int temp;
	bool result = json->getNumber("buffer", &temp);
	ASSERT(result);
	m_buffer = static_cast<blink::WebClipboard::Buffer>(temp);

	result = json->getBoolean("containsFilesnames", &m_containsFilenames);
	ASSERT(result);
}

ReadHTMLKey::ReadHTMLKey()
	: m_buffer(blink::WebClipboard::BufferStandard)
	, m_fragmentStart(0)
	, m_fragmentEnd(0)
{

}

ReadHTMLKey::ReadHTMLKey(blink::WebClipboard::Buffer buffer, blink::WebURL sourceUrl, unsigned fragmentStart, unsigned fragmentEnd)
	: m_buffer(buffer)
	, m_sourceUrl(sourceUrl)
	, m_fragmentStart(fragmentStart)
	, m_fragmentEnd(fragmentEnd)
{

}

ReadHTMLKey::~ReadHTMLKey(){}

size_t ReadHTMLKey::size(){
	return GenericKey::size()
		+ sizeof(m_buffer)
		+ sizeof(m_sourceUrl)
		+ sizeof(m_fragmentStart)
		+ sizeof(m_fragmentEnd);
}

unsigned ReadHTMLKey::hashVal() const {
	StringBuilder builder;
	builder.appendNumber(m_buffer);
	builder.append(',');
	if(!m_sourceUrl.isNull()){
		builder.append(m_sourceUrl.string().latin1().c_str());
	}
	builder.append(',');
	builder.appendNumber(m_fragmentStart);
	builder.append(',');
	builder.appendNumber(m_fragmentEnd);

	return StringHash::hash(builder.toString());
}

PassRefPtr<JSONObject> ReadHTMLKey::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("type", "ReadHTMLKey");
	retval->setNumber("buffer", m_buffer);
	retval->setString("sourceUrl", String(m_sourceUrl.string().latin1().c_str()));
	retval->setNumber("fragmentStart", m_fragmentStart);
	retval->setNumber("fragmentEnd", m_fragmentEnd);
	return retval;
}

void ReadHTMLKey::deserialize(PassRefPtr<JSONObject> json){
	int temp;
	bool result = json->getNumber("buffer", &temp);
	ASSERT(result);
	m_buffer = static_cast<blink::WebClipboard::Buffer>(temp);

	result = json->getNumber("fragmentStart", &m_fragmentStart);
	ASSERT(result);

	result = json->getNumber("fragmentEnd", &m_fragmentEnd);
	ASSERT(result);

	String strTemp;
	result = json->getString("sourceUrl", &strTemp);
	ASSERT(result);

	m_sourceUrl = blink::WebURL(KURL(ParsedURLString, strTemp));

}

CustomDataKey::CustomDataKey()
	: m_buffer(blink::WebClipboard::BufferStandard)
{

}

CustomDataKey::CustomDataKey(blink::WebClipboard::Buffer buffer, blink::WebString string)
	: m_buffer(buffer)
	, m_string(string)
{

}

CustomDataKey::~CustomDataKey(){}

size_t CustomDataKey::size(){
	return GenericKey::size() + sizeof(m_buffer) + m_string.length();
}

unsigned CustomDataKey::hashVal() const{
	StringBuilder builder;
	builder.appendNumber(m_buffer);
	builder.append(',');
	if(!m_string.isNull()){
		builder.append(m_string.latin1().c_str());
	}

	return StringHash::hash(builder.toString());
}

PassRefPtr<JSONObject> CustomDataKey::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("type", "CustomDataKey");
	retval->setNumber("buffer", m_buffer);
	retval->setString("string", String(m_string.latin1().c_str()));
	return retval;
}

void CustomDataKey::deserialize(PassRefPtr<JSONObject> json){
	int temp;
	bool result = json->getNumber("buffer", &temp);
	ASSERT(result);
	m_buffer = static_cast<blink::WebClipboard::Buffer>(temp);

	String strTemp;
	result = json->getString("string", &strTemp);
	ASSERT(result);

	m_string = blink::WebString(strTemp);
}


PublicKeyAndChallengeStringKey::PublicKeyAndChallengeStringKey()
	: m_keySizeIndex(0)
{

}

PublicKeyAndChallengeStringKey::PublicKeyAndChallengeStringKey(unsigned keySizeIndex, blink::WebString challenge, blink::WebURL url)
	: m_keySizeIndex(keySizeIndex)
	, m_challenge(challenge)
	, m_url(url)
{

}

PublicKeyAndChallengeStringKey::~PublicKeyAndChallengeStringKey(){}

size_t PublicKeyAndChallengeStringKey::size(){
	return GenericKey::size()
		+ sizeof(m_keySizeIndex)
		+ m_challenge.length()
		+ m_url.string().length();
}

unsigned PublicKeyAndChallengeStringKey::hashVal() const{
	StringBuilder builder;
	builder.appendNumber(m_keySizeIndex);
	builder.append(',');
	if(!m_challenge.isNull()){
		builder.append(m_challenge.latin1().c_str());
	}
	builder.append(',');
   	if(!m_url.isNull()){
    	builder.append(m_url.string().latin1().c_str());
    }
	return StringHash::hash(builder.toString());
}

PassRefPtr<JSONObject> PublicKeyAndChallengeStringKey::serialize(){
	RefPtr<JSONObject> retval = JSONObject::create();
	retval->setString("type", "PublicKeyAndChallengeStringKey");
	retval->setNumber("keySizeIndex", m_keySizeIndex);
	retval->setString("challenge", String(m_challenge.latin1().c_str()));
	retval->setString("url", String(m_url.string().latin1().c_str()));
	return retval;
}

void PublicKeyAndChallengeStringKey::deserialize(PassRefPtr<JSONObject> json){

	String strTemp;
	bool result = json->getString("url", &strTemp);
	ASSERT(result);
	m_url = blink::WebURL(KURL(ParsedURLString, strTemp));

	result = json->getString("challenge", &strTemp);
	ASSERT(result);
	m_challenge = blink::WebString(strTemp);

	result = json->getNumber("keySizeIndex", &m_keySizeIndex);
	ASSERT(result);
}


}



