#pragma once
#include "dfa.h"
#include<iostream>
#include<string>
#include<mutex>

class GenSolution 
{
private:
	std::mutex mtx;
	const int nThreads = 16;
public:
	//static double FindSolution();
	GenSolution() = default;
	static void GenerateGenericTest(std::string fileName, int lengthStr, int numStates);
	static dfa* CopyDFAAndCalculate(dfa* current, int id, int src, int dst, 
			int transition, dfa* max);
	// for multithreaded version, iterates through numStates, from start state to end
	static void ThreadCopyAndCalculate(int start, int end, int max,
			std::vector<dfa*>& dfaVec, dfa* current, int itr, std::mutex* m);
	static void PlotResults(std::vector<double>& x, std::vector<double>& y, std::string title);
	void ParallelExhaustiveSearch(int numStates);
};
