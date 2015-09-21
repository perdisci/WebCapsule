/*
 * WebURLRequestHash.h
 *
 *  Created on: Jun 4, 2014
 *      Author: cjneasbi
 */

#ifndef WebURLRequestHash_h
#define WebURLRequestHash_h

#include "wtf/HashTraits.h"
#include "public/platform/WebURLRequest.h"
#include "public/platform/WebHTTPBody.h"
#include "public/platform/WebString.h"
#include "public/platform/WebURL.h"
#include "public/platform/WebHTTPHeaderVisitor.h"
#include "wtf/text/StringBuilder.h"
#include "wtf/text/StringHash.h"
#include "wtf/Vector.h"
#include "platform/Logging.h"

namespace WebCore {

struct WebURLRequestHash {

	class HeaderHashVisitor : public blink::WebHTTPHeaderVisitor {
	public:
		HeaderHashVisitor()
		{
		}
		~HeaderHashVisitor()
		{
		}
		void visitHeader(const blink::WebString& name, const blink::WebString& value){
			m_headerData.append(std::pair<std::string, std::string>(name.utf8(), value.utf8()));
		}

		String toString(){
			StringBuilder builder;
			if(m_headerData.size() > 0){
				std::sort(m_headerData.begin(), m_headerData.end());
				for(Vector<std::pair<std::string, std::string> >::const_iterator it = m_headerData.begin(); it != m_headerData.end() ; it++){
					builder.append(it->first.c_str());
					builder.append(": ");
					builder.append(it->second.c_str());
					builder.append("\n");
				}
			} else {
				builder.append("");
			}
			return builder.toString();
		}

		unsigned hash(){
			StringBuilder builder;
			if(m_headerData.size() > 0){
				std::sort(m_headerData.begin(), m_headerData.end());
				for(Vector<std::pair<std::string, std::string> >::const_iterator it = m_headerData.begin(); it != m_headerData.end() ; it++){
						//referrer header value is based upon the document currently opened in the window.
						if(it->first.compare("Referer") != 0){ //TODO probably need to reset the referrer correctly instead of ignoring it
							builder.append(it->first.c_str());
							builder.append(it->second.c_str());
						}
				}
			} else {
				builder.append("");
			}
			return StringHash::hash(builder.toString());
		}

	private:
		Vector<std::pair<std::string, std::string> > m_headerData;
	};


    static void toString(const blink::WebURLRequest& object, String& out) {
    	StringBuilder builder;
    	blink::WebURL url = object.url();
    	builder.append("URL: ");
    	if(!url.isNull()){
    		builder.append(String(url.string()));
    	}
    	builder.append("\n");

    	builder.append("Method: ");
    	blink::WebString httpMethod = object.httpMethod();
    	if(!httpMethod.isNull()){
    		builder.append(httpMethod.latin1().c_str());
    	}
    	builder.append("\n");

    	builder.append("Cache Policy: ");
    	builder.appendNumber(object.cachePolicy());
    	builder.append("\n");

    	builder.append("Allow Stored Credentials: ");
    	builder.appendNumber(object.allowStoredCredentials() ? 1 : 0);
    	builder.append("\n");

    	builder.append("Report Upload Progress: ");
    	builder.appendNumber(object.reportUploadProgress() ? 1 : 0);
    	builder.append("\n");

    	builder.append("Report Load Timing: ");
    	builder.appendNumber(object.reportLoadTiming() ? 1 : 0);
    	builder.append("\n");

    	builder.append("Report Raw Headers: ");
    	builder.appendNumber(object.reportRawHeaders() ? 1 : 0);
    	builder.append("\n");

    	builder.append("Has User Gesture: ");
    	builder.appendNumber(object.hasUserGesture() ? 1 : 0);
    	builder.append("\n");

    	builder.append("Download To File: ");
    	builder.appendNumber(object.downloadToFile() ? 1 : 0);
    	builder.append("\n");

    	builder.append("Priority: ");
    	builder.appendNumber(object.priority());
    	builder.append("\n");

    	builder.append("Requestor ID: ");
    	builder.appendNumber(object.requestorID()); //probably don't need this for identity comparison
    	builder.append("\n");

    	builder.append("Requestor Process ID: ");
    	builder.appendNumber(object.requestorProcessID()); //probably don't need this for identity comparison
		builder.append("\n");

    	builder.append("App Cache Host ID: ");
    	builder.appendNumber(object.appCacheHostID()); //probably don't need this for identity comparison
    	builder.append("\n");
    	//RefPtr<ExtraData> m_extraData; //don't think we can copy this
    	builder.append("Target Type: ");
    	builder.appendNumber(object.targetType());
    	builder.append("\n");

    	builder.append("Referrer Policy: ");
    	builder.appendNumber(object.referrerPolicy());
    	builder.append("\n");

    	builder.append("Headers:\n");
    	HeaderHashVisitor headerVisitor;
    	object.visitHTTPHeaderFields(&headerVisitor);
    	builder.append(headerVisitor.toString());
    	builder.append("\n");


    	/*blink::WebHTTPBody httpBody = object.httpBody();
    	if(!httpBody.isNull()){
    		blink::WebHTTPBody::Element elem;
    		for(unsigned int i = 0; i < httpBody.elementCount(); i++){
    			httpBody.elementAt(i, elem);
    			builder.appendNumber(elem.type);
    			switch(elem.type){
    			case(blink::WebHTTPBody::Element::TypeData):
    					builder.append(elem.data.data(), elem.data.size());
    					break;
    			case(blink::WebHTTPBody::Element::TypeFile):
    					builder.append(elem.filePath.latin1().c_str());
    					break;
    			case(blink::WebHTTPBody::Element::TypeFileSystemURL):
    					builder.append(elem.fileSystemURL.string().latin1().c_str());
    					break;
    			case(blink::WebHTTPBody::Element::TypeBlob):
    					builder.append(elem.blobUUID.latin1().c_str());
    					break;
    			}
    		}
    	}*/

    	out = builder.toString();
    }


    static unsigned hash(const blink::WebURLRequest& object) {
    	StringBuilder builder;
    	blink::WebURL url = object.url();
    	if(!url.isNull()){
    		builder.append(String(url.string()));
    	}

    	blink::WebString httpMethod = object.httpMethod();
    	if(!httpMethod.isNull()){
    		builder.append(httpMethod.latin1().c_str());
    	}

    	/*
    	 * Cache policy is used by the underlying platform, but since we short circuit
    	 * the platform this is irrelevant to us for matching purposes.
    	 */
    	//builder.appendNumber(object.cachePolicy());
    	//builder.appendNumber(object.reportLoadTiming() ? 1 : 0); //XXX: This is not serialized via CrossThreadResourceRequestData
    	//builder.appendNumber(object.reportRawHeaders() ? 1 : 0); //XXX: This is not serialized via CrossThreadResourceRequestData
    	//builder.appendNumber(object.reportUploadProgress() ? 1 : 0); //XXX: This is not serialized via CrossThreadResourceRequestData
    	//builder.appendNumber(object.hasUserGesture() ? 1 : 0); //XXX: WORKAROUND: sometimes (though rarely) user gesture is not carried over properly during replay
		//builder.appendNumber(object.priority()); //probably unimportant for replay match
    	//builder.appendNumber(object.requestorID()); //probably don't need this for identity comparison
    	//builder.appendNumber(object.appCacheHostID()); //probably don't need this for identity comparison
    	//RefPtr<ExtraData> m_extraData; //don't think we can copy this

    	builder.appendNumber(object.downloadToFile() ? 1 : 0);
    	builder.appendNumber(object.allowStoredCredentials() ? 1 : 0);
    	builder.appendNumber(object.targetType());
    	builder.appendNumber(object.referrerPolicy());

    	HeaderHashVisitor headerVisitor;
    	object.visitHTTPHeaderFields(&headerVisitor);
    	builder.appendNumber(headerVisitor.hash());

    	//TODO sort body elements first
    	blink::WebHTTPBody httpBody = object.httpBody();
    	if(!httpBody.isNull()){
    		blink::WebHTTPBody::Element elem;
    		for(unsigned int i = 0; i < httpBody.elementCount(); i++){
    			httpBody.elementAt(i, elem);
    			builder.appendNumber(elem.type);
    			switch(elem.type){
    			case(blink::WebHTTPBody::Element::TypeData):
    					builder.append(elem.data.data(), elem.data.size());
    					break;
    			case(blink::WebHTTPBody::Element::TypeFile):
    					builder.append(elem.filePath.latin1().c_str());
    					break;
    			case(blink::WebHTTPBody::Element::TypeFileSystemURL):
    					builder.append(String(elem.fileSystemURL.string()));
    					break;
    			case(blink::WebHTTPBody::Element::TypeBlob):
    					builder.append(elem.blobUUID.latin1().c_str());
    					break;
    			}
    		}
    	}

    	return StringHash::hash(builder.toString());
    }

    static bool equal(const blink::WebURLRequest& a, const blink::WebURLRequest& b) { return hash(a) == hash(b); }
    static const bool safeToCompareToEmptyOrDeleted = false;
};

struct WebURLRequestHashTraits : WTF::GenericHashTraits<blink::WebURLRequest> {
	static const bool hasIsEmptyValueFunction = true;
    static blink::WebURLRequest emptyValue()
    {
    	blink::WebURLRequest value = blink::WebURLRequest();
    	value.initialize();
    	value.setAppCacheHostID(-2);
    	value.setRequestorProcessID(-2);
    	value.setRequestorID(-2);
    	return value;
    }
    static bool isEmptyValue(const blink::WebURLRequest& value)
    {
    	return value.url().isEmpty() && value.appCacheHostID() == -2 && value.requestorID() == -2
    			&& value.requestorProcessID() == -2;
    }
    static void constructDeletedValue(blink::WebURLRequest& value)
    {
    	value.reset();
    	value.initialize();
    	value.setAppCacheHostID(-1);
    	value.setRequestorProcessID(-1);
    	value.setRequestorID(-1);
    }
    static bool isDeletedValue(const blink::WebURLRequest& value)
    {
    	return value.url().isEmpty() && value.appCacheHostID() == -1 && value.requestorID() == -1
    			&& value.requestorProcessID() == -1;
    }
};

} // namespace WebCore



#endif /* WebURLRequestHash_h */
