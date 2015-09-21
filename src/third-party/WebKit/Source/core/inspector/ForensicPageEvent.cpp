/*
 * ForensicPageEvent.cpp
 *
 *  Created on: Jul 14, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicPageEvent.h"

namespace WebCore {

ForensicPageEvent::ForensicPageEvent(const String& pageURL, const PageURLRequestMapLocation& mapLocation, const double timestamp)
	: ForensicEvent(timestamp)
	, m_pageURL(pageURL)
	, m_mapLocation(mapLocation)
{
}

ForensicPageEvent::ForensicPageEvent()
{
}

ForensicPageEvent::~ForensicPageEvent()
{
}

size_t ForensicPageEvent::size(){
	return m_mapLocation.currentUrlRootAlias.sizeInBytes()
			+ sizeof(m_mapLocation.pos)
			+ m_pageURL.sizeInBytes()
			+ ForensicEvent::size();
}

PassRefPtr<JSONObject> ForensicPageEvent::serialize(){
	return serialize("ForensicPageEvent");
}

PassRefPtr<JSONObject> ForensicPageEvent::serialize(const String& type){
	RefPtr<JSONObject> retval = ForensicEvent::serialize(type);
	retval->setString("pageURL", m_pageURL);
	retval->setString("currentUrlRootAlias", m_mapLocation.currentUrlRootAlias);
	retval->setNumber("pos", m_mapLocation.pos);
	return retval;
}

void ForensicPageEvent::deserialize(PassRefPtr<JSONObject> json){
	RefPtr<JSONObject> guard = json;
	ForensicEvent::deserialize(guard);

	bool result;
	result = guard->getString("pageURL", &m_pageURL);
	ASSERT(result);

	result = guard->getString("currentUrlRootAlias", &(m_mapLocation.currentUrlRootAlias));
	ASSERT(result);

	result = guard->getNumber("pos", &(m_mapLocation.pos));
	ASSERT(result);
}

const String& ForensicPageEvent::pageURL() {
	return m_pageURL;
}

const PageURLRequestMapLocation& ForensicPageEvent::pageURLRequestMapLocation(){
	return m_mapLocation;
}


} /* namespace WebCore */
