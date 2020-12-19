#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <set>
#include <algorithm>
#include <map>
#include <vector>
using namespace std;


struct backtrackState{
	vector<vector<int>> studentList; // student -> [course]
	vector<vector<int>> enrollmentList; // course -> [student]
	vector<int> courseList; // course -> timeslot
	
	vector<set<int>> conflictMatrix; // course -> [course]
	
	set<int> assignedCourse; // set of assigned course at current path
	int currentAssignmentValue; // 
	vector<vector<int>> currentAssignment; // timeslot -> [course]
	
	int bestAssignmentValue;
	vector<vector<int>> bestAssignment;// timeslot -> [course]
	
	int maxTimeSlot;
	string outputFile;
};

void readData(backtrackState& state, string stuFileName){
	state.outputFile = stuFileName.substr(0,stuFileName.length()-3) + "sol";
	
	ifstream file(stuFileName.c_str());
    
	string str;
	int iStudent = 0;
	while(getline(file, str)){
		auto currStudent = vector<int>{};
		while(str.size() > 4){
			if(str[0] == ' '){
				str = str.substr(1,string::npos);
			}
			int tmp = stoi(str.substr(0,4));
			str = str.substr(4,string::npos);
			
			if(tmp > state.courseList.size()){
				state.courseList.resize(tmp,-1);
				state.enrollmentList.resize(tmp);
			}
			state.enrollmentList[tmp-1].push_back(iStudent);
			currStudent.push_back(tmp-1);
		}
		iStudent++;
		state.studentList.push_back(currStudent);
	}
    file.close();
}

void saveBestAssignment(backtrackState& state){
	state.bestAssignmentValue = state.currentAssignmentValue;
	for(int i=0;i<state.bestAssignment.size();i++){
		state.bestAssignment[i].clear();
		for(int j=0;j<state.currentAssignment[i].size();j++){
			state.bestAssignment[i].push_back(state.currentAssignment[i][j]);
		}
	}
		
	ofstream file(state.outputFile);
	file << endl;
	for(int i=0;i<state.courseList.size();i++){
		file << i + 1 << " " << state.courseList[i] << endl;
	}
	file.close();
		
}

void computeConflictMatrix(backtrackState& state){
	state.conflictMatrix.resize(state.courseList.size());
	
	for(auto& list : state.studentList){
		for(int i=0;i<list.size();i++){
			for(int j=i+1;j<list.size();j++){
				state.conflictMatrix[list[i]].insert(list[j]);
				state.conflictMatrix[list[j]].insert(list[i]);
			}
		}
	}
}

int incrementalCost(backtrackState& state, int course, int timeSlot){
	// Check conflict
	for(auto j : state.currentAssignment[timeSlot]){
		if(state.conflictMatrix[course].count(j) > 0){
			return -1;
		}
	}
	
	int tmpCost = 0;
	for(auto i : state.enrollmentList[course]){
		for(auto j : state.studentList[i]){
			if(j != course && state.courseList[j] !=-1 && abs(state.courseList[j]-timeSlot) < 6 ){
				tmpCost += 1 << (4-(abs(state.courseList[j]-timeSlot)-1));
			}
		}
	}
	return tmpCost;
}

bool comparisonFunction(pair<int,int> a, pair<int,int> b){
	return a.second > b.second;
}

void validCandidates(backtrackState& state, vector<int>& candidateList){
	// No heuristics, we just check if the course has been previously assigned.
	// Later, we can add heuristic here if needed (e.g. SDO, LDO etc.)
	vector<pair<int,int>> heuristicList;
	for(int i=0;i<state.courseList.size();i++){
		if(state.assignedCourse.count(i) == 0){
			
			int heuristicValue=0;
			//heuristicValue += state.conflictMatrix[i].size(); // Largest Degree Ordering
			//heuristicValue += state.enrollmentList[i].size(); // Largest Enrollment
			
			/*
			//Saturation Degree Ordering
			for(int j =0;j < state.maxTimeSlot;j++){
				if(incrementalCost(state, i, j)== - 1){
					heuristicValue++;
				}
			}
			*/
			heuristicList.push_back(pair<int,int>(i, heuristicValue));
		}
	}
	sort(heuristicList.begin(),heuristicList.end(),comparisonFunction);
	
	for(int i=0;i< heuristicList.size();i++){
		candidateList.push_back(heuristicList[i].first);
	}
	
}

void backtrackFunction(backtrackState& state){
	if(state.assignedCourse.size() == state.courseList.size()){
		if((state.bestAssignmentValue == -1) || (state.bestAssignmentValue > state.currentAssignmentValue)){
			saveBestAssignment(state);
			cout << "Assignment found " << ((double)state.bestAssignmentValue)/state.studentList.size() << endl;
		}
		return;
	}
	
	
	vector<int> candidate;
	validCandidates(state, candidate);
	for(int i=0;i<candidate.size();i++){
		int course = candidate[i];
		for(int timeSlot=0;timeSlot<state.maxTimeSlot;timeSlot++){
			int tmp = incrementalCost(state, course, timeSlot);
			if(tmp < 0 || (state.bestAssignmentValue != -1 && state.bestAssignmentValue <= state.currentAssignmentValue + tmp)){
				continue;
			}
			
			state.currentAssignmentValue += tmp;
			state.assignedCourse.insert(course);
			state.courseList[course] = timeSlot;
			state.currentAssignment[timeSlot].push_back(course);
			
			backtrackFunction(state);
			
			state.currentAssignment[timeSlot].pop_back();
			state.courseList[course] = -1;
			state.assignedCourse.erase(course);
			state.currentAssignmentValue -= tmp;
			
		}
	}
}

int main(int argc, char *argv[]){
	backtrackState state;
	readData(state, argv[1]);
	state.maxTimeSlot = stoi(argv[2]);
	state.currentAssignmentValue = 0;
	state.currentAssignment.resize(state.maxTimeSlot);
	state.bestAssignmentValue = -1;
	state.bestAssignment.resize(state.maxTimeSlot);
	
	computeConflictMatrix(state);
	backtrackFunction(state);
	
	
	return 0;
}