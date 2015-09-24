
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicSerialization_h
#define ForensicSerialization_h

#include "config.h"
#include <v8.h>
#include "wtf/PassRefPtr.h"
#include "wtf/text/Base64.h"
#include "platform/JSONValues.h"
#include "public/platform/WebURLRequest.h"
#include "public/platform/WebURLResponse.h"
#include "public/platform/WebURLError.h"
#include "platform/CrossThreadCopier.h"
#include "platform/network/ResourceRequest.h"
#include "platform/network/ResourceResponse.h"
#include "platform/weborigin/KURL.h"
#include "core/loader/HistoryItem.h"

#include <map>
#include <vector>

namespace WebCore {

static const char* JSON_KEY = "key";
static const char* JSON_VALUE  = "value";

template<class T>
class SerializableBase
{
public:
	virtual ~SerializableBase() {}

	virtual PassRefPtr<T> serialize() = 0;
	virtual void deserialize(PassRefPtr<T> json) = 0;
};

typedef SerializableBase<JSONObject> Serializable;
typedef SerializableBase<JSONArray> SerializableVector;

class FINAL SerializationUtils
{
public:
	virtual ~SerializationUtils() {}

	static PassRefPtr<JSONObject> serialize(const blink::WebURLRequest& request){
		return SerializationUtils::serialize(request.toResourceRequest());
	}

	static PassRefPtr<JSONObject> serialize(const CrossThreadHTTPHeaderMapData& headers){
		RefPtr<JSONObject> retval = JSONObject::create();
		Vector<std::pair<String, String> >::const_iterator it;
		for(it = headers.begin(); it != headers.end(); it++){
			std::pair<String, String> header = *it;
			retval->setString(header.first, header.second);
		}
		return retval;
	}

	static PassRefPtr<JSONObject> serialize(HistoryItem& item){
		RefPtr<JSONObject> retval = JSONObject::create();
		retval->setString("urlString", item.urlString());

		RefPtr<JSONArray> docStateJSON = JSONArray::create();
		const Vector<String>& docstate = item.documentState();
		Vector<String>::const_iterator end = docstate.end();
		for(Vector<String>::const_iterator it = docstate.begin(); it != end; ++it){
			docStateJSON->pushString(*it);
		}
		retval->setArray("documentState", docStateJSON);

		retval->setString("target", item.target());

		RefPtr<JSONObject> referrerJSON = JSONObject::create();
		const Referrer& referrer =  item.referrer();
		referrerJSON->setString("referrer", referrer.referrer);
		referrerJSON->setNumber("referrerPolicy", referrer.referrerPolicy);
		retval->setObject("referrer", referrerJSON);


		RefPtr<JSONObject> scrollpointJSON = JSONObject::create();
		const IntPoint& scrollpoint = item.scrollPoint();
		scrollpointJSON->setNumber("x", scrollpoint.x());
		scrollpointJSON->setNumber("y", scrollpoint.y());
		retval->setObject("scrollPoint", scrollpointJSON);

		retval->setNumber("pageScaleFactor",item.pageScaleFactor());

		if(item.stateObject()){
			retval->setString("stateObject", item.stateObject()->toWireString());
		} else {
			retval->setString("stateObject", "");
		}


		retval->setNumber("itemSequenceNumber", item.itemSequenceNumber());
		retval->setNumber("documentSequenceNumber", item.documentSequenceNumber());

	    if(item.formData()){
	    	Vector<char> formData;
	    	item.formData()->flatten(formData);
	    	retval->setString("formData", base64Encode(formData));
	    } else {
	    	retval->setString("formData", "");
	    }

	    retval->setString("formContentType", item.formContentType());


		return retval;
	}

	static PassRefPtr<HistoryItem> deserializeHistoryItem(PassRefPtr<JSONObject> json){
		RefPtr<HistoryItem> retval = HistoryItem::create();

		String strTemp;
		double dTemp;
		int intTemp;

		bool result = json->getString("urlString", &strTemp);
		ASSERT(result);
		retval->setURLString(strTemp);

		RefPtr<JSONArray> docStateJSON = json->getArray("documentState");
		Vector<String> vecTemp;
		ASSERT(docStateJSON);
		for(size_t i = 0; i < docStateJSON->length(); i++){
			docStateJSON->get(i)->asString(&strTemp);
			vecTemp.append(strTemp);
		}
		retval->setDocumentState(vecTemp);

		result = json->getString("target", &strTemp);
		ASSERT(result);
		retval->setTarget(strTemp);


		RefPtr<JSONObject> referrerJSON = json->getObject("referrer");
		ASSERT(referrerJSON);
		referrerJSON->getString("referrer", &strTemp);
		referrerJSON->getNumber("referrerPolicy", &intTemp);
		retval->setReferrer(Referrer(strTemp, static_cast<ReferrerPolicy>(intTemp)));

		IntPoint scrollPoint;
		RefPtr<JSONObject> scrollpointJSON = json->getObject("scrollPoint");
		ASSERT(scrollpointJSON);
		result = scrollpointJSON->getNumber("x", &intTemp);
		ASSERT(result);
		scrollPoint.setX(intTemp);
		result = scrollpointJSON->getNumber("y", &intTemp);
		ASSERT(result);
		scrollPoint.setY(intTemp);
		retval->setScrollPoint(scrollPoint);

		result = json->getNumber("pageScaleFactor",&dTemp);
		ASSERT(result);
		retval->setPageScaleFactor(dTemp);

		result = json->getString("stateObject", &strTemp);
		ASSERT(result);
		if(!strTemp.isEmpty()){
			retval->setStateObject(SerializedScriptValue::createFromWire(strTemp));
		}

		result = json->getNumber("itemSequenceNumber", &dTemp);
		ASSERT(result);
		retval->setItemSequenceNumber(dTemp);

		result = json->getNumber("documentSequenceNumber", &dTemp);
		ASSERT(result);
		retval->setDocumentSequenceNumber(dTemp);

		result = json->getString("formData", &strTemp);
		ASSERT(result);
		if(!strTemp.isEmpty()){
			Vector<char> vecTemp;
			base64Decode(strTemp, vecTemp);
			retval->setFormData(FormData::create(vecTemp));
		}

		result = json->getString("formContentType", &strTemp);
		ASSERT(result);
		retval->setFormContentType(AtomicString(strTemp));

		return retval;
	}


	static PassRefPtr<JSONObject> serialize(const ResourceRequest& request){
		RefPtr<JSONObject> retval = JSONObject::create();
		OwnPtr<CrossThreadResourceRequestData> requestData = CrossThreadCopier<ResourceRequest>::copy(request);

		retval->setString("url", requestData->m_url.string());
		retval->setNumber("cachePolicy", requestData->m_cachePolicy);
		retval->setNumber("timeoutInterval", requestData->m_timeoutInterval);
		retval->setString("firstPartyForCookies", requestData->m_firstPartyForCookies.string());
		retval->setString("httpMethod", requestData->m_httpMethod);

		if(requestData->m_httpHeaders){
			retval->setObject("httpHeaders", SerializationUtils::serialize(*(requestData->m_httpHeaders.get())));
		} else {
			retval->setObject("httpHeaders", JSONObject::create());
		}

		if(requestData->m_httpBody){
			Vector<char> bodyData;
			requestData->m_httpBody->flatten(bodyData);
			retval->setString("httpBody", base64Encode(bodyData));
		} else {
			retval->setString("httpBody", "");
		}

		retval->setBoolean("allowStoredCredentials", requestData->m_allowStoredCredentials);
		retval->setBoolean("reportUploadProgress", requestData->m_reportUploadProgress);
		retval->setBoolean("hasUserGesture", requestData->m_hasUserGesture);
		retval->setBoolean("downloadToFile", requestData->m_downloadToFile);
		retval->setNumber("priority", requestData->m_priority);
		retval->setNumber("intraPriorityValue", requestData->m_intraPriorityValue);
		retval->setNumber("requestorID", requestData->m_requestorID);
		retval->setNumber("requestorProcessID", requestData->m_requestorProcessID);
		retval->setNumber("appCacheHostID", requestData->m_appCacheHostID);
		retval->setNumber("targetType", requestData->m_targetType);
		retval->setNumber("referrerPolicy", requestData->m_referrerPolicy);

		return retval;
	}

	static void deserialize(PassRefPtr<JSONObject> json, CrossThreadHTTPHeaderMapData& buf){
		bool result;
		JSONObject::const_iterator::Keys end = json->end().keys();
		for(JSONObject::const_iterator::Keys it = json->begin().keys(); it != end; ++it){
			String key = *it;
			String value;
			result = json->getString(key, &value);
			ASSERT(result);
			buf.append(std::pair<String, String>(key, value));
		}
	}

	static PassOwnPtr<ResourceRequest> deserializeRequest(PassRefPtr<JSONObject> json){
		OwnPtr<CrossThreadResourceRequestData> requestData = adoptPtr(new CrossThreadResourceRequestData());

		String strTemp;
		int intTemp;

		bool result = json->getString("url", &strTemp);
		ASSERT(result);
		requestData->m_url = KURL(ParsedURLString, strTemp);

		result = json->getNumber("cachePolicy", &intTemp);
		ASSERT(result);
		requestData->m_cachePolicy = static_cast<ResourceRequestCachePolicy>(intTemp);

		result = json->getNumber("timeoutInterval", &(requestData->m_timeoutInterval));
		ASSERT(result);

		result = json->getString("firstPartyForCookies", &strTemp);
		ASSERT(result);
		requestData->m_firstPartyForCookies = KURL(ParsedURLString, strTemp);

		result = json->getString("httpMethod", &(requestData->m_httpMethod));
		ASSERT(result);

		RefPtr<JSONObject> headersJSON = json->getObject("httpHeaders");
		ASSERT(result);
		requestData->m_httpHeaders = adoptPtr(new CrossThreadHTTPHeaderMapData());
		if(headersJSON->size() > 0){
			SerializationUtils::deserialize(headersJSON, *(requestData->m_httpHeaders.get()));
		}

		result = json->getString("httpBody", &strTemp);
		ASSERT(result);
		if(!strTemp.isEmpty()){
			Vector<char> vecTemp;
			base64Decode(strTemp, vecTemp);
			requestData->m_httpBody = FormData::create(vecTemp);
		}

		result = json->getBoolean("allowStoredCredentials", &(requestData->m_allowStoredCredentials));
		ASSERT(result);

		result = json->getBoolean("reportUploadProgress", &(requestData->m_reportUploadProgress));
		ASSERT(result);

		result = json->getBoolean("hasUserGesture", &(requestData->m_hasUserGesture));
		ASSERT(result);

		result = json->getBoolean("downloadToFile", &(requestData->m_downloadToFile));
		ASSERT(result);

		result = json->getNumber("priority", &intTemp);
		ASSERT(result);
		requestData->m_priority = static_cast<ResourceLoadPriority>(intTemp);

		result = json->getNumber("intraPriorityValue", &(requestData->m_intraPriorityValue));
		ASSERT(result);

		result = json->getNumber("requestorID", &(requestData->m_requestorID));
		ASSERT(result);

		result = json->getNumber("requestorProcessID", &(requestData->m_requestorProcessID));
		ASSERT(result);

		result = json->getNumber("appCacheHostID", &(requestData->m_appCacheHostID));
		ASSERT(result);

		result = json->getNumber("targetType", &intTemp);
		ASSERT(result);
		requestData->m_targetType = static_cast<ResourceRequest::TargetType>(intTemp);

		result = json->getNumber("referrerPolicy", &intTemp);
		ASSERT(result);
		requestData->m_referrerPolicy = static_cast<ReferrerPolicy>(intTemp);

		return ResourceRequest::adopt(requestData.release());
	}


	static PassRefPtr<JSONObject> serialize(const blink::WebURLResponse& request){
		return SerializationUtils::serialize(request.toResourceResponse());
	}

	static PassRefPtr<JSONObject> serialize(const ResourceResponse& response){
		RefPtr<JSONObject> retval = JSONObject::create();
		OwnPtr<CrossThreadResourceResponseData> responseData = CrossThreadCopier<ResourceResponse>::copy(response);

		retval->setString("url", responseData->m_url.string());
		retval->setString("mimeType", responseData->m_mimeType);
		retval->setNumber("expectedContentLength", responseData->m_expectedContentLength);
		retval->setString("textEncodingName", responseData->m_textEncodingName);
		retval->setString("suggestedFilename", responseData->m_suggestedFilename);
		retval->setNumber("httpStatusCode", responseData->m_httpStatusCode);
		retval->setString("httpStatusText", responseData->m_httpStatusText);

		if(responseData->m_httpHeaders){
			retval->setObject("httpHeaders", SerializationUtils::serialize(*(responseData->m_httpHeaders.get())));
		} else {
			retval->setObject("httpHeaders", JSONObject::create());
		}

		retval->setNumber("lastModifiedDate", responseData->m_lastModifiedDate);

		if(responseData->m_resourceLoadTiming){
			retval->setObject("resourceLoadTiming", SerializationUtils::serialize(*(responseData->m_resourceLoadTiming.get())));
		} else {
			retval->setObject("resourceLoadTiming", JSONObject::create());
		}

		retval->setString("securityInfo", String(responseData->m_securityInfo.data()));
		retval->setNumber("httpVersion", responseData->m_httpVersion);
		retval->setNumber("appCacheID", responseData->m_appCacheID);
		retval->setString("appCacheManifestURL", responseData->m_appCacheManifestURL.string());
		retval->setBoolean("isMultipartPayload", responseData->m_isMultipartPayload);
		retval->setBoolean("wasFetchedViaSPDY", responseData->m_wasFetchedViaSPDY);
		retval->setBoolean("wasNpnNegotiated", responseData->m_wasNpnNegotiated);
		retval->setBoolean("wasAlternateProtocolAvailable", responseData->m_wasAlternateProtocolAvailable);
		retval->setBoolean("wasFetchedViaProxy", responseData->m_wasFetchedViaProxy);
		retval->setNumber("responseTime", responseData->m_responseTime);
		retval->setString("remoteIPAddress", responseData->m_remoteIPAddress);
		retval->setNumber("remotePort", responseData->m_remotePort);
		retval->setString("downloadedFilePath", responseData->m_downloadedFilePath);

		//XXX this serializes just the handle, not the file itself
		if(responseData->m_downloadedFileHandle){
			retval->setObject("downloadedFileHandle", SerializationUtils::serialize(*(responseData->m_downloadedFileHandle.get())));
		} else {
			retval->setObject("downloadedFileHandle", JSONObject::create());
		}


		return retval;
	}

	static PassOwnPtr<ResourceResponse> deserializeResponse(PassRefPtr<JSONObject> json){
		OwnPtr<CrossThreadResourceResponseData> responseData = adoptPtr(new CrossThreadResourceResponseData());

		String strTemp;
		double dTemp;
		int intTemp;

		bool result = json->getString("url", &strTemp);
		ASSERT(result);
		responseData->m_url = KURL(ParsedURLString, strTemp);

		result = json->getString("mimeType", &(responseData->m_mimeType));
		ASSERT(result);

		result = json->getNumber("expectedContentLength", &dTemp);
		ASSERT(result);
		responseData->m_expectedContentLength = dTemp;

		result = json->getString("textEncodingName", &(responseData->m_textEncodingName));
		ASSERT(result);

		result = json->getString("suggestedFilename", &(responseData->m_suggestedFilename));
		ASSERT(result);

		result = json->getNumber("httpStatusCode", &(responseData->m_httpStatusCode));
		ASSERT(result);

		result = json->getString("httpStatusText", &(responseData->m_httpStatusText));
		ASSERT(result);

		RefPtr<JSONObject> headersJSON = json->getObject("httpHeaders");
		ASSERT(headersJSON);
		responseData->m_httpHeaders = adoptPtr(new CrossThreadHTTPHeaderMapData());
		if(headersJSON->size() > 0){
			SerializationUtils::deserialize(headersJSON, *(responseData->m_httpHeaders.get()));
		}

		result = json->getNumber("lastModifiedDate", &dTemp);
		ASSERT(result);
		responseData->m_lastModifiedDate = dTemp;

		RefPtr<JSONObject> timingJSON = json->getObject("resourceLoadTiming");
		ASSERT(timingJSON);
		if(timingJSON->size() > 0){
			responseData->m_resourceLoadTiming = ResourceLoadTiming::create();
			SerializationUtils::deserialize(timingJSON, *(responseData->m_resourceLoadTiming.get()));
		}

		result = json->getString("securityInfo", &strTemp);
		ASSERT(result);
		responseData->m_securityInfo = CString(strTemp.latin1());

		result = json->getNumber("httpVersion", &intTemp);
		ASSERT(result);
		responseData->m_httpVersion = static_cast<ResourceResponse::HTTPVersion>(intTemp);

		result = json->getNumber("appCacheID", &dTemp);
		ASSERT(result);
		responseData->m_appCacheID = dTemp;

		result = json->getString("appCacheManifestURL", &strTemp);
		ASSERT(result);
		responseData->m_appCacheManifestURL = KURL(ParsedURLString, strTemp);

		result = json->getBoolean("isMultipartPayload", &(responseData->m_isMultipartPayload));
		ASSERT(result);

		result = json->getBoolean("wasFetchedViaSPDY", &(responseData->m_wasFetchedViaSPDY));
		ASSERT(result);

		result = json->getBoolean("wasNpnNegotiated", &(responseData->m_wasNpnNegotiated));
		ASSERT(result);

		result = json->getBoolean("wasAlternateProtocolAvailable", &(responseData->m_wasAlternateProtocolAvailable));
		ASSERT(result);

		result = json->getBoolean("wasFetchedViaProxy", &(responseData->m_wasFetchedViaProxy));
		ASSERT(result);

		result = json->getNumber("responseTime", &(responseData->m_responseTime));
		ASSERT(result);

		result = json->getString("remoteIPAddress", &(responseData->m_remoteIPAddress));
		ASSERT(result);

		result = json->getNumber("remotePort", &(intTemp));
		ASSERT(result);
		responseData->m_remotePort = intTemp;

		result = json->getString("downloadedFilePath", &(responseData->m_downloadedFilePath));
		ASSERT(result);

		RefPtr<JSONObject> handleJSON = json->getObject("downloadedFileHandle");
		ASSERT(handleJSON);
		if(handleJSON->size() > 0){
			responseData->m_downloadedFileHandle = SerializationUtils::deserializeBlobDataHandle(handleJSON);
		}

		return ResourceResponse::adopt(responseData.release());
	}

	static PassRefPtr<JSONObject> serialize(const blink::WebURLError& error){
		RefPtr<JSONObject> retval = JSONObject::create();
		retval->setString("domain", String(error.domain.latin1().c_str()));
		retval->setBoolean("isCancellation", error.isCancellation);
		retval->setString("localizedDescription", String(error.localizedDescription.latin1().c_str()));
		retval->setNumber("reason", error.reason);
		retval->setBoolean("staleCopyInCache", error.staleCopyInCache);
		retval->setString("unreachableURL", String(error.unreachableURL.string().latin1().c_str()));
		return retval;
	}

	static void deserialize(PassRefPtr<JSONObject> json, blink::WebURLError& buf){
		String strTemp;
		bool boolTemp;
		double dTemp;

		bool result = json->getString("domain", &strTemp);
		ASSERT(result);
		buf.domain = blink::WebString(strTemp);

		result = json->getBoolean("isCancellation", &boolTemp);
		ASSERT(result);
		buf.isCancellation = boolTemp;

		result = json->getString("localizedDescription", &strTemp);
		ASSERT(result);
		buf.localizedDescription = blink::WebString(strTemp);

		result = json->getNumber("reason", &dTemp);
		ASSERT(result);
		buf.reason = dTemp;

		result = json->getBoolean("staleCopyInCache", &boolTemp);
		ASSERT(result);
		buf.staleCopyInCache = boolTemp;

		result = json->getString("unreachableURL", &strTemp);
		ASSERT(result);
		buf.unreachableURL = blink::WebURL(KURL(ParsedURLString, strTemp));
	}

	static PassRefPtr<JSONObject> serialize(const ResourceLoadTiming& timing){
		RefPtr<JSONObject> retval = JSONObject::create();
		retval->setNumber("requestTime", timing.requestTime);
		retval->setNumber("proxyStart",timing.proxyStart);
		retval->setNumber("proxyEnd",timing.proxyEnd);
		retval->setNumber("dnsStart",timing.dnsStart);
		retval->setNumber("dnsEnd",timing.dnsEnd);
		retval->setNumber("connectStart",timing.connectStart);
		retval->setNumber("connectEnd",timing.connectEnd);
		retval->setNumber("sendStart",timing.sendStart);
		retval->setNumber("sendEnd",timing.sendEnd);
		retval->setNumber("receiveHeadersEnd",timing.receiveHeadersEnd);
		retval->setNumber("sslStart",timing.sslStart);
		retval->setNumber("sslEnd",timing.sslEnd);
		return retval;
	}

	static void deserialize(PassRefPtr<JSONObject> json, ResourceLoadTiming& timing){
		bool result = json->getNumber("requestTime", &(timing.requestTime));
		ASSERT(result);

		result = json->getNumber("proxyStart",&(timing.proxyStart));
		ASSERT(result);

		result = json->getNumber("proxyEnd",&(timing.proxyEnd));
		ASSERT(result);

		result = json->getNumber("dnsStart",&(timing.dnsStart));
		ASSERT(result);

		result = json->getNumber("dnsEnd",&(timing.dnsEnd));
		ASSERT(result);

		result = json->getNumber("connectStart",&(timing.connectStart));
		ASSERT(result);

		result = json->getNumber("connectEnd",&(timing.connectEnd));
		ASSERT(result);

		result = json->getNumber("sendStart",&(timing.sendStart));
		ASSERT(result);

		result = json->getNumber("sendEnd",&(timing.sendEnd));
		ASSERT(result);

		result = json->getNumber("receiveHeadersEnd",&(timing.receiveHeadersEnd));
		ASSERT(result);

		result = json->getNumber("sslStart",&(timing.sslStart));
		ASSERT(result);

		result = json->getNumber("sslEnd",&(timing.sslEnd));
		ASSERT(result);
	}


	static PassRefPtr<JSONObject> serialize(BlobDataHandle& handle){
			RefPtr<JSONObject> retval = JSONObject::create();
			retval->setNumber("size", handle.size());
			retval->setString("uuid", handle.uuid());
			retval->setString("type", handle.type());
			return retval;
	}

	static PassRefPtr<BlobDataHandle> deserializeBlobDataHandle(PassRefPtr<JSONObject> json){
		double size;
		String uuid, type;
		bool result = json->getNumber("size", &size);
		ASSERT(result);

		result = json->getString("uuid",&uuid);
		ASSERT(result);

		result = json->getString("type",&type);
		ASSERT(result);

		return BlobDataHandle::create(uuid, type, size);
	}

	static PassRefPtr<JSONObject> serialize(v8::PlatformInstrumentationData& data){
			RefPtr<JSONObject> retval = JSONObject::create();

			retval->setArray("currentTimeVals", serialize(data.currentTimeVals));
			retval->setArray("mathRandomVals", serialize(data.mathRandomVals));

			return retval;
	}

	static PassRefPtr<JSONArray> serialize(const std::map<std::string, std::vector<double> >& src){
		RefPtr<JSONArray> retval = JSONArray::create();

		std::map<std::string, std::vector<double> >::const_iterator end = src.end();
		for(std::map<std::string, std::vector<double> >::const_iterator it = src.begin(); it != end; ++it){
			RefPtr<JSONObject> mapEntry = JSONObject::create();
			mapEntry->setString(JSON_KEY, String(it->first.c_str()));
			mapEntry->setArray(JSON_VALUE, serialize(it->second));
			retval->pushObject(mapEntry);

		}

		return retval;
	}

	static PassRefPtr<JSONArray> serialize(const std::vector<double>& src){
		RefPtr<JSONArray> retval = JSONArray::create();

		std::vector<double>::const_iterator end = src.end();
		for(std::vector<double>::const_iterator it = src.begin(); it != end; it++){
			retval->pushNumber(*it);
		}

		return retval;
	}

	static void deserialize(PassRefPtr<JSONObject> json, v8::PlatformInstrumentationData& data){
		RefPtr<JSONArray> mapEntries = json->getArray("currentTimeVals");
		if (mapEntries){
			deserialize(mapEntries, data.currentTimeVals);
		}

		mapEntries = json->getArray("mathRandomVals");
		if (mapEntries){
			deserialize(mapEntries, data.mathRandomVals);
		}
	}

	static void deserialize(PassRefPtr<JSONArray> json,  std::map<std::string, std::vector<double> >& dest){
		String temp;
		bool result;

		for(size_t i = 0; i < json->length(); i++){
			RefPtr<JSONObject> mapEntry = json->get(i)->asObject();
			ASSERT(mapEntry);

			result = mapEntry->getString(JSON_KEY, &temp);
			ASSERT(result);

			RefPtr<JSONArray> mapValues = mapEntry->getArray(JSON_VALUE);
			ASSERT(mapValues);

			std::vector<double> buf;
			deserialize(mapValues, buf);

			dest.insert(std::pair<std::string, std::vector<double> >(std::string(temp.latin1().data()), buf));
		}
	}

	static void deserialize(PassRefPtr<JSONArray> json, std::vector<double>& dest){
		double temp;
		bool result;

		for(size_t i = 0; i < json->length(); i++){
			result = json->get(i)->asNumber(&temp);
			ASSERT(result);
			dest.push_back(temp);
		}
	}
};

class FINAL Utils
{
public:
	virtual ~Utils() {}

	static size_t size(const blink::WebURLError& error){
		return error.domain.length()
			+ sizeof(error.isCancellation)
			+ error.localizedDescription.length()
			+ sizeof(error.reason)
			+ error.unreachableURL.string().length();
	}
};

}


#endif /* ForensicSerialization_h */
