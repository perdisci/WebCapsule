
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicLoadHistoryItemEvent.h"
#include "core/inspector/ForensicEventVisitor.h"
#include "core/inspector/WebCapsule.h"

namespace WebCore {

PassRefPtr<ForensicLoadHistoryItemEvent> ForensicLoadHistoryItemEvent::create(
		HistoryItem* item,
		HistoryLoadType historyLoadType,
		ResourceRequestCachePolicy cachePolicy,
		double timestamp) {
	return adoptRef(new ForensicLoadHistoryItemEvent(item, historyLoadType, cachePolicy, timestamp));
}

PassRefPtr<ForensicLoadHistoryItemEvent> ForensicLoadHistoryItemEvent::create(PassRefPtr<JSONObject> obj){
	return adoptRef(new ForensicLoadHistoryItemEvent(obj));
}

ForensicLoadHistoryItemEvent::ForensicLoadHistoryItemEvent(
		HistoryItem* item,
		HistoryLoadType historyLoadType,
		ResourceRequestCachePolicy cachePolicy,
		double timestamp)
	: ForensicEvent(timestamp)
	, m_historyLoadType(historyLoadType)
	, m_cachePolicy(cachePolicy)
{
    copyHistoryItem(item); // performs a deep copy of HistoryItem
}

ForensicLoadHistoryItemEvent::ForensicLoadHistoryItemEvent(PassRefPtr<JSONObject> obj){
	deserialize(obj);
}

ForensicLoadHistoryItemEvent::~ForensicLoadHistoryItemEvent()
{

}

void ForensicLoadHistoryItemEvent::accept(ForensicEventVisitor& visitor){
		visitor.visit(*this);
}

//TODO find a better way to get the size of a HistoryItem
size_t ForensicLoadHistoryItemEvent::size(){
	return sizeof(m_historyLoadType) + sizeof(m_cachePolicy) + sizeof(*(m_item.get())) + ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicLoadHistoryItemEvent::serialize(){
	RefPtr<JSONObject> retval = ForensicEvent::serialize("ForensicLoadHistoryItemEvent");
	retval->setObject("item", SerializationUtils::serialize(*(m_item.get())));
	retval->setNumber("historyLoadType", m_historyLoadType);
	retval->setNumber("cachePolicy", m_cachePolicy);
	return retval;
}


void ForensicLoadHistoryItemEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	int temp;
	bool result = guard->getNumber("historyLoadType", &temp);
	ASSERT(result);
	m_historyLoadType = static_cast<HistoryLoadType>(temp);

	result = guard->getNumber("cachePolicy", &temp);
	ASSERT(result);
	m_cachePolicy = static_cast<ResourceRequestCachePolicy>(temp);

	RefPtr<JSONObject> itemJSON = guard->getObject("item");
	ASSERT(itemJSON);
	m_item = SerializationUtils::deserializeHistoryItem(itemJSON);
}

HistoryItem* ForensicLoadHistoryItemEvent::item(){
	return m_item.get();
}

HistoryLoadType ForensicLoadHistoryItemEvent::historyLoadType() {
	return m_historyLoadType;
}

ResourceRequestCachePolicy ForensicLoadHistoryItemEvent::cachePolicy() {
	return m_cachePolicy;
}

void ForensicLoadHistoryItemEvent::copyHistoryItem(HistoryItem* item) {
    ASSERT(item);

    m_item = HistoryItem::create();

    m_item->setURLString(item->urlString().isolatedCopy());

    // FIXME: there may be a better (more efficient) way to perform a deep copy of Vector<String>
    // but for now this should do, to make sure we deep copy each String element
    Vector<String> docState;
    for(unsigned int i=0; i<item->documentState().size(); i++) {
        docState.append(item->documentState().at(i).isolatedCopy());
    }
    m_item->setDocumentState(docState);

    m_item->setTarget(item->target().isolatedCopy());
    m_item->setReferrer(Referrer(item->referrer().referrer.string().isolatedCopy(),item->referrer().referrerPolicy));
    m_item->setScrollPoint(item->scrollPoint());
    m_item->setPageScaleFactor(item->pageScaleFactor());
    m_item->setStateObject(adoptRef(item->stateObject())); // FIXME: we probably need to make a deep copy of this as well!
    m_item->setItemSequenceNumber(item->itemSequenceNumber());
    m_item->setDocumentSequenceNumber(item->documentSequenceNumber());

    if(item->formData()) // form data may not exist
        m_item->setFormData(item->formData()->deepCopy());

    String fct = item->formContentType().string().isolatedCopy(); // may be unnecessary, but we want to make sure to deep copy this string
    m_item->setFormContentType(AtomicString(fct.latin1().data()));

}

} /* namespace WebCore */
