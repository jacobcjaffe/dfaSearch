#include<iostream>
#include<vector>
#include<iomanip>
#include<random>
#include<thread>
#include<functional>
#include<atomic>
#include<mutex>
#include<cassert>
#include<deque>
#include<fstream>
#include"dfa.h"
#include"GenSolution.h"
#include<sciplot/sciplot.hpp>

// change length from 9 vs 15
// keep 9 fixed, vary the length, for each length, optimum point will come differently, keep track of iterations!
// varying the length, up to 3 to four times the number of states
// track with two at a time??
/*
TODO:
search and see if you can find a better case with the test case
given a dfa, fixed everything
length vs success ratio, what is the nature of that graph

plot that from multiple machines on the same x and y axis, to do a comparison

how this random process changes, fix the length as say 15, plot the initial machine
success ratio, how is it improving? what is the shape of it?

MATPLOT++
latex

randomly generate for different starting states

after optimizing accepting states, can we make it incremental, not recalc verytime
drastic change, small chang, states not changing?
how long to reach mlocal maximum
reach quicker, quality may be quite good

compare with original algorithm
*/

int loops = 1000;
int idx1 = 0;
int idx2 = 0;
bool serial = false;
bool exhaustive = true;
bool test = true;
int STR_LENGTH = 15;
int NUM_STATES = 11;
int NUM_TESTS = 4;
// TODO: 15 states
// change lenght of string
// 1 or 2 experiments with a very large number of states
// iteratoins until local maximum, and success ratio, how does it grow
// from the application point of view, where should we stop
std::mutex idxLock;
int nThreads = 15;

int main(int argc, char** argv) {
	std::cout << "hi" << std::endl;
    std::ofstream out("output.txt");

    // initialize vectors
    std::vector<std::thread> tVec;
    std::vector<dfa*> dfaVector(10, nullptr);
    std::vector<dfa*> dfaVec;
    std::vector<dfa*> backtrack;
    std::vector<double> numCorrectVector(10, 0);
    double maxNumCorrect;
    int maxIndex;
    dfa* temp;

    dfa* current = new dfa();
    //int numCorrect = current->CalculateNumCorrectBrute();
    //backtrack.push_back(current);
    GenSolution::GenerateGenericTest("test.txt", 21, 11);
    current->fromFile("test.txt");
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 25); 
	int idx = 0;
	double max = 0;
	dfa* maxDFA = nullptr;

	/*
	test = true;
	if (test) {
		int s = dist(rng);
		for (int i = 0; i < 15; i++) {
			s = dist(rng);
			std::cout << "\t" << s << std::endl;
			while (s % 2 == 0) {
				s = dist(rng);
				std::cout << "\t" << s << std::endl;
			}
			GenSolution::GenerateGenericTest("test.txt", s, 11);
			delete current;
			current = new dfa();
			current->fromFile("test.txt");
			std::cout << std::setprecision(15) << std::get<0>(current->CalculateNumCorrect()) << std::endl;
			std::cout << std::setprecision(15) << current->CalculateNumCorrectBrute() << std::endl;
		}
		return 0;
	}
	*/

    if (serial) {
        int numStates = current->GetNumStates();
		// random search
        if (!exhaustive) { 
            for (int i = 0; i < loops; i++) {
                for (int j = 0; j < loops; j++) {
                    GenSolution::CopyDFAAndCalculate(current, (i * loops) + j, dist(rng), dist(rng), 0, maxDFA);
                    GenSolution::CopyDFAAndCalculate(current, (i * loops) + j, dist(rng), dist(rng), 1, maxDFA);
                }
                if (!dfaVec.empty()) {
                    // linearly search for max size
                    for (int k = 0; k < dfaVec.size(); k++) {
                        if (dfaVec[k]->GetNumCorrectStrings() > max) {
                            idx = k;
                            max = dfaVec[k]->GetNumCorrectStrings();
                        }
                    }

                    current = dfaVec[idx];
                    backtrack.push_back(current);

                    for (int k = 0; k < dfaVec.size(); k++) {
                        if (k != idx) {
                            delete dfaVec[k];
                        }
                    }
                    dfaVec.clear();
                }
                else {
                    std::cout << "found local maximum, num correct: " << 
						current->GetNumCorrectStrings() << std::endl;
                    break;
                }
            }
        }
        else {
            for (int i = 0; i < loops; i++) {
                // exhaustively go through all neighbors
                for (int j = 0; j < numStates; j++) {
                    for (int k = 0; k < numStates; k++) {
                        GenSolution::CopyDFAAndCalculate(current, i, j, k, 0, maxDFA);
                        GenSolution::CopyDFAAndCalculate(current, i, j, k, 1, maxDFA);
                    }
                }

				if (maxDFA != nullptr) {
					current = maxDFA;
					delete maxDFA;
					maxDFA = nullptr;
				}
				else {
                    std::cout << "found local maximum, num correct: " << 
						current->GetNumCorrectStrings() << std::endl;
                    out << current->GetNumCorrectStrings() << "\n";
                    out << current->PrintTransitionTable();
                    break;
                }
            }
        }
    }
    else if (!serial) {
		GenSolution solution;
		std::cout << "initializing" << std::endl;
		// multithreaded exhaustive search
		solution.ParallelExhaustiveSearch(11);
		return 0;
    }

    return 0;
}
