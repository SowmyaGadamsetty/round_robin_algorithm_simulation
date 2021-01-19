/*
Walaa Mahmoud
Sowmya 
CSC 564 
Operating Systems
RR Project
Dr. Humphries


*/
/******************declare preprocessor directives********************/
#include <iostream>
#include <iomanip>
#include <fstream>
#include <queue>
#include <vector>
/********************************************************************/


using namespace std;

ofstream outfile;
/***declare a structure to hold all the attributes of the process*****/
struct Process {
	unsigned int id;					// unique identifier for the process
	unsigned int originalArrivalTime;	// in milliseconds
	unsigned int arrivalTime;			// in milliseconds, gets updated everytime the process is requeued
	unsigned int burstTime;			// the burst time in millisecond unit

	unsigned int serviceTime;	// the time that it was serviced by the CPU
	unsigned int waitingTime;	// time it took to wait before it was serviced
	unsigned int ExitTime;	// time it finished

	unsigned int processedburstTime;	// keeps track the quantum time processed in every pre-emption
	unsigned int preemptions;			// keeps track the number of times the process was preempted
};

/************* load text file into process ******************************
 * This function is used to load the data from the text file 'InputData'
 * into each process and save them into the struct and then 
 *	poopulates the inital queue
*********************************************************************/
void initialization(queue<Process *> & intialQueue) {
	unsigned int nextProcessId = 1;

	ifstream inFile("InputData.txt");

	while(!inFile.eof()) {
		unsigned int processId = nextProcessId;
		unsigned int arrivalTime;	// In seconds
		double burstTime;	// In seconds

		inFile >> arrivalTime;
		inFile >> burstTime;

		Process *process = new Process();
		process->id = nextProcessId;
		process->originalArrivalTime = arrivalTime * 1000;			// Convert to milliseconds
		process->arrivalTime = arrivalTime * 1000;					// Convert time to milliseconds
		process->burstTime = (unsigned int)(burstTime * 1000);	// Convert time to milliseconds
		process->waitingTime = 0;
		process->preemptions = 0;

		nextProcessId++;

		intialQueue.push(process);
	}

	inFile.close();
}

/************* Entry of the program  ******************************
 * prompt the user to enter quantum and context switch and store them
 * execute the round robin and calculates the parameters
 *	finally print the cacculated info into a file and out to the screen
*********************************************************************/
int main() {
	queue<Process *> intialQueue;	// Create an intial queue to hold the processes
	queue<Process *> readyQueue;	// create a ready queue to be used by scheduler
	vector<Process *> finishedProcesses;	// to hold the processes that completed execution

	Process *currentCPUProcess = NULL; 	// create an instance of the struct 
	
	unsigned int maximumQueueLength = 0; // variable to hold the maximum queue length

	unsigned int totalQueueLength = 0; // variable to hold the totalQueueLength length
	unsigned int queueChanges = 0;	// serves as a counter to keep track when the ready queue changes
	
/************* Prompt the user ******************************
 * ask the user to select a quantum and a context switch from 
 * a the list, the quantum ranges are 50, 100, 250, 500 and the context switch .
 *	ranges are0, 5, 10, 15, 20, 25. then save the input into the corresponding 
 * variables
*********************************************************************/	
	float Quantum;
	float Context_switch;
	cout << "Enter the value for the Time Quantum in (ms)" << endl << "Enter either 50, 100, 250, 500" << endl;
	cin >> Quantum;
	cout << " quantum is  = " << Quantum<< endl;
	cout << "Enter the value for the quantum switch in (ms)" << endl << "Enter either 0, 5, 10, 15, 20, 25" << endl;
	cin >> Context_switch;
	

	outfile.open("statistics.txt"); // open the Simulation Statistics file
	initialization(intialQueue); // call the initialization to start loading and initializing the structure

	
	unsigned int time = 0; // Start the simulation, CPU time start at 0 milliseconds
	
/************* Populate the queue ******************************
 * check if any of the queues is not empty then loop, and do some check
 * first check: if the inital queue is not empty and the arrivall time is less or equal to CPU time,
 *	populate the ready queue and check against the max queue length and update it accordingly.  
*********************************************************************/
	while(!intialQueue.empty() || !readyQueue.empty()) {
		// Put all processes in the queue for those who have arrived
		while(true) {
			if(!intialQueue.empty() && intialQueue.front()->arrivalTime <= time) {
				Process *process = intialQueue.front();
				intialQueue.pop();
				readyQueue.push(process);

				if(readyQueue.size() > maximumQueueLength)
					maximumQueueLength = readyQueue.size();

				totalQueueLength += readyQueue.size();
				queueChanges++;
			} else {
				break;
			}
		}
/*************************************************
 * 2nd check:  check if the CPU is busy, then calculate the wait time and go and check,
 *the process burst time left against the quantum and act accordingly.  
*********************************************************************/
		// If the CPU is currently processing something then fast forward it
		if(currentCPUProcess != NULL) {
			int burstTimeLeft = currentCPUProcess->burstTime - currentCPUProcess->processedburstTime;

			// Calculate the time it waited
			currentCPUProcess->waitingTime += (time - currentCPUProcess->arrivalTime);
			currentCPUProcess->arrivalTime = 0;

			// If the quantum time is not yet finished then put the process back to the dispatcher queue
			//cout << "[Time: " << time << "]";
			//cout << "[ID: P" << currentCPUProcess->id << "]";
			
			if(burstTimeLeft > Quantum) {
				currentCPUProcess->processedburstTime += Quantum;
				currentCPUProcess->preemptions++;
				readyQueue.push(currentCPUProcess);

				time += Quantum;

				totalQueueLength += readyQueue.size();
				queueChanges++;


			} else {
				time += burstTimeLeft;

				currentCPUProcess->processedburstTime += burstTimeLeft;
				currentCPUProcess->ExitTime = time;

				finishedProcesses.push_back(currentCPUProcess);

			}


/*************************************************
Free the CPU and update the current  time by adding the context switch 
*********************************************************************/

			time += Context_switch;
			currentCPUProcess = NULL;
		}

		// If the CPU is available get the next process and put it into the CPU
		if(currentCPUProcess == NULL) {
			if(!readyQueue.empty()) {
				// Get to the next process right away
				currentCPUProcess = readyQueue.front();
				readyQueue.pop();

				time += Context_switch;
			} else if(!intialQueue.empty()) {
				// If the inital is empty then fast forward the time to the next process
				time = intialQueue.front()->arrivalTime;
			}
		}
	}
/******************Calculate the requirements *******************************
	 calculate the total turnaround time and total waiting time 
	 Find the process with the lowest average waiting time.
	 Average waiting time for a process = Total waiting time / sum of all total waiting time of all processes
*********************************************************************/
	unsigned int totalTurnAroundTime = 0;
	unsigned int totalWaitingTime = 0;

	for(unsigned int i = 0; i < finishedProcesses.size(); i++) {
		totalTurnAroundTime += finishedProcesses[i]->ExitTime - finishedProcesses[i]->originalArrivalTime;
		totalWaitingTime += finishedProcesses[i]->waitingTime;
	}

	// Find the process with the lowest average waiting time
	Process * lowestAveWaitingProcess = finishedProcesses[0];
 
	for(unsigned int i = 1; i < finishedProcesses.size(); i++) {
		// Average waiting time for a process = Total waiting time / sum of all total waiting time of all processes
		if(((double)finishedProcesses[i]->waitingTime / (double)totalWaitingTime) < 
		   (lowestAveWaitingProcess->waitingTime / (double)totalWaitingTime)) {
			lowestAveWaitingProcess = finishedProcesses[i];
		}
	}

/******************Printing and storing final data *******************************
	 load all the calculated requirements into the statitics file
	 print them into the screen.
*********************************************************************/
	outfile << endl << "ROUND ROBIN SIMULATION: " << endl;

	outfile << "***************************************" << endl << endl;

	//output simulation parameters
	outfile << "Time Quantum: " << Quantum << endl << endl;
	outfile << "Dispatch Overhead : " << Context_switch << endl << endl;

	//output computed statistics 	
	outfile << "Maximum queue length: " << maximumQueueLength << endl << endl;
	
	outfile << "Average waiting time: " << (totalWaitingTime / finishedProcesses.size()) << endl << endl;
	outfile << "Average turn around time: " << ((double)totalTurnAroundTime / (double)finishedProcesses.size()) << endl << endl;
	
	outfile << "Average queue length: " << ((double)totalQueueLength / (double)queueChanges) << endl << endl;
	
	outfile << "Overall simulation time: " << finishedProcesses.back()->ExitTime << endl << endl;
	
	outfile << "total queue length " << maximumQueueLength << endl;
	


	
	cout << fixed;
	cout << "-----------------------------------" << endl;
	cout << "Time Quantum            : " << Quantum << endl;
	cout << "Dispatch Overhead       : " << Context_switch << endl;
	cout << "Maximum queue length    : " << maximumQueueLength << endl;
	cout << "Average waiting time    : " << setprecision(2) << ((double)totalWaitingTime / (double)finishedProcesses.size()) << endl;
	cout << "Average turn around time: " << setprecision(2) << ((double)totalTurnAroundTime / (double)finishedProcesses.size()) << endl;
	
	cout << "Average queue length    : " << setprecision(2) << ((double)totalQueueLength / (double)queueChanges) << endl;
	
	cout << "Overall simulation time : " << finishedProcesses.back()->ExitTime << endl;
	cout << "total queue length	: " 	<< maximumQueueLength << endl;
	cout << "-----------------------------------" << endl;
	cout << "Process with lowest average waiting time : [Arrival Time: " << lowestAveWaitingProcess->originalArrivalTime << " ms, Burst Time: " << lowestAveWaitingProcess->burstTime << "]" << endl;
	
	// Clear memory allocated objects
	for(unsigned int i = 0; i < finishedProcesses.size(); i++)
		delete finishedProcesses[i];

	system("pause");

	return 0;
}
