/*
 * ForensicWebClipboard.cpp
 *
 *  Created on: Oct 2, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicWebClipboard.h"

namespace WebCore {

ForensicWebClipboard::ForensicWebClipboard(PassRefPtr<ForensicReplayDataStore> dataStore,
		PassRefPtr<ForensicReplayDataRecorder> dataRecorder,
		blink::Platform* platform,
		PassOwnPtr<ForensicInspectorStateWrapper> state,
		blink::WebClipboard* clipboard)
		: m_dataStore(dataStore)
		, m_dataRecorder(dataRecorder)
		, m_platform(platform)
		, m_state(state)
		, m_clipboard(clipboard)
{

}

ForensicWebClipboard::~ForensicWebClipboard() {

}

uint64 ForensicWebClipboard::sequenceNumber(blink::WebClipboard::Buffer buffer){
	uint64 retval;
	if(m_state->replaying()) {
		retval = m_dataStore->nextClipboardSequenceNumber(buffer);
	} else {
		retval = m_clipboard->sequenceNumber(buffer);
		if(m_state->recording()){
			m_dataRecorder->appendClipboardSequenceNumber(buffer, retval);
		}
	}
	return retval;
}

bool ForensicWebClipboard::isFormatAvailable(blink::WebClipboard::Format format,
		blink::WebClipboard::Buffer buffer){
	bool retval;
	if(m_state->replaying()) {
		retval = m_dataStore->nextClipboardIsFormatAvailable(format, buffer);
	} else {
		retval = m_clipboard->isFormatAvailable(format, buffer);
		if(m_state->recording()){
			m_dataRecorder->appendClipboardIsFormatAvailable(format, buffer, retval);
		}
	}
	return retval;
}

blink::WebVector<blink::WebString> ForensicWebClipboard::readAvailableTypes(
		blink::WebClipboard::Buffer buffer, bool* containsFilenames){
	blink::WebVector<blink::WebString> retval;
	if(m_state->replaying()) {
		retval = m_dataStore->nextClipboardReadAvailableTypes(buffer, containsFilenames);
	} else {
		retval = m_clipboard->readAvailableTypes(buffer, containsFilenames);
		if(m_state->recording()){
			m_dataRecorder->appendClipboardReadAvailableTypes(buffer, containsFilenames, retval);
		}
	}
	return retval;
}

blink::WebString ForensicWebClipboard::readPlainText(blink::WebClipboard::Buffer buffer){
	blink::WebString retval;
	if(m_state->replaying()) {
		retval = m_dataStore->nextClipboardReadPlainText(buffer);
	} else {
		retval = m_clipboard->readPlainText(buffer);
		if(m_state->recording()){
			m_dataRecorder->appendClipboardReadPlainText(buffer, retval);
		}
	}
	return retval;
}

blink::WebString ForensicWebClipboard::readHTML(blink::WebClipboard::Buffer buffer,
		blink::WebURL* sourceUrl, unsigned* fragmentStart, unsigned* fragmentEnd){
	blink::WebString retval;
	if(m_state->replaying()) {
		retval = m_dataStore->nextClipboardReadHTML(buffer, sourceUrl, fragmentStart, fragmentEnd);
	} else {
		retval = m_clipboard->readHTML(buffer, sourceUrl, fragmentStart, fragmentEnd);
		if(m_state->recording()){
			m_dataRecorder->appendClipboardReadHTML(buffer, sourceUrl, fragmentStart, fragmentEnd, retval);
		}
	}
	return retval;
}

blink::WebData ForensicWebClipboard::readImage(blink::WebClipboard::Buffer buffer){
	blink::WebData retval;
	if(m_state->replaying()) {
		retval = m_dataStore->nextClipboardReadImage(buffer);
	} else {
		retval = m_clipboard->readImage(buffer);
		if(m_state->recording()){
			m_dataRecorder->appendClipboardReadImage(buffer, retval);
		}
	}
	return retval;
}

blink::WebString ForensicWebClipboard::readCustomData(
		blink::WebClipboard::Buffer buffer, const blink::WebString& type){
	blink::WebString retval;
	if(m_state->replaying()) {
		retval = m_dataStore->nextClipboardReadCustomData(buffer, type);
	} else {
		retval = m_clipboard->readCustomData(buffer, type);
		if(m_state->recording()){
			m_dataRecorder->appendClipboardReadCustomData(buffer, type, retval);
		}
	}
	return retval;
}

void ForensicWebClipboard::writePlainText(const blink::WebString& plainText){
	m_clipboard->writePlainText(plainText);
}

void ForensicWebClipboard::writeHTML(const blink::WebString& htmlText,
		const blink::WebURL& sourceUrl,
		const blink::WebString& plainText,
		bool writeSmartPaste){
	m_clipboard->writeHTML(htmlText, sourceUrl, plainText, writeSmartPaste);
}

void ForensicWebClipboard::writeImage(const blink::WebImage& image,
		const blink::WebURL& sourceUrl,
		const blink::WebString& title){
	m_clipboard->writeImage(image, sourceUrl, title);

}

void ForensicWebClipboard::writeDataObject(const blink::WebDragData& data){
	m_clipboard->writeDataObject(data);
}

} /* namespace WebCore */
