#ifndef DFA
#define DFA
#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include "BitVector.h"
#include<gmp.h>

class CalcResults {
	public:
		CalcResults() {};
		mpz_t Correct;
		mpz_t CorrectlyAcc;
		mpz_t IncorrectlyAcc;
		
		~CalcResults() {
			mpz_clear(Correct);
			mpz_clear(CorrectlyAcc);
			mpz_clear(IncorrectlyAcc);
		};
};

// a class to define the dfa and the methods that act on it
class dfa 
{
private:
    // nextState = transitiontable[alphabet][currentstate]
    mpz_t*** tableAcceptedStr = nullptr;
    int** transitionTable = new int*[2];
    int numStates = -1;
	std::vector<int> acceptingStates;
    int numAcceptingStates = 0;
    int lengthStr = 0;

    std::string fileName;
    double numCorrectlyIdentified;
	bool debug = false;
	double ratio;
	mpz_t Correct;

public :
    dfa();
    // pull definition from input text file
    void fromFile(std::string path);

	CalcResults* CalculateNumCorrect();
    double CalculateNumCorrectBrute();
	double OptimizeAcceptingStates();
	double CalculateRatio();
    bool CheckMembership(std::vector<bool> &bits);
    bool incrementBitVec(std::vector<bool>& bits);
	void Scramble();

    // debug methods
    std::string PrintTransitionTable();
	std::string PrintAcceptingStates();
    void PrintTableAcceptedStr();

    // dfs for a solution, skip if not possible
    bool DFSSolution();

    // changes one, transition, checks the new number of correct
    double Perturb(int initialState, int transition, int finalState);

    // returns a deep copy
    dfa* GetDeepCopy(bool debug);

    // set methods for deep copies
    void SetNumStates(int newNumStates);
    void SetAcceptingStates(int newNumStates, std::vector<int>& states);
    void SetTransitionTable(int newNumStates, int** table);
    void SetTableAccepted(int newNumStates, int newLengthStr);

    // get and set methods for string lengths to test on
    int GetLengthStr();
    void SetLengthStr(int newLength);
    double GetNumCorrectStrings();
    int GetNumStates();
	mpz_t* GetNumCorrectStringsMPZ();

    void WriteToDisk(std::string fileName);

    ~dfa();
};


#endif
