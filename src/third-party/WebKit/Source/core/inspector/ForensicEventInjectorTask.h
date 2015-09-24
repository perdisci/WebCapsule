
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef ForensicEventInjectorTask_h
#define ForensicEventInjectorTask_h

#include "core/inspector/ForensicEvent.h"
#include "core/inspector/ForensicEventInjector.h"
#include "core/inspector/InspectorForensicsAgent.h"
#include "core/inspector/ForensicInspectorStateWrapper.h"
#include "public/platform/WebThread.h"
#include "wtf/Noncopyable.h"
#include "wtf/Vector.h"
#include "wtf/PassRefPtr.h"
#include "wtf/RefPtr.h"
#include "core/inspector/WebCapsule.h"
#include "platform/Timer.h"

namespace blink{
class Platform;
}

namespace WebCore {
class ForensicEvent;

class ForensicEventInjectorTask : public blink::WebThread::Task {
	WTF_MAKE_NONCOPYABLE(ForensicEventInjectorTask);
public:
	ForensicEventInjectorTask(
			const Vector<RefPtr<ForensicEvent> >&,
			blink::Platform*,
			PassOwnPtr<ForensicInspectorStateWrapper>,
			PassRefPtr<ForensicEventInjector>,
            double recordStartTime, 
            double replayStartTime,
			bool = false,
			double = 1.0);
	ForensicEventInjectorTask(
			blink::Platform*,
			PassOwnPtr<ForensicInspectorStateWrapper>,
			PassRefPtr<ForensicEventInjector>,
            double recordStartTime,
            double replayStartTime,
			bool = false,
			double = 1.0);
	void addEvent(PassRefPtr<ForensicEvent>);
	~ForensicEventInjectorTask();
	void run();

private:
	class DelayedEventInjectionPair : public blink::WebThread::Task {
	public:
		DelayedEventInjectionPair(PassOwnPtr<ForensicInspectorStateWrapper> state,
				PassRefPtr<ForensicEvent> event,
				PassRefPtr<ForensicEventInjector> injector,
				double delayMs,
				bool ignoreAgentState)
		    : m_event(event)
			, m_injector(injector)
			, m_delayMs(delayMs)
			, m_ignoreAgentState(ignoreAgentState)
			, m_executed(false)
			, m_timer(this, &DelayedEventInjectionPair::timerFired)
		{
			m_state = state;
			m_agentStateId = m_state->stateId();
		};

		virtual ~DelayedEventInjectionPair(){};

		void run(){
			m_executed = true;
			if(m_ignoreAgentState || m_state->replaying(m_agentStateId)){
				WebCapsule::log(WebCapsule::DebugLogLevel,"DelayedEventInjectionPair: injecting event.");
				m_event->accept(*(m_injector.get()));
			} else {
				WebCapsule::log(WebCapsule::DebugLogLevel,"DelayedEventInjectionPair: event injection canceled.");
				m_event->cancel(); //for informational completeness
			}
		};

		double delayMs(){
			return m_delayMs;
		}

		void setDelayMs(long long delayMs){
			m_delayMs = delayMs;
		}

		void timerFired(Timer<DelayedEventInjectionPair>* timer){
			run();
		}

		void start(){
			double delay = delayMs() / 1000.0;
			WebCapsule::log(WebCapsule::DebugLogLevel,"DelayedEventInjectionPair: timer started, firing in %f (s).", delay);
			m_timer.startOneShot(delay, FROM_HERE);
		}

		bool isActive(){
			return m_timer.isActive();
		}

		PassRefPtr<ForensicEvent> event(){
			return m_event;
		}
	private:
		OwnPtr<ForensicInspectorStateWrapper> m_state;
		RefPtr<ForensicEvent> m_event;
		RefPtr<ForensicEventInjector> m_injector;
		double m_delayMs;
		bool m_ignoreAgentState;
		long m_agentStateId;
		bool m_executed;
		Timer<DelayedEventInjectionPair> m_timer;
	};

	RefPtr<ForensicEventInjector> m_injector;
    blink::Platform* m_platform;
    OwnPtr<ForensicInspectorStateWrapper> m_state;
	Vector<RefPtr<ForensicEvent> > m_events;

    double m_recordStartTime;
    double m_replayStartTime;

	bool m_ignoreAgentState; //take no account of InspectorForensicsAgent state
	double m_timeScaler;

    double recordStartTime();
    double replayStartTime();

	void runUndelayed();
	void runDelayed(double = 1.0);


	static struct ForensicEventComparator {
	  bool operator() (const RefPtr<ForensicEvent> i, const RefPtr<ForensicEvent> j) {
		  return i->timestamp() < j->timestamp();
		  //return const_cast<ForensicEvent*>(i)->timestamp() < const_cast<ForensicEvent*>(j)->timestamp();
	  }
	} eventComp;
};

}

#endif /* ForensicEventInjectorTask_h */
