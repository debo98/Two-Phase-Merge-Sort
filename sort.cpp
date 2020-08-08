#include <bits/stdc++.h>
#include <fstream>
#include <string>

using namespace std;

/***************************************
			GLOBAL VARIABLES 
***************************************/
// METADATA
typedef struct column{
	string name;
	int sz;
}column;
column col_details[20];
int number_of_cols = 0;

// MISC VARIABLES
int flag;
vector<int> sort_on;
int sort_on_size;
int memory_limit;
int threads;
int window;

// PHASE 1
int number_of_output_files = 1;
vector<vector<string>> buffer;

/**************************************
			METADATA
**************************************/

void scan_metadata(){
	cout << "Scanning metadata file...\n";
	ifstream metadata;
	metadata.open("./metadata.txt");
	if (!metadata) {
    	cerr << "ERROR: FILE NOT FOUND\n";
    	exit(1);
	}
	while(!metadata.eof()){
		try{
			getline(metadata, col_details[number_of_cols].name, ',');
			string size_str;
			getline(metadata, size_str);
			if(col_details[number_of_cols].name == "" && size_str == "")
				continue;
			col_details[number_of_cols++].sz = stoi(size_str);
		}
		catch (exception &e) {
			cerr << "ERROR: WRONG INPUT FORMAT\n";
			exit(1);
		}
	} 
	metadata.close();
	cout << "Metadata scanned successfully.\n";
}

/**************************************
			MISC FUNCTIONS
**************************************/

void find_window_size(string inputfilename){
	ifstream input;
	input.open(inputfilename);
	if (!input) {
    	cerr << "ERROR: FILE NOT FOUND\n";
    	exit(1);
	}
	string data;
	getline(input, data, '\n');
	input.close();
	window = (int) (0.8 * memory_limit * 1024 * 1024 / sizeof(data));
	
	ifstream file(inputfilename.c_str(), ifstream::in | ifstream::binary);
	file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.close();
	if(window * sizeof(data) < fileSize / window){
		cerr << "ERROR: FILE TOO BIG\n";
		exit(1);
	}
	// window = 1000;
}

vector<string> line_to_vec(ifstream &input_file){
	vector<string> linedata;
	char ch;
	string data;
	for(int col_no = 0; col_no < number_of_cols; col_no++){
		data.clear();
		for(int i=0; i < col_details[col_no].sz; i++){
			// if(input_file.eof()) return vector<string>();
			input_file.get(ch);
			if(ch == '\x00' && linedata.empty() && data.empty()){
				return vector<string>();
			}
			else if(ch == '\x00'){
				cerr << "ERROR: WRONG INPUT FORMAT\n";
				exit(1);
			}
			data.push_back(ch);
		}
		linedata.push_back(data);
		if(col_no == number_of_cols - 1){
			// End line
			input_file.get(ch);
		}
		else{
			// 2 spaces
			input_file.get(ch); input_file.get(ch);
		}
	}
	return linedata;
}

/**************************************
			PHASE 1
**************************************/


void write_to_file(string filepath){
	int size = buffer.size();
	ofstream output;
	output.open(filepath);
	if(!output){
		cerr << "ERROR: COULD NOT CREATE FILE\n";
		exit(1);
	}
	for(int i=0; i < size; i++){
		for(int j=0; j < number_of_cols-1; j++){
			output << buffer[i][j] << "  ";
		}
		output << buffer[i][number_of_cols-1] << "\n";
	}
	output.close();
}

bool comparator(const vector<string> &v1, const vector<string> &v2){
	bool ret=false;
	for(int i=0; i < sort_on_size; i++){
		if(v1[sort_on[i]] != v2[sort_on[i]]){
			if(flag==1) return (v1[sort_on[i]] < v2[sort_on[i]]);
			return (v1[sort_on[i]] > v2[sort_on[i]]);
		}
	}
	return 1;
}

void phase1_ops(){
	cout << "Sorting sublist #" << number_of_output_files << "...\n";
	sort(buffer.begin(), buffer.end(), comparator);
	cout << "Writing to file for sublist #" << number_of_output_files << "...\n";
	write_to_file("./temp_out"+to_string(number_of_output_files)+".txt");
	buffer.clear();
}

void phase1(char* inputfilename){
	cout << "Scanning input file...\n";
	ifstream input;
	input.open(inputfilename);
	if (!input) {
    	cerr << "ERROR: FILE NOT FOUND\n";
    	exit(1);
	}
	int lines_read = 0;
	while(true){
		if(lines_read < window){
			vector<string> linedata = line_to_vec(input);
			if(input.eof()) break;
			if(!linedata.empty()){
				buffer.push_back(linedata);
			}
			lines_read++;
		}
		else{
			phase1_ops();
			number_of_output_files++;
			lines_read = 0;
		}
		if(input.eof()) break;
	}
	phase1_ops();
	input.close();
	cout << "Phase1 completed successfully.\n";
}

/**************************************
				PHASE 2
**************************************/

bool com(const pair<vector<string>,int>& v1, const pair<vector<string>,int>& v2){
	bool ret=false;
	

	for(int i=0; i < sort_on_size; i++){
		if(v1.first[sort_on[i]] != v2.first[sort_on[i]]){
			if(flag==-1) return (v1.first[sort_on[i]] < v2.first[sort_on[i]]);
			return (v1.first[sort_on[i]] > v2.first[sort_on[i]]);
		}
	}
	return true;
}

void phase2(char* outputfilename){
	ifstream input[number_of_output_files];
	ofstream output;
	for(int i=0; i < number_of_output_files; i++){
		input[i].open("./temp_out" + to_string(i+1) + ".txt");
	}
	output.open(outputfilename);
	for(int i=0; i < number_of_output_files; i++){
		if(!input[i]){
			cerr << "ERROR: FILE NOT FOUND\n";
			exit(1);
		}
	}
	if(!output){
		cerr << "ERROR: COULD NOT CREATE FILE\n";
		exit(1);
	}

	cout << "Starting merging...\n";
	priority_queue< pair<vector<string>,int>, vector<pair<vector<string>,int> >, std::function<bool(pair<vector<string>,int>, pair<vector<string>,int>)>> q(com);
	for(int i=0; i < number_of_output_files; i++){
		vector<string> linedata = line_to_vec(input[i]);
		q.push(make_pair(linedata, i));
	}
	while(!q.empty()){	
		vector<string> f = q.top().first;
		int s = q.top().second;
		q.pop();
		for(int i=0; i < number_of_cols-1; i++){
			output << f[i] << "  ";
		}
		if(number_of_cols)
			output << f[number_of_cols-1] << "\n";
		vector<string> linedata;
		if(!input[s].eof()){
			linedata = line_to_vec(input[s]);
		}
		if(!linedata.empty()){
			q.push(make_pair(linedata, s));
		}
	}
	output.close();
	cout << "Phase 2 completed successfully.\n";
}


/**************************************
				MAIN
**************************************/

int main(int argc, char** argv){
	memory_limit = stoi(argv[3]);
	scan_metadata();
	find_window_size(argv[1]);
	int start = 5;
	// Single Thread
	if(!strcmp(argv[4], "asc"))
		flag = 1;
	else if(!strcmp(argv[4], "desc"))
		flag = -1;
	// Multi Thread
	else{
		start = 6;
		try{
			threads = stoi(argv[4]);
		}
		catch(exception &e){
			cerr << "ERROR: WRONG INPUT FORMAT\n";
			exit(1);
		}
		if(!strcmp(argv[5], "asc"))
			flag = 1;
		else if(!strcmp(argv[5], "desc"))
			flag = -1;
		else{
			cerr << "ERROR: WRONG INPUT FORMAT\n";
			exit(1);
		}
	}
	for(int i=start; i < argc; i++){
		for(int j=0; j < number_of_cols; j++){
			if(!col_details[j].name.compare(argv[i])){
				sort_on.push_back(j);
			}
		}
	}
	sort_on_size = sort_on.size();
	phase1(argv[1]);
	phase2(argv[2]);
	cout << "TASK COMPLETED!!!\n";
	return 0;
}