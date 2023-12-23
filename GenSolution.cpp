#include "GenSolution.h"
#include<fstream>
#include <iomanip>
#include <sciplot/specs/TitleSpecsOf.hpp>
#include<string>
#include<random>
#include<mutex>
#include<cassert>
#include<sciplot/sciplot.hpp>
#include<thread>

using namespace sciplot;

// method to generate the base case
void GenSolution::GenerateGenericTest(std::string fileName, int lengthStr, int numStates) {
	if (lengthStr % 2 != 1) {
		std::printf("generated test case needs an odd length string");
		return;
	}
	if (numStates < 3) {
		std::printf("generated test case needs at least 3 states");
		return;
	}

	std::ofstream out;
	out.open(fileName);
	if (out.is_open()) 
	{
		out << numStates << '\n';
		// start with one accepting state, furthest right
		out << 1 << " " << (numStates / 2 - 1) << '\n';
		out << lengthStr << '\n';

		// transition table
		// format (n) (n-1)... (n / 2 + 1) (0) (1) ... (n / 2)
		// on state 0
		int testing = (numStates / 2) + 1;
		out << testing << " " << 1 << '\n';
		for (int i = 1; i < floor(numStates / 2); i++) {
			if (i == floor(numStates / 2)) {
				break;
			}
			out << (i - 1) << " " << (i + 1) << '\n';
		}
		// the furthest right state will have edges go toward itself
		out << (numStates / 2) << " " << (numStates / 2) << '\n';
		// the first left state will have 0 to another further left, and 1 going to 0
		out << (numStates / 2) + 1 + 1 << " " << 0 << '\n';
		for (int i = (numStates / 2) + 2; i < numStates - 1; i++) {
			out << (i + 1) << " " << (i - 1) << '\n';
		}
		// the furthest left state will have 0 edge to itself, 1 edge to the right
		out << (numStates - 1) << " " << (numStates - 2) << std::endl;
	}
	else {
		std::cout << "couldn't open file to write to" << std::endl;
	}
	std::cout << "finished writing case to file" << std::endl;
}

dfa* GenSolution::CopyDFAAndCalculate(dfa* current, int id, int src, int dst, int transition, dfa* maxDFA) {
	//std::cout << "ThreadId: " << id << std::endl;
	//std::printf("perturbing %i to %i on %i\n", src, dst, transition);
	dfa* test = current->GetDeepCopy(false);
	dfa* dummy;
	double numCorrect = test->Perturb(src, transition, dst);
	//std::printf("finished perturbing \n");
	if (maxDFA != nullptr) {
		if (numCorrect > maxDFA->GetNumCorrectStrings()) {
			//std::printf("found larger\n");
			delete maxDFA;
			return test;
		}
		else {
			//std::printf("didn't find one larger");
			delete test;
			return maxDFA;
		}
	}
	return test;
}

 void GenSolution::ThreadCopyAndCalculate(int start, int end, int max, 
		std::vector<dfa*>& dfaVec, dfa* current, int itr, std::mutex* m) 
{
	std::printf("thread %i calculating from start: %i to end: %i\n", itr, start, end);
	thread_local dfa* localMax = nullptr;
	int idx = 0;
	// exhaustively search for all changes from start to end state
	for (int i = start; i < end; i++) {
		for (int j = 0; j < current->GetNumStates(); j++) {
			localMax = GenSolution::CopyDFAAndCalculate(current, itr, i, j, 0, localMax);
			localMax = GenSolution::CopyDFAAndCalculate(current, itr, i, j, 0, localMax);
		}
	}
	if (localMax != nullptr) 
	{
		if (localMax->GetNumCorrectStrings() > current->GetNumCorrectStrings()) {
			std::printf("thread %i found one larger", itr);
		}
		else {
			return;
		}
	}
	// lock
	{
		const std::lock_guard<std::mutex> lock(*m);
		if (localMax != nullptr) {
			dfaVec.push_back(localMax);
			std::printf("pushed to dfaVec, size is %i\n", (int)dfaVec.size());
		}
		// else found a local maxima
	}
	return;
}

void GenSolution::ParallelExhaustiveSearch(int numStates) {
	std::cout << "starting the test" << std::endl;
	dfa* current;
	int start = 0, end = 0, maxNumCorrect = 0, maxIndex = -1;
	std::mutex mtx;
	std::mutex* mPtr = &mtx;
	dfa* maxPtr;
	int itr = 0;
	int count = 0;
	std::ofstream out("output.txt");
	std::vector<std::thread> tVec;
	std::vector<dfa*> dfaVec;
	std::vector<double> x;
	std::vector<double> y;
	dfa* temp = new dfa();
	temp->fromFile("test.txt");

	for (int testNum = 3; testNum < 4.5 * numStates; testNum+=2) {
		x.push_back(testNum);
		std::cout << "lengthStr: " << testNum << std::endl;
		GenerateGenericTest("test.txt", testNum, numStates);
		current = new dfa();
		current->fromFile("test.txt");
		itr = 0;
		current->OptimizeAcceptingStates();
		//return;
		// print starting point
		out << "Starting DFA: " << current->GetNumCorrectStrings() << "\n";
		out << "Length String: " << testNum << "\n";
		out << current->PrintAcceptingStates();
		out << current->PrintTransitionTable();
		while(true) {
			std::cout << "computing iteration " << itr+1 << std::endl;
			itr++;
			start = 0;
			for (int threadID = 0; threadID < nThreads; threadID++) {
				if (threadID == nThreads - 1) {
					end = numStates;
				}
				else {
					end = (numStates * (threadID + 1)) / nThreads;
				}
				tVec.push_back(std::thread(&GenSolution::ThreadCopyAndCalculate, 
					start, end, testNum, std::ref(dfaVec), std::cref(current), threadID, 
					std::ref(mPtr)));
				if (end == numStates) {
					break;
				}
				start = end;
			}
			for (int threadID = 0; threadID < nThreads; threadID++) {
				tVec[threadID].join();
			}
			tVec.clear();
			std::cout << "threads joined" << std::endl;

			// search for a max
			maxIndex = -1;
			maxNumCorrect = 0;
			maxPtr = nullptr;
			// if none were found, we have a local maxima
			if (dfaVec.size() == 0) {
				std::cout << "found local maxima!" << std::endl;
				break;
			}
			else {
				std::cout << "making best move..." << std::endl;
				count++;
			}

			// linearly search for next best, all guranteed to be better than current
			for (int i = 0; i < dfaVec.size(); i++) {
				if (dfaVec[i]->GetNumCorrectStrings() > maxNumCorrect) {
					maxPtr = dfaVec[i];
					maxNumCorrect = maxPtr->GetNumCorrectStrings();
				}
			}
			// clean up, don't delete best move
			for (int i = 0; i < dfaVec.size(); i++) {
				if (dfaVec[i] != maxPtr) {
					delete dfaVec[i];
				}
			}
			dfaVec.clear();

			if (maxPtr != nullptr) {
				std::printf("we found one \n");
				//std::cout << maxPtr->PrintTransitionTable();
				out << itr << "\n";
				out << std::setprecision(15) <<
					maxPtr->GetNumCorrectStrings() << "\n";
				out << std::setprecision(15)
					<< maxPtr->GetNumCorrectStrings() / std::pow(2, testNum) << "\n";
				out << maxPtr->PrintAcceptingStates() << "\n";
				out << maxPtr->PrintTransitionTable();
				out << "\n--------------------------------------------\n\n";
				delete current;
				current = maxPtr;
			}
		}
		std::cout << "Ending DFA: ";
		std::cout << std::setprecision(15) << current->GetNumCorrectStrings() << "\n";
		std::cout << std::setprecision(15) <<
			current->GetNumCorrectStrings() / std::pow(2, testNum) << "\n";
		std::cout << "Number of steps: " << count << "\n";
		std::cout << current->PrintAcceptingStates();
		std::cout << current->PrintTransitionTable();
		std::cout << "\n\n--------------------------------------------\n\n";
		count = 0;
		out << "Ending DFA: ";
		out << std::setprecision(15) << current->GetNumCorrectStrings() << "\n";
		out << std::setprecision(15) <<
			current->GetNumCorrectStrings() / std::pow(2, testNum) << "\n";
		out << "Number of steps: " << count << "\n";
		out << current->PrintAcceptingStates();
		out << current->PrintTransitionTable();
		out << "\n\n--------------------------------------------\n\n";
		count = 0;
		y.push_back(current->GetNumCorrectStrings() / std::pow(2, testNum));
		std::cout << "    FINAL" << current->GetNumCorrectStrings() << std::endl;
	}
	std::string title = "Ratio of Correctly Identified Strings With Constant "
		+ std::to_string(numStates) 
		+ " States";
	std::cout << title << std::endl;
	PlotResults(x, y, title);
}

void GenSolution::PlotResults(std::vector<double>& x, std::vector<double>& y, std::string t)
{
	std::cout << "plotting " << t << std::endl;
	Plot2D plot;
	plot.xlabel("Length of String");
	plot.ylabel("Ratio of Strings Correctly Identfied");
	plot.yrange(0.0, 1.0);
	plot.xrange((int)0, x.back());
	plot.drawCurveWithPoints(x, y);

	std::ofstream table("table.txt");
	table << t << std::endl;
	for (int i = 0; i < x.size(); i++) {
		table << x[i] << " ";
	}
	table << std::endl;
	for (int j = 0; j < y.size(); j++) {
		table << y[j] << " ";
	}
	table << std::endl;

	Figure fig = {{plot}};
	fig.title(t);
	fig.palette("dark2");

	Canvas canvas = {{fig}};
	canvas.title(t);
	canvas.size(800, 800);
	canvas.show();
	std::string savefile = t + ".pdf";
	canvas.save(savefile);
	return;
}
