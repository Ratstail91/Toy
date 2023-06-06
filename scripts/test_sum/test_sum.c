#include <stdio.h>

//functions be local in C
int sum(int n) {
	if (n < 2) {
		return n;
	}

	return n + sum(n - 1);
}

//the test case (C)
void test_sum(int key, int val) {
	const int result = sum(val);
	printf("%d: %d\n", key, result);
}

int main() {
	for (int i = 0; i <= 10; i++) {
		test_sum(i, i * 1000);
	}
}