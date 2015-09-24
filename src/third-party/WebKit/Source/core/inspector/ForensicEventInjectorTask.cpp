
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#include "config.h"
#include "core/inspector/ForensicEventInjectorTask.h"
//#include "core/inspector/ForensicPlatformWrapper.h"
#include "public/platform/Platform.h"
#include "core/inspector/ForensicInspectorStateWrapper.h"
#include "platform/Logging.h"
#include "wtf/Assertions.h"
#include "core/inspector/WebCapsule.h"

namespace WebCore{

ForensicEventInjectorTask::ForensicEventInjectorTask(
		const Vector<RefPtr<ForensicEvent> >& events,
		blink::Platform* platform,
		PassOwnPtr<ForensicInspectorStateWrapper> state,
		PassRefPtr<ForensicEventInjector> injector,
        double recordStartTime,
        double replayStartTime,
		bool ignoreAgentState,
		double timeScaler)
		: m_platform(platform)
        , m_recordStartTime(recordStartTime)
        , m_replayStartTime(replayStartTime)
		, m_ignoreAgentState(ignoreAgentState)
		, m_timeScaler(timeScaler)
{
	ASSERT(timeScaler >= 0.0);

	m_injector = injector;
	m_state = state;

	//copy the pointers to avoid concurrency issues
	for(Vector<RefPtr<ForensicEvent> >::const_iterator it = events.begin(); it != events.end(); ++it){
		addEvent((*it));
	}
}

ForensicEventInjectorTask::ForensicEventInjectorTask(blink::Platform* platform,
		PassOwnPtr<ForensicInspectorStateWrapper> state,
		PassRefPtr<ForensicEventInjector> injector,
        double recordStartTime,
        double replayStartTime,
		bool ignoreAgentState,
		double timeScaler)
	    : m_platform(platform)
        , m_recordStartTime(recordStartTime)
        , m_replayStartTime(replayStartTime)
	    , m_ignoreAgentState(ignoreAgentState)
	    , m_timeScaler(timeScaler)
{
	ASSERT(timeScaler >= 0.0);

	m_injector = injector;
	m_state = state;
}

ForensicEventInjectorTask::~ForensicEventInjectorTask()
{
	if(!m_events.isEmpty()){
		m_events.clear(); //Might not be necessary
	}
}

void ForensicEventInjectorTask::addEvent(PassRefPtr<ForensicEvent> event){
	m_events.append(event);
}

void ForensicEventInjectorTask::run(){
	if(m_timeScaler > 0.0){
		runDelayed(m_timeScaler);
	} else {
		runUndelayed();
	}
}

void ForensicEventInjectorTask::runUndelayed(){
	//iterate over events injecting them
	if(!m_events.isEmpty()){
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjectorTask: Beginning injection of %lu events.", (unsigned long)m_events.size());
		std::sort(m_events.begin(), m_events.end(), eventComp);
		Vector<RefPtr<ForensicEvent> >::const_iterator it;
		for(it = m_events.begin(); it != m_events.end(); ++it){
			if(m_ignoreAgentState || m_state->replaying() /*m_agent.replaying()*/){
				WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjectorTask: Injecting event.");
				(*it)->accept(*(m_injector.get()));
			} else {
				break;
			}
		}

		//TODO set the state of the remaining events to canceled
		/*for(; it != m_events.end(); ++it){
			(*it)->cancel();
		}*/

		/*if(!m_ignoreAgentState){
			m_agent.stopReplay();
		}*/
		WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjectorTask: Event injection terminated.");
	}
}

void ForensicEventInjectorTask::runDelayed(double timeScaler) {
	if(!m_events.isEmpty()) {
		Vector<DelayedEventInjectionPair*> timedEvents;

		std::sort(m_events.begin(), m_events.end(), eventComp);
		for(Vector<RefPtr<ForensicEvent> >::const_iterator it = m_events.begin(); it != m_events.end(); ++it){
            double now = m_platform->currentTime();
            double delay = (((*it)->timestamp() - recordStartTime()) + replayStartTime() - now) * 1000 * timeScaler; // delay needs to be
            if(delay < 0)
                delay = 0;

            DelayedEventInjectionPair* de = new DelayedEventInjectionPair(ForensicInspectorStateWrapper::create(*(m_state.get())), (*it), m_injector, delay, false);

            m_platform->currentThread()->postDelayedTask(de, de->delayMs());

            WebCapsule::log(WebCapsule::DebugLogLevel,"ForensicEventInjectorTask::runDelayed : postDelayedTask : timestamp=%f, recordStartTime=%f, replayStartTime=%f, now=%f, delay=%f ",  (*it)->timestamp(), recordStartTime(), replayStartTime(), now, delay);
		}
	}
}


double ForensicEventInjectorTask::recordStartTime() {
    return m_recordStartTime;
}

double ForensicEventInjectorTask::replayStartTime() {
    return m_replayStartTime;
}


} // namespace 



