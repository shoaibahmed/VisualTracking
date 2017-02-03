#include "PWPTracker.h"

int main()
{
	cout << "Welcome to PWP tracker" << endl;
	
	PWPTracker myTracker("template.png");
	myTracker.startTracking();

	return 0;
}