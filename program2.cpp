#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
#include <utility>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
using namespace std;

typedef vector<int> VI; // 1-dimensional array
typedef vector<VI> VVI; // 2-dimensional array
typedef vector<VVI> VVVI; // 3-dimensional array

const int MAX_PROCESS_NUM = 123;

int main(int argc, char *argv[]) {
	// N = the number of filters, x = row size, y = column size
	int N, x, y;
	cin >> N >> x >> y;

	// filters[i] = 3-dimensional filter of 3-channel
	vector<VVVI> filters(N);

	for (int i = 0; i < N; ++i) {
		// filters[i][ch][j][k] = value of (j, k) of
		// (ch)th channel of (i)th filter
		filters[i] = VVVI(3, VVI(x, VI(y)));
		for (int ch = 0; ch < 3; ++ch)
			for (int j = 0; j < x; ++j)
				for (int k = 0; k < y; ++k)
					cin >> filters[i][ch][j][k];
	}
	// X = input row size, Y = input column size
	int X, Y;
	cin >> X >> Y;
	
	// input[i] = 2-dimensional input of (i)th channel
	VVVI input(3);

	for (int ch = 0; ch < 3; ++ch) {
		// zero padding
		input[ch] = VVI(X+2, VI(Y+2));
		for (int i = 1; i <= X; ++i)
			for (int j = 1; j <= Y; ++j)
				cin >> input[ch][i][j];
	}

	int total_process_num = atoi(argv[1]);
	// pros[i] = group of assigned filters in one process
	// pros[i][j] = (filter, filter's id) for (j)th filter in (i)th process
	vector<pair<VVVI, int>> pros[MAX_PROCESS_NUM];

	// entire time measurement start
	timeval start, end;
	gettimeofday(&start, NULL);

	// assign each filter to process
	for (int i = 0; i < N; ++i)
		pros[i % total_process_num].push_back(make_pair(filters[i], i));

	// make input file "tmp_input_i" for each process
	// make format of program1's input
	for (int i = 0; i < total_process_num; ++i) {
		ofstream fout;
		fout.open("tmp_input_" + to_string(i));
		
		int num_filter = pros[i].size();
		fout << num_filter << ' ' << x << ' ' << y << "\n";
		// write filter information
		for (int j = 0; j < num_filter; ++j) {
			VVVI& filter = pros[i][j].first;
			for (int ch = 0; ch < 3; ++ch) {
				// for each channel
				for (int p = 0; p < x; ++p) {
					for (int q = 0; q < y; ++q)
						fout << filter[ch][p][q] << ' ';
					fout << "\n";
				}
			}
		}
		// write input information
		fout << X << ' ' << Y << "\n";
		for (int ch = 0; ch < 3; ++ch) {
			// for each channel
			for (int p = 1; p <= X; ++p) {
				for (int q = 1; q <= Y; ++q)
					fout << input[ch][p][q] << ' ';
				fout << "\n";
			}
		}
		fout.close();
	}
	// pid[i] = (i)th child process
	pid_t pid[MAX_PROCESS_NUM] = {0};
	
	for (int i = 0; i < total_process_num; ++i) {
		pid[i] = fork(); // create process

		if (pid[i] == 0) {
			pid[i] = getpid();
			// for each child process, give input file "tmp_input_i" to
			// program1 and get output file "tmp_output_i"
			char *args[] = {(char *)"./program1", NULL};

			// open input and output files
			int in = open(("tmp_input_" + to_string(i)).c_str(), O_RDONLY);
    		int out = open(("tmp_output_" + to_string(i)).c_str(), O_WRONLY | 
				O_TRUNC | O_CREAT,S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);

    		// replace standard input with input file
    		dup2(in, 0);
    		// replace standard output with output file
    		dup2(out, 1);
    		// close unused file descriptors
    		close(in);
    		close(out);			
    		// execute program 1
   			execvp("./program1", args);

			// terminate (i)th child process
			exit(0);
		}
	}
	// parent process should wait until
	// all child process terminate
	int status;
	for (int i = 0; i < total_process_num; ++i)
		wait(&status);

	// row/column size of each result
	int n = X-x+3, m = Y-y+3;
	// res[i] = result of passing input by filters[i]
	VVVI res(N, VVI(n, VI(m)));
	// ptime[i] = time(ms) taken by (i)th process
    long ptime[MAX_PROCESS_NUM] = {0};

	for (int i = 0; i < total_process_num; ++i) {
		// Using output file "tmp_output_i", save result of 
        // each filter in (i)th process
        ifstream fin;
        fin.open("tmp_output_" + to_string(i));

        int num_filter = pros[i].size();
        for (int p = 0; p < num_filter; ++p) {
            int fid = pros[i][p].second; // filter's id
            // result of (fid)th filter is in "tmp_output_i"
            for (int q = 0; q < n; ++q)
                for (int r = 0; r < m; ++r)
                    fin >> res[fid][q][r];
		}
		fin >> ptime[i];
        fin.close();
        // remove temporary files
        remove(("tmp_input_" + to_string(i)).c_str());
        remove(("tmp_output_" + to_string(i)).c_str());
	}
	// entire time measurement end
	gettimeofday(&end, NULL);

	// print result in order
	// n,m = row/column size of each result
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < n; ++j) {
			for (int k = 0; k < m; ++k)
				cout << res[i][j][k] << ' ';
			cout << "\n";
		}
		cout << "\n";
	}
	// print time of each child process
	for (int i = 0; i < total_process_num; ++i)
		cout << ptime[i] << ' ';
	cout << "\n";
	// print entire time
	// it doesn't include parent process' input/output time
	long start_time = 1000000*start.tv_sec + start.tv_usec;
	long end_time = 1000000*end.tv_sec + end.tv_usec;
	cout << (end_time - start_time)/1000 << "\n";
}






