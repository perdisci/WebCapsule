
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicWebURLLoader_h
#define ForensicWebURLLoader_h

#include "public/platform/WebURLLoader.h"
#include "public/platform/WebURLLoaderClient.h"
#include "core/inspector/ForensicReplayDataStore.h"
#include "core/inspector/ForensicReplayDataRecorder.h"
#include "core/inspector/ForensicInspectorStateWrapper.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefPtr.h"
#include "wtf/PassOwnPtr.h"

namespace blink{
class Platform;
class WebData;
class WebURLRequest;
class WebURLResponse;
struct WebURLError;
}

namespace WebCore {
class InspectorForensicsAgent;

class ForensicWebURLLoader : public blink::WebURLLoader, protected blink::WebURLLoaderClient {
public:
	ForensicWebURLLoader(PassRefPtr<ForensicReplayDataStore>, blink::Platform*, PassOwnPtr<ForensicInspectorStateWrapper>); // for replay
	ForensicWebURLLoader(PassRefPtr<ForensicReplayDataRecorder>, blink::Platform*, PassOwnPtr<ForensicInspectorStateWrapper>, blink::WebURLLoader*); // for recording
	virtual ~ForensicWebURLLoader() {};

	//WebURLLoader API
    virtual void loadSynchronously(const blink::WebURLRequest&,
    		blink::WebURLResponse&, blink::WebURLError&, blink::WebData&) OVERRIDE FINAL;
    virtual void loadAsynchronously(const blink::WebURLRequest&,
    		blink::WebURLLoaderClient*) OVERRIDE FINAL;
    virtual void cancel() OVERRIDE FINAL;
    virtual void setDefersLoading(bool) OVERRIDE FINAL;

    //WebURLLoaderClient API
    virtual void willSendRequest(blink::WebURLLoader*, blink::WebURLRequest&, const blink::WebURLResponse& redirectResponse) OVERRIDE FINAL;
    virtual void didSendData(blink::WebURLLoader*, unsigned long long bytesSent, unsigned long long totalBytesToBeSent) OVERRIDE FINAL;
    virtual void didReceiveResponse(blink::WebURLLoader*, const blink::WebURLResponse&) OVERRIDE FINAL;
    virtual void didReceiveData(blink::WebURLLoader*, const char*, int, int encodedDataLength) OVERRIDE FINAL;
    virtual void didReceiveCachedMetadata(blink::WebURLLoader*, const char* data, int length) OVERRIDE FINAL;
    virtual void didFinishLoading(blink::WebURLLoader*, double finishTime, int64 encodedDataLength) OVERRIDE FINAL;
    virtual void didFail(blink::WebURLLoader*, const blink::WebURLError&) OVERRIDE FINAL;
    virtual void didDownloadData(blink::WebURLLoader*, int, int) OVERRIDE FINAL;

private:
    void loadSynchronouslyRecording(const blink::WebURLRequest&, double,
        		blink::WebURLResponse&, blink::WebURLError&, blink::WebData&);
    void loadSynchronouslyReplay(const blink::WebURLRequest&,
            		blink::WebURLResponse&, blink::WebURLError&, blink::WebData&);
    void loadAsynchronouslyRecording(const blink::WebURLRequest&);
    void loadAsynchronouslyReplay(const blink::WebURLRequest&);


    void handleMissingSynchronousRequest(const blink::WebURLRequest& request, blink::WebURLResponse& response, blink::WebURLError& error, blink::WebData& data);
    void handleMissingAsynchronousRequest(const blink::WebURLRequest&, blink::WebURLLoaderClient*);
    blink::WebURLRequest& createRedirectEvents(const blink::WebURLRequest&, Vector<RefPtr<ForensicEvent> >&);

    void initMissingSynchronousRequest();
    blink::WebURLResponse m_missingSynchronousResponse;
    String m_missingDataString;

    RefPtr<ForensicReplayDataStore> m_dataStore;
    RefPtr<ForensicReplayDataRecorder> m_dataRecorder;
    blink::Platform* m_platform;
    OwnPtr<ForensicInspectorStateWrapper> m_state;
    OwnPtr<blink::WebURLLoader> m_loader;
    blink::WebURLLoaderClient* m_client;
    OwnPtr<blink::WebURLResponse> m_missingResponse;
    Vector<RefPtr<ForensicEvent> > m_events;

    unsigned int m_reqID;
};

} /* namespace WebCore */
#endif /* ForensicWebURLLoader_h */
