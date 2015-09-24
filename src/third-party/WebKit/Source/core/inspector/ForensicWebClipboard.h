
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicWebClipboard_h
#define ForensicWebClipboard_h

#include "public/platform/WebClipboard.h"
#include "public/platform/WebVector.h"
#include "core/inspector/ForensicReplayDataStore.h"
#include "core/inspector/ForensicInspectorStateWrapper.h"
#include "wtf/OwnPtr.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefPtr.h"
#include "wtf/PassOwnPtr.h"

namespace blink{
class Platform;
class WebData;
class WebString;
class WebImage;
class WebDragData;
class WebURL;
}

namespace WebCore {

class ForensicWebClipboard: public blink::WebClipboard {
public:
	ForensicWebClipboard(PassRefPtr<ForensicReplayDataStore>, PassRefPtr<ForensicReplayDataRecorder>, blink::Platform*, PassOwnPtr<ForensicInspectorStateWrapper>, blink::WebClipboard*);
	virtual ~ForensicWebClipboard();

	virtual uint64 sequenceNumber(Buffer);
	virtual bool isFormatAvailable(Format, Buffer);
	virtual blink::WebVector<blink::WebString> readAvailableTypes(Buffer, bool*);
	virtual blink::WebString readPlainText(Buffer);
	virtual blink::WebString readHTML(Buffer, blink::WebURL*, unsigned*, unsigned*);
	virtual blink::WebData readImage(Buffer);
	virtual blink::WebString readCustomData(Buffer, const blink::WebString&);
	virtual void writePlainText(const blink::WebString&);
	virtual void writeHTML(const blink::WebString&, const blink::WebURL&, const blink::WebString&, bool);
	virtual void writeImage(const blink::WebImage&, const blink::WebURL&, const blink::WebString&);
	virtual void writeDataObject(const blink::WebDragData&);

private:
    RefPtr<ForensicReplayDataStore> m_dataStore;
    RefPtr<ForensicReplayDataRecorder> m_dataRecorder;
    blink::Platform* m_platform;
    OwnPtr<ForensicInspectorStateWrapper> m_state;
	WebClipboard* m_clipboard;
};

} /* namespace WebCore */
#endif /* ForensicWebClipboard_h */
