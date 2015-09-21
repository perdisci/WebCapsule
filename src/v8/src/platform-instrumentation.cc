/*
 * platform-instrumenation.cc
 *
 *  Created on: Sep 2, 2014
 *      Author: cjneasbi
 */

#include "platform-instrumentation.h"
#include "objects.h"
#include <sstream>

#define REPLAY_TIME_TOLERANCE 100 // 100ms tolerance

namespace v8 {
namespace internal {
	static PlatformInstrumentation* platformInst = 0;

	void PlatformInstrumentation::init(){
		if(!platformInst){
			platformInst = new PlatformInstrumentation();
			OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation Init\n");
		}
	}

	PlatformInstrumentation* PlatformInstrumentation::current(){
		return platformInst;
	}

	void PlatformInstrumentation::TearDown(){
		if(platformInst){
			delete platformInst;
		}
	}


	PlatformInstrumentation::PlatformInstrumentation()
		: currentState(OFF)
		, currentTimeMap(stackTraceMatcher)
		, mathRandomMap(stackTraceMatcher)
	{
		reset();
	}

	PlatformInstrumentation::~PlatformInstrumentation()
	{
		//currentTimeVals.Free();
	}

	double PlatformInstrumentation::TimeCurrentMillis(){
		if(platformInst){
			switch(platformInst->state()){
			case RECORDING:
				{
					double retval = OS::TimeCurrentMillis();
					platformInst->HandleTimeCurrentMillis(retval);
					OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::TimeCurrentMillis recorded value: %f\n", retval);
					return retval;
				}
				break;
			case REPLAYING:
				{
					double retval = platformInst->NextTimeCurrentMills();
					OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::TimeCurrentMillis replaying value: %f\n", retval);
					return retval;
				}
				break;
			}
		}
		return OS::TimeCurrentMillis();
	}

	void PlatformInstrumentation::HandleTimeCurrentMillis(double val){
		addToMap(&currentTimeMap, val);
	}


	double PlatformInstrumentation::NextTimeCurrentMills(){
		double nowRecTime = (OS::TimeCurrentMillis() - replayStartTimeMs()) + recordStartTimeMs();
		double* next = nextFromMap<double>(&currentTimeMap);
		if(!next){
			return nowRecTime;
		}

		if(*next < nowRecTime-REPLAY_TIME_TOLERANCE){// TODO(Roberto): I added the REPLAY_TIME_TOLERANCE parameter, which seems to make things work better. This needs to be tested more thoroughly.
			do{
				next = nextFromMap<double>(&currentTimeMap);
				if(!next){
					OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::currentTime : ran out of values : %f\n", nowRecTime);
					return nowRecTime;
				}
			} while(*next < nowRecTime-REPLAY_TIME_TOLERANCE);
            OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::currentTime : advanced the clock : %f\n", *next);
		} else {
			OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::currentTime : slowing down the clock : %f\n", *next);
		}

		return *next;


        /*double nowRecTime = (OS::TimeCurrentMillis() - replayStartTimeMs()) + recordStartTimeMs();
        if(currentTimeValsIndex >= currentTimeVals.length())
            return nowRecTime;

        double next = currentTimeVals[currentTimeValsIndex];
        if(next < nowRecTime) {
            do {
                currentTimeValsIndex++;
                if(currentTimeValsIndex >= currentTimeVals.length()) {
                    OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::currentTime : ran out of values : %f\n", nowRecTime);
                    return nowRecTime; // we ran out of values, and therefore we estimate what to return
                }

                next =  currentTimeVals[currentTimeValsIndex];
            } while(next < nowRecTime);
            double retval = currentTimeVals[currentTimeValsIndex];
            OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::currentTime : advanced the clock : %f\n", retval);
            return retval;
        }
        else {
            OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::currentTime : slowing down the clock : %f\n", next);
            return next;
        }*/
	}

	void PlatformInstrumentation::HandleMathRandomVals(double val){
		if(platformInst && platformInst->state() == RECORDING){
			OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::HandleMathRandomVals recorded value: %f\n", val);
			platformInst->addToMap(&(platformInst->mathRandomMap), val);
		}
	}

	double PlatformInstrumentation::NextMathRandomVals(){
		/*//XXX
		* WORKAROUND platformInst->mathRandomValsIndex < platformInst->mathRandomVals.length() should be
		* replaced with ASSERT(platformInst->mathRandomValsIndex < platformInst->mathRandomVals.length())
		* inside of the if statement
		*/
		if(platformInst && platformInst->state() == REPLAYING) {
			double* next = platformInst->nextFromMap<double>(&(platformInst->mathRandomMap));
			if(next){
			    OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::NextMathRandomVals :  replayed random value = %f\n", *next);
			    return *next;
			} else {
			    OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::NextMathRandomVals :  REPLAY INCONGRUENCE : no more random values available\n");
		        return -1;
			}

			/*if(platformInst->mathRandomValsIndex < platformInst->mathRandomVals.length()) {
                double retval = platformInst->mathRandomVals[platformInst->mathRandomValsIndex++];
			    OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::NextMathRandomVals :  replayed random value = %f\n", retval);
			    return retval;
		    } else {
			    OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::NextMathRandomVals :  REPLAY INCONGRUENCE : no more random values available\n");
		        return -1;
		    }*/
        }
        return -1;
	}

    double PlatformInstrumentation::recordStartTime() {
        return m_recordStartTime;
    }

    double PlatformInstrumentation::recordStartTimeMs() {
        return m_recordStartTime * 1000;
    }

    double PlatformInstrumentation::replayStartTime() {
        return m_replayStartTime;
    }

    double PlatformInstrumentation::replayStartTimeMs() {
        return m_replayStartTime * 1000;
    }

    void PlatformInstrumentation::setRecordStartTime(double t /* expects value in seconds */) {
        m_recordStartTime = t;
    }

    void PlatformInstrumentation::setReplayStartTime(double t /* expects value in seconds */) {
        m_replayStartTime = t;
    }

	void PlatformInstrumentation::startRecording(){
		OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::startRecording Start Recording\n");
		clear();
		currentState = RECORDING;
	}

	void PlatformInstrumentation::startReplay(){
		OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::startReplay Start Replay\n");
		reset();
		currentState = REPLAYING;
	}

	void PlatformInstrumentation::stop(){
		OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::stop Stopping\n");
		currentState = OFF;
	}


	PlatformInstrumentation::State PlatformInstrumentation::state(){
		return currentState;
	}

	void PlatformInstrumentation::clear(){
		currentTimeMap.Clear();
		mathRandomMap.Clear();
	}

	void PlatformInstrumentation::reset(){
		resetMap<double>(&currentTimeMap);
		resetMap<double>(&mathRandomMap);
	}

	PlatformInstrumentationData PlatformInstrumentation::dump(){
		PlatformInstrumentationData retval;
		copyMap(&currentTimeMap, &(retval.currentTimeVals));
		copyMap(&mathRandomMap, &(retval.mathRandomVals));
		return retval;
	}

	void PlatformInstrumentation::load(const PlatformInstrumentationData& data){
		clear();
		copyMap(&(data.currentTimeVals), &currentTimeMap);
		copyMap(&(data.mathRandomVals), &mathRandomMap);
	}

	std::string PlatformInstrumentation::getCurrentStackTrace(){
		v8::Isolate* iso = v8::Isolate::GetCurrent();
		ASSERT(iso && iso->InContext());
	    v8::HandleScope handleScope(iso);
		return stackTraceToString(v8::StackTrace::CurrentStackTrace(iso, 200));
	}

	std::string PlatformInstrumentation::stackTraceToString(Local<v8::StackTrace> trace){ // as JSON
		std::stringstream stream;
		int frameCount = trace->GetFrameCount();
		for(int i = 0; i < frameCount; i++){
			stream << stackFrameToString(trace->GetFrame(i));
			if(i < frameCount - 1){
				stream << ",\n";
			} else {
				stream << "\n";
			}
		}
		stream << "]";

		//OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::stackTraceToString StackTrace:\n%s\n", stream.str().c_str());

		return stream.str();
	}

	std::string PlatformInstrumentation::stackFrameToString(Local<v8::StackFrame> frame){ //as JSON
		std::stringstream stream;

		stream << "{\"line\": " << frame->GetLineNumber(); //zero means error
		stream << ", \"col\": " << frame->GetColumn(); //zero means error

		//TODO determine the cause of function name and script name being empty, could these be eval scripts?
		v8::String::Utf8Value funcName(frame->GetFunctionName());
		if(*funcName){
			//OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::stackFrameToString Function Name: %s\n", *funcName);
			stream << ", \"function\": \"" << *funcName << "\"";
		} else {
			OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::stackFrameToString Function Name: <empty>\n");
			stream << ", \"function\": \"\"";
		}

		v8::String::Utf8Value scriptName(frame->GetScriptName());
		if(*scriptName){
			//OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::stackFrameToString Script Name: %s\n", *scriptName);
			stream << ", \"script\": \"" << *scriptName << "\"}";
		} else {
			OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::stackFrameToString Script Name: <empty>\n");
			stream << ", \"script\": \"\"}";
		}

		return stream.str();
	}

	/*std::string PlatformInstrumentation::stackFrameToString(Local<v8::StackFrame> frame){ //as JSON
		std::stringstream stream;

		v8::String::Utf8Value funcName(frame->GetFunctionName());
		v8::String::Utf8Value scriptName(frame->GetScriptName());

		stream << "{\"line\": " << frame->GetLineNumber();
		stream << ", \"col\": " << frame->GetColumn();
		stream << ", \"function\": \"" << *funcName << "\"";
		stream << ", \"script\": \"" << *scriptName << "\"}";

		return stream.str();

	}*/

	bool PlatformInstrumentation::stackTraceMatcher(void* key1, void* key2) {
	  const std::string* name1 = reinterpret_cast<const std::string*>(key1);
	  const std::string* name2 = reinterpret_cast<const std::string*>(key2);
	  return name1->compare(*name2) == 0;
	}

	template <typename T>
	void PlatformInstrumentation::addToMap(HashMap* map, T val){
		std::string* stackTrace = new std::string(getCurrentStackTrace()); //create key which will be owned by the HashMap
		uint32_t hash = hashKey(stackTrace);
		HashMap::Entry* entry = map->Lookup(stackTrace, hash, false);
		if(entry){ // seen stack trace before
			HashMapValue<T>* values = reinterpret_cast<HashMapValue<T>* >(entry->value);
			values->Add(val);
			delete stackTrace; //new key not needed, already exists in the map
		} else {
			entry = map->Lookup(stackTrace, hash, true);
			HashMapValue<T>* values = new HashMapValue<T>();
			values->Add(val);
			entry->value = values;
		}
	}

	template <typename T>
	void PlatformInstrumentation::resetMap(HashMap* map){
		HashMap::Entry* entry = map->Start();
		while(entry){
			HashMapValue<T>* values = reinterpret_cast<HashMapValue<T>* >(entry->value);
			values->reset();
			entry = map->Next(entry);
		}
	}

	template <typename T> T*
	PlatformInstrumentation::nextFromMap(HashMap* map){
		std::string stackTrace = getCurrentStackTrace();
		uint32_t hash = hashKey(&stackTrace);
		HashMap::Entry* entry = map->Lookup(&stackTrace, hash, false);
		if(entry){
			HashMapValue<T>* values = reinterpret_cast<HashMapValue<T>* >(entry->value);
			return values->next();
		}
		OS::Print("-- WEBCAPSULE -- V8::PlatformInstrumentation::nextFromMap :  REPLAY INCONGRUENCE : have not seen stacktrace before: \n%s\n", stackTrace.c_str());
		return NULL;
	}

	template <typename K, typename V>
	void PlatformInstrumentation::copyMap(HashMap* src, std::map<K, std::vector<V> >* dest){
		HashMap::Entry* entry = src->Start();
		while(entry){
			HashMapValue<V>* values = reinterpret_cast<HashMapValue<V>* >(entry->value);
			K* key = reinterpret_cast<K*>(entry->key);
			std::vector<V> valuesCpy;
			for(int i = 0; i < values->length(); i++){
				valuesCpy.push_back((*values)[i]);
			}
			dest->insert(std::pair<K, std::vector<V> >(K(*key), valuesCpy));
			entry = src->Next(entry);
		}
	}

	template <typename K, typename V>
	void PlatformInstrumentation::copyMap(const std::map<K, std::vector<V> >* src, HashMap* dest){
		typename std::map<K, std::vector<V> >::const_iterator end = src->end();
		for (typename std::map<K, std::vector<V> >::const_iterator it = src->begin(); it != end; ++it){
			K* key = new K(it->first); //copy key
			uint32_t hash = hashKey(key); //hash key
			HashMap::Entry* entry = dest->Lookup(key, hash, true); //insert key, get value

			//copy values
			HashMapValue<V>* values = new HashMapValue<V>();
			entry->value = values;
			typename std::vector<V>::const_iterator end2 = it->second.end();
			for(typename std::vector<V>::const_iterator it2 = it->second.begin(); it2 != end2; ++it2){
				values->Add(*it2); //insert value
			}
		}
	}

	template <typename K>
	uint32_t PlatformInstrumentation::hashKey(K* key){
		ASSERT(false);
	}

	template <>
	uint32_t PlatformInstrumentation::hashKey<std::string>(std::string* key){
		return StringHasher::HashSequentialString(key->c_str(), key->length(), Isolate::Current()->heap()->HashSeed());
	}

	template <typename T>
	HashMapValue<T>::HashMapValue()
		: index(0)
	{

	}

	template <typename T>
	T* HashMapValue<T>::next(){
		if(index < this->length()){
			return &((*this)[index++]);
		}
		return 0;
	}

	template <typename T>
	void HashMapValue<T>::reset(){
		index = 0;
	}

}}



