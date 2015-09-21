/*
 * ForensicInspectorStateWrapper.cpp
 *
 *  Created on: Sep 12, 2014
 *      Author: cjneasbi
 */

#include "config.h"
#include "core/inspector/ForensicInspectorStateWrapper.h"
#include "core/inspector/InspectorForensicsAgent.h"

namespace WebCore {

PassOwnPtr<ForensicInspectorStateWrapper> ForensicInspectorStateWrapper::create(InspectorState* state){
	return adoptPtr(new ForensicInspectorStateWrapper(state));
}

PassOwnPtr<ForensicInspectorStateWrapper> ForensicInspectorStateWrapper::create(ForensicInspectorStateWrapper& other){
	return adoptPtr(new ForensicInspectorStateWrapper(other));
}

ForensicInspectorStateWrapper::ForensicInspectorStateWrapper(InspectorState* state)
	: m_state(state)
{

}

ForensicInspectorStateWrapper::~ForensicInspectorStateWrapper() {

}

bool ForensicInspectorStateWrapper::recording(){
	return m_state->getBoolean(ForensicsAgentState::forensicsAgentRecording);
}

bool ForensicInspectorStateWrapper::recording(long stateId){
	return recording() && checkStateId(stateId);
}

bool ForensicInspectorStateWrapper::replaying(){
	return m_state->getBoolean(ForensicsAgentState::forensicsAgentReplaying);
}

bool ForensicInspectorStateWrapper::replaying(long stateId){
	return replaying() && checkStateId(stateId);
}

long ForensicInspectorStateWrapper::stateId(){
	return m_state->getLong(ForensicsAgentState::forensicsAgentStateID, -1);
}

bool ForensicInspectorStateWrapper::checkStateId(long id){
	return id == stateId();
}

} /* namespace WebCore */
