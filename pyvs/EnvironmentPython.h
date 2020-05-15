#ifndef __VS_EMVIRONMENT_PYTHON_H__
#define __VS_EMVIRONMENT_PYTHON_H__
#include "../sim/Environment.h"
#include <vector>
#include <string>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include "WrapperFunctions.h"
#include "Normalizer.h"

class EnvironmentPython
{
public:
	EnvironmentPython(int numAgent);
	// For general properties
	int getNumState();
	int getNumAction();
	int getNumDofs();
	int getSimulationHz(){return mSlaves[0]->getSimulationHz();}
	int getControlHz(){return mSlaves[0]->getControlHz();}

	// For each slave
	void step(int id);
	void stepAtOnce(int id);
	// void steps(int id);
	void reset(int id);
	void resets();
	bool isTerminalState(int id);
	np::ndarray getState(int id, int index);
	// np::ndarray getLocalState(int id, int index);
	void setAction(np::ndarray np_array, int id, int index);
	void setActions(np::ndarray np_array);
	double getReward(int id, int index, int verbose);
	double getNumIterations();

	// int getNumBallTouch(int id);

	void endOfIteration();

	// np::ndarray getHardcodedAction(int id, int index);

	// void reconEnvFromState(int id, int index, np::ndarray curLocalState);

	// For all slaves
	void stepsAtOnce();

	Normalizer* mNormalizer;
private:
	std::vector<Environment*> mSlaves;
	int mNumSlaves;
	int mNumState;
	int mNumAction;
};

#endif