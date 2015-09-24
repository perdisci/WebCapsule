
/*
 * Copyright (C) 2015 University of Georgia. All rights reserved.
 *
 * This file is subject to the terms and conditions defined at
 * https://github.com/perdisci/WebCapsule/blob/master/LICENSE.txt
 *
 */


#ifndef PLATFORM_INSTRUMENTATION_H_
#define PLATFORM_INSTRUMENTATION_H_

#include "v8.h"
#include "v8globals.h"
#include "platform.h"
#include "list.h"
#include "hashmap.h"
#include <string>
#include <map>
#include <vector>

namespace v8 {
namespace internal {

template <typename T>
class  HashMapValue : public List<T> {
public:
	HashMapValue();
	virtual ~HashMapValue(){};

	T* next();
	void reset();

private:
	int index;
};

class PlatformInstrumentation {
public:
	enum State {
		OFF,
		RECORDING,
		REPLAYING
	};

	static void init();
	static PlatformInstrumentation* current();
	static void TearDown();
	~PlatformInstrumentation();

	static double TimeCurrentMillis();

	void HandleTimeCurrentMillis(double);
	double NextTimeCurrentMills();


	static void HandleMathRandomVals(double);
	static double NextMathRandomVals();

    double recordStartTime();
    double replayStartTime();
    double recordStartTimeMs();
    double replayStartTimeMs();
    void setRecordStartTime(double t);
    void setReplayStartTime(double t);

	void startRecording();
	void startReplay();
	void stop();
	State state();
	void clear();
	void reset();
	PlatformInstrumentationData dump();
	void load(const PlatformInstrumentationData&);

private:
	PlatformInstrumentation();
	std::string getCurrentStackTrace();
	std::string stackTraceToString(Local<v8::StackTrace>);
	std::string stackFrameToString(Local<v8::StackFrame>);
	static bool stackTraceMatcher(void* key1, void* key2);
	template <typename T> void addToMap(HashMap*, T);
	template <typename T> void resetMap(HashMap*);
	template <typename T> T* nextFromMap(HashMap*);
	template <typename K, typename V> void copyMap(HashMap* src, std::map<K, std::vector<V> >* dest); //assumes dest is empty
	template <typename K, typename V> void copyMap(const std::map<K, std::vector<V> >* src, HashMap* dest); //assumes dest is empty
	template <typename K> uint32_t hashKey(K*);

	HashMap currentTimeMap; //XXX new currentTime store
	HashMap mathRandomMap; //XXX new mathRandom store

	double m_recordStartTime;
	double m_replayStartTime;

	State currentState;
};

}}

#endif /* PLATFORM_INSTRUMENTATION_H_ */
