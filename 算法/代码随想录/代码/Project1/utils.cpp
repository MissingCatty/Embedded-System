#include "utils.h"

void Utils::printMatrix(const vector<vector<int>>& matrix) {
	for (int i = 0; i < matrix.size(); i++) {
		for (int j = 0; j < matrix[i].size(); j++) {
			cout << matrix[i][j] << "\t";
		}
		cout << endl;
	}
}
