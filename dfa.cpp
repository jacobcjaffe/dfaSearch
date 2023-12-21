#include "dfa.h"
#include<string>
#include<queue>
#include<tuple>
#include<queue>
#include<random>
#include<bitset>
#include<cassert>

/// <summary>
/// default constructor
/// </summary>
dfa::dfa() 
{
    transitionTable[0] = nullptr;
    transitionTable[1] = nullptr;
}

/// <summary>
/// initializes the object private members by extracting all of the information from a text file.
/// initializes the transition table, table for calculating number of correctly accepted/rejected strings, 
/// and the set of accepting states.
/// </summary>
void dfa::fromFile(std::string path)
{
    std::ifstream in;
    in.open(path);
    int state;
    if (in.is_open()) {
        in >> numStates;
        transitionTable[0] = new int[numStates];
        transitionTable[1] = new int[numStates];

        // get the accepting states
        in >> numAcceptingStates;
		int temp;
        for (int i = 0; i < numAcceptingStates; i++) {
            in >> temp;
			acceptingStates.push_back(temp);
        }

        // get the length of string
        in >> lengthStr;

        // get the transition table
        for (int i = 0; i < numStates; i++) {
            in >> transitionTable[0][i];
            in >> transitionTable[1][i];
        }

        // initialize table to calculate the number of accepted string
        tableAcceptedStr = new double** [numStates];
        for (int i = 0; i < numStates; i++) {
            tableAcceptedStr[i] = new double* [lengthStr + 1];
            for (int j = 0; j < lengthStr + 1; j++) {
                tableAcceptedStr[i][j] = new double[lengthStr + 1];
                for (int k = 0; k < lengthStr + 1; k++) {
                    tableAcceptedStr[i][j][k] = 0;
                }
            }
        }
    }
    else {
        std::cout << "could't open file" << std::endl;
    }
}


/// <summary>
/// Calculates the number of correctly accepted strings and correctly rejected strings of size lengthStr.
/// </summary>
/// <returns></returns>
std::tuple<double, double, double> dfa::CalculateNumCorrect()
{
	// reset to 0
	for (int i = 0; i < numStates; i++) {
		for (int k = 0; k < lengthStr; k++) {
			for (int j = 0; j < lengthStr; j++) {
				tableAcceptedStr[i][k][j] = 0;
			}
		}
	}
		
    // format: tableAcceptedStr[state][string length][number of 1's]
    // initialize for empty string
    for (int i = 0; i < numAcceptingStates; i++) {
        tableAcceptedStr[acceptingStates[i]][0][0] = 1;
    }

    // build up to a solution
    for (int numOnes = 0; numOnes <= lengthStr; numOnes++) {
        for (int currentLength = 1; currentLength <= lengthStr; currentLength++) {
            for (int currentState = 0; currentState < numStates; currentState++) {
                int state_on_zero = transitionTable[0][currentState];
                int state_on_one = transitionTable[1][currentState];
                tableAcceptedStr[currentState][currentLength][numOnes] +=
					tableAcceptedStr[state_on_zero][currentLength - 1][numOnes];
                if (numOnes != 0 && currentLength != 0) {
                    tableAcceptedStr[currentState][currentLength][numOnes] +=
						tableAcceptedStr[state_on_one][currentLength - 1][numOnes - 1];
                }
            }
        }
    }


    double numCorrect = 0;
	double numCorrectlyAcc = 0;
	double numIncorrectlyAcc = 0;
    // SUM(count[start][length n][number of ones from n/2+1 to n]
    for (int i = lengthStr / 2 + 1; i <= lengthStr; i++) {
        numCorrect += tableAcceptedStr[0][lengthStr][i];
		numCorrectlyAcc += tableAcceptedStr[0][lengthStr][i];
    }
    // there are half of all possible strings that should be rejected
    numCorrect += pow(2, lengthStr - 1);
    // subtract amount accepted that should have been rejected
    for (int i = 1; i < (lengthStr / 2 + 1); i++) {
        numCorrect -= tableAcceptedStr[0][lengthStr][i];
		numIncorrectlyAcc += tableAcceptedStr[0][lengthStr][i];
    }

    numCorrectlyIdentified = numCorrect;
    return std::make_tuple(numCorrect, numCorrectlyAcc, numIncorrectlyAcc);
}

// brute force check the number of accepted strings of length lengthStr
double dfa::CalculateNumCorrectBrute() 
{
    bool overflow = false;
    double total = 0;

    BitVector bits(lengthStr);
    while (!bits.IsOverflow()) {
        if (CheckMembership(bits.GetVector())) {
            if (bits.HalfOrMoreOnes()) {
                total++;
            }
        }
        else {
            if (!bits.HalfOrMoreOnes()) {
                total++;
            }
        }
        ++bits;
    }
    numCorrectlyIdentified = total;
    
    return total;
}

// method to optimize the set of accepting states
double dfa::OptimizeAcceptingStates(){
	double numCorrectlyAccepted = 0;
	double numIncorrectlyAccepted = 0;
	double numCorrect = 0;
	BitVector bits(lengthStr);
	std::vector<int> localAcceptingStates;
	this->numAcceptingStates = 1;

	for(int i = 0; i < numStates; i++) {
		this->acceptingStates.clear();
		this->acceptingStates.push_back(i);
		std::tie(numCorrect, numCorrectlyAccepted, numIncorrectlyAccepted) = CalculateNumCorrect();
		std::cout << numCorrectlyAccepted << " " << numIncorrectlyAccepted << std::endl;
		// if number correctly accepted exceeds number incorrectly accepted
		if (numCorrectlyAccepted >= numIncorrectlyAccepted) {
			/*
			std::printf("numCorrectlyAccepted: %f numIncorrectlyAccepted: %f", 
					numCorrectlyAccepted, numIncorrectlyAccepted);
			*/
			localAcceptingStates.push_back(i);
		}
	}
	acceptingStates.clear();
	numAcceptingStates = 0;
	// copy all local accepting states
	for (int i = 0; i < localAcceptingStates.size(); i++) {
		//std::printf("pushing back 1");
		this->acceptingStates.push_back(localAcceptingStates[i]);
		numAcceptingStates++;
	}
	this->numAcceptingStates = this->acceptingStates.size();
	std::tie(numCorrect, numCorrectlyAccepted, numIncorrectlyAccepted) = this->CalculateNumCorrect();
	numCorrectlyIdentified = numCorrect;
	return numCorrect;
}

// checks the membership of the string
bool dfa::CheckMembership(std::vector<bool>& bits) 
{
    int state = 0;
    // iterate through the string
    for (int i = 0; i < bits.size(); i++) {
        state = transitionTable[bits[i]][state];
    }
    // see if the final state is a member of the accepting states
    for (int i = 0; i < numAcceptingStates; i++) {
        if (state == acceptingStates[i]) {
            return true;
        }
    }
    return false;
}

// go to next iteration of string, return true if adding 1 would yield overflow
bool dfa::incrementBitVec(std::vector<bool>& bits) {
    for (int i = 0; i < lengthStr; i++) {
        std::cout << bits[i] << " ";
    }
    std::cout << std::endl;
    for (int i = 0; i < bits.size(); i++) {
        if (bits[i] == 0) {
            bits[i] = 1;
            return false;
        }
        else {
            bits[i] = 0;
        }
    }
    return true;
}

// debugging methods
std::string dfa::PrintTransitionTable() 
{
    std::string output;
    if (numStates == -1) {
        std::cout << "transition table not initialized" << std::endl;
        return output;
    }
    output += "The Transition Table\n";
    output += " \t";
    for (int i = 0; i < numStates; i++) {
        output += std::to_string(i) + "\t \t";
    }
    output += "\n";
    for (int i = 0; i < 2; i++) {
        output += std::to_string(i) + "\t";
        for (int j = 0; j < numStates; j++) {
            output += std::to_string(transitionTable[i][j]) + "\t|\t";
        }
        output += "\n";
    }
    output += "\n";
    return output;
}

std::string dfa::PrintAcceptingStates() 
{
	std::string str;
    if (numAcceptingStates <= 0) {
        str = "accepting states not initialized\n";
        return str;
    }
    str = std::to_string(numAcceptingStates) + " accepting state(s): ";
    for (int i = 0; i < acceptingStates.size(); i++) {
        str += std::to_string(acceptingStates[i]) + " ";
    }
	str += "\n\n";
	return str;
}

void dfa::PrintTableAcceptedStr()
{
    for (int i = 0; i <= lengthStr; i++) {
        std::cout << "number of ones = " << i << std::endl;
        for (int j = 0; j < numStates; j++) {
            for (int k = 0; k <= lengthStr; k++) {
                std::cout << tableAcceptedStr[j][k][i] << "  ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

/// <summary>
/// Performs a DFS to see if a solution is even possible.
/// </summary>
/// <returns></returns>
bool dfa::DFSSolution() 
{
    std::vector<bool> visited(numStates);
	std::queue<int> searchQueue;
    int nextState, currState;

    for (int i = 0; i < numStates; i++) {
        visited[i] = false;
    }

    // start the queue with all of the accepting states
    for (int i = 0; i < numAcceptingStates; i++) {
        searchQueue.push(acceptingStates[i]);
    }

    // depth first search starting from the accepting state
    while (!searchQueue.empty()) {
        currState = searchQueue.front();
        nextState = transitionTable[0][currState];
        // if next state is the starting state, there is a possible solution
        if (nextState == 0) {
            return true;
        }
        // if the next state hasn't been visited, add it to the queue
        if (visited[nextState] == false) {
            searchQueue.push(nextState);
        }

        nextState = transitionTable[1][currState];
        // if next state is the starting state, there is a possible solution
        if (nextState == 0) {
            return true;
        }
        // if the next state hasn't been visited, add it to the queue
        if (visited[nextState] == false) {
            searchQueue.push(nextState);
        }

        searchQueue.pop();
    }
    
    // if not found
    return false;
}

double dfa::Perturb(int initialState, int transition, int finalState) {
    // save the previous just in case
    transitionTable[transition][initialState] = finalState;
    //numCorrectlyIdentified = CalculateNumCorrectBrute();
	//std::printf("perturbing");
	numCorrectlyIdentified = this->OptimizeAcceptingStates();
    return numCorrectlyIdentified;
}

/// <summary>
///  Returns a pointer to a deep copy of the current object.
/// </summary>
dfa* dfa::GetDeepCopy(bool debug) {
    dfa* newDFA = new dfa();
    newDFA->SetNumStates(numStates);
    newDFA->SetAcceptingStates(numAcceptingStates, acceptingStates);
    newDFA->SetTransitionTable(numStates, transitionTable);
    newDFA->SetLengthStr(lengthStr);
    newDFA->SetTableAccepted(numStates, lengthStr);
    newDFA->SetLengthStr(lengthStr);
    if (debug) {
        newDFA->PrintAcceptingStates();
        newDFA->PrintTransitionTable();
        newDFA->PrintTableAcceptedStr();
    }
    return newDFA;
}

/// <summary>
/// Get method to return the number of correctly calculated strings
/// </summary>
/// <returns></returns>
double dfa::GetNumCorrectStrings() {
    return numCorrectlyIdentified;
}

/// <summary>
/// Get method for the string length.
/// </summary>
/// <returns></returns>
int dfa::GetLengthStr()
{
    // TODO: implement this
    return lengthStr;
}

/// <summary>
/// Method to quickly change the length of string.
/// </summary>
/// <param name="newLength"></param>
void dfa::SetLengthStr(int newLength) 
{
    lengthStr = newLength;
}

void dfa::SetNumStates(int newNumStates) 
{
    numStates = newNumStates;
}

/// <summary>
/// Method used in setting accepting states for a deep copy
/// </summary>
/// <param name="newNumStates"></param>
/// <param name="states"></param>
void dfa::SetAcceptingStates(int newNumStates, std::vector<int>& states) 
{
	/*
    if (acceptingStates != nullptr) {
        std::cout << "ERROR: dfa is already set, can't allocate accepting states" << std::endl;
        return;
    }
	*/
    numAcceptingStates = newNumStates;
	acceptingStates.clear();
    //std::memcpy(&acceptingStates, &states, numStates);
    for (int i = 0; i < states.size(); i++) {
        acceptingStates.push_back(states[i]);
    }
    return;
}

/// <summary>
/// Method ised in setting the transition table for a deep copy
/// </summary>
/// <param name="table"></param>
void dfa::SetTransitionTable(int newNumStates, int** table) 
{
	/*
    // check to see if transition table is already allocated
    if (transitionTable[0] != nullptr || transitionTable[1] != nullptr) {
        std::cout << "ERROR: dfa is already set, can't allocate transition table" << std::endl;
        return;
    }
	*/
    // copy memory
    transitionTable[0] = new int[newNumStates];
    transitionTable[1] = new int[newNumStates];
    for (int i = 0; i < newNumStates; i++) {
        transitionTable[0][i] = table[0][i];
        transitionTable[1][i] = table[1][i];
    }
    return;
}

void dfa::SetTableAccepted(int newNumStates, int newLengthStr) {
    // check to see if the tableAcceptedStr has been allocated
    if (tableAcceptedStr != nullptr) {
        std::cout << "ERROR: dfa is already set, can't allocate the table for accepted strings" << std::endl;
        return;
    }

	// initialize table to calculate the number of accepted string
	tableAcceptedStr = new double** [newNumStates];
	for (int i = 0; i < newNumStates; i++) {
		tableAcceptedStr[i] = new double* [newLengthStr + 1];
		for (int j = 0; j <= newLengthStr; j++) {
			tableAcceptedStr[i][j] = new double[newLengthStr + 1];
			for (int k = 0; k <= newLengthStr; k++) {
				tableAcceptedStr[i][j][k] = 0;
			}
		}
	}
    return;
}

int dfa::GetNumStates()
{
    return numStates;
}

/// <summary>
/// Write the dfa to disk
/// </summary>
/// <param name="fileName"></param>
void dfa::WriteToDisk(std::string fileName) 
{

}

// clean up allocated memory
dfa::~dfa() 
{
    for (int i = 0; i < 2; i++) {
        if (transitionTable[i] != nullptr) {
            delete transitionTable[i];
        }
    }
    if (tableAcceptedStr != nullptr) {
        for (int i = 0; i < numStates; i++) {
            for (int j = 0; j < lengthStr; j++) {
                delete tableAcceptedStr[i][j];
            }
            delete tableAcceptedStr[i];
        }
    }
}

void dfa::Scramble() {
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist(0, numStates-1);
	for (int i = 0; i < 1000; i++) {
		transitionTable[dist(rng)%2][dist(rng)] = dist(rng);
	}
	return;
}