#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <pthread.h>
#include <vector>
#include <ctime>
#include <sys/time.h>
#include <algorithm>
using namespace std;

typedef vector<int> VI; // 1-dimensional array
typedef vector<VI> VVI; // 2-dimensional array
typedef vector<VVI> VVVI; // 3-dimensional array

const int MAX_THREAD_NUM = 123;

/******shared memory******/
// N = the number of filters, x = row size, y = column size
int N, x, y;
// filters[i] = 3-dimensional filter of 3-channel
vector<VVVI> filters;
// X = input row size, Y = input column size
int X, Y;
// input[i] = 2-dimensional input of (i)th channel
VVVI input;
// row/column size of each result
int n, m;
// res[i] = result of passing input by filters[i]
VVVI res;
// thtime[i] = time(ms) taken by (i)th thread
VI thtime;

/*************************/

VVI convolution(const VVI& f, const VVI& g) {
    // f = 2-dimensional input data (1-channel)
    // g = 2-dimensional filter (1-channel)
    int fr = f.size(), fc = f[0].size();
    int gr = g.size(), gc = g[0].size();
    // ret = return value, result of convolution f*g
    VVI ret(fr-gr+1, VI(fc-gc+1));

    // (sx, sy): left uppermost position of g
    // it also means calculating position of ret
    for (int sx = 0; sx <= fr-gr; ++sx)
        for (int sy = 0; sy <= fc-gc; ++sy)
            // for each g position (sx, sy), calculate ret[sx][sy]
            for (int x = 0; x < gr; ++x)
                for (int y = 0; y < gc; ++y)
                    ret[sx][sy] += f[sx+x][sy+y] * g[x][y];
    return ret;
}

VVI convolLayer(const VVVI& input, const VVVI& filter) {
    // input = 3-dimensional input data (3-channel)
    // filter = 3-dimensional filter (3-channel)
    // ret = return value, result of passing input through filter
    VVI ret = convolution(input[0], filter[0]);

    for (int ch = 1; ch <= 2; ++ch) {
        VVI res = convolution(input[ch], filter[ch]);
        // just add value of same position for each channel
        for (int i = 0; i < ret.size(); ++i)
            for (int j = 0; j < ret[i].size(); ++j)
                ret[i][j] += res[i][j];
    }
    return ret;
}

VVI reluLayer(const VVI& f) {
    // f = 2-dimensional output which has passed through convolution layer
    int n = f.size(), m = f[0].size();
    // ret = return value, result of passing f through ReLU layer
    VVI ret(n, VI(m));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            ret[i][j] = max(0, f[i][j]);
    return ret;
}

void *thread_function(void *arg) {
	// main function in thread
	// arg contain filters' id
	// pass input by each filter and save result in res
	// return time taken by thread
	VI* fid = static_cast<VI*>(arg);
	timeval start, end;
	gettimeofday(&start, NULL);

	for (int i = 0; i+1 < fid->size(); ++i) {
		VVI tmp = convolLayer(input, filters[(*fid)[i]]);
		res[(*fid)[i]] = reluLayer(tmp);
	}
	gettimeofday(&end, NULL);
	long start_time = 1000000*start.tv_sec + start.tv_usec;
	long end_time = 1000000*end.tv_sec + end.tv_usec;

	// last element of fid = thread id
	int tid = fid->back();
	thtime[tid] = (end_time - start_time)/1000;
	// terminate thread
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {

	cin >> N >> x >> y;
	filters.resize(N);

	for (int i = 0; i < N; ++i) {
		// filters[i][ch][j][k] = value of (j, k) of
		// (ch)th channel of (i)th filter
		filters[i] = VVVI(3, VVI(x, VI(y)));
		for (int ch = 0; ch < 3; ++ch)
			for (int j = 0; j < x; ++j)
				for (int k = 0; k < y; ++k)
					cin >> filters[i][ch][j][k];
	}
	cin >> X >> Y;
	input.resize(3);

	for (int ch = 0; ch < 3; ++ch) {
		// zero padding
		input[ch] = VVI(X+2, VI(Y+2));
		for (int i = 1; i <= X; ++i)
			for (int j = 1; j <= Y; ++j)
				cin >> input[ch][i][j];
	}

	int total_thread_num = atoi(argv[1]);
	pthread_t thread_handle[MAX_THREAD_NUM];
	void *thread_result;
	// each variable's explanation is written above
    n = X-x+3; m = Y-y+3;
    res = VVVI(N, VVI(n, VI(m)));
	thtime.resize(total_thread_num);

	// entire time measurement start
	timeval start, end;
	gettimeofday(&start, NULL);
    
	// assign each filter to thread
	// ths[i] = group of filter id in (i)th thread
	VVI ths(total_thread_num);
	for (int i = 0; i < N; ++i)
		ths[i % total_thread_num].push_back(i);
	// last element of ths[i] indicates thread id
	for (int i = 0; i < ths.size(); ++i)
		ths[i].push_back(i);

	// create threads
	for (int i = 0; i < total_thread_num; ++i)
		pthread_create(&thread_handle[i], NULL, thread_function, &ths[i]);

	// wait until all threads terminate
	for (int i = 0; i < total_thread_num; ++i)
		pthread_join(thread_handle[i], &thread_result);	

	// entire time measurement stop
    gettimeofday(&end, NULL);

	// print result in order
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < m; ++k)
                cout << res[i][j][k] << ' ';
            cout << "\n";
        }
        cout << "\n";
    }
	// print each thread time
	for (int i = 0; i < total_thread_num; ++i)
		cout << thtime[i] << ' ';
	cout << "\n";
	// print entire time
	// it doesn't include input/output time
	long start_time = 1000000*start.tv_sec + start.tv_usec;
    long end_time = 1000000*end.tv_sec + end.tv_usec;
	cout << (end_time - start_time)/1000 << "\n";
}























