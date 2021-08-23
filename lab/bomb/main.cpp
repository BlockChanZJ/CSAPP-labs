#include <bits/stdc++.h>
using namespace std;



int main() {
	for (int i = 'a'; i <= 'z'; i++)  {
		printf ("%c %d\n",i,i & 15);
	}
	cout << sizeof(long) << endl;
	return 0;
}
