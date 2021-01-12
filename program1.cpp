#include <iostream>
#include <vector>
#include <algorithm>
#include <ctime>
#include <sys/time.h>
#include <unistd.h>
using namespace std;

typedef vector<int> VI; // 1-dimensional array
typedef vector<VI> VVI; // 2-dimensional array
typedef vector<VVI> VVVI; // 3-dimensional array

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

int main() {
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
	long total_time_us = 0; // entire time
	for (int i = 0; i < N; ++i) {
		timeval start, end;
		gettimeofday(&start, NULL);
		
		VVI tmp = convolLayer(input, filters[i]);
		VVI res = reluLayer(tmp); // final output
		
		gettimeofday(&end, NULL);
		long start_time = 1000000*start.tv_sec + start.tv_usec;
		long end_time = 1000000*end.tv_sec + end.tv_usec;
		total_time_us += end_time - start_time;

		for (int j = 0; j < res.size(); ++j) {
			for (int k = 0; k < res[j].size(); ++k)
				cout << res[j][k] << ' ';
			cout << "\n";
		}
		cout << "\n";
	}
	// time doesn't include input/output time
	cout << total_time_us/1000 << "\n";
}







