#include <cmath>
#include <iostream>


int smallArmCal(double rou, double h, double &theta, double &phay)
{
	double a(1.0), b(2.0);

    theta = asin((a * a + rou * rou + h * h - b * b) / (2 * a * sqrt(rou * rou + h * h))) - asin((a * a - rou * rou - h * h - b * b) / (2 * b * (rou * rou + h * h)));
    return 0;
}

/****************
 * x, y单位为mm
 * theta, alpha单位为rad
******************/
void scaraArmCal(double x, double y, double &theta, double &alpha)
{
	const double a(200.0), b(200.0);

    double rouSquare = x * x + y * y;
    double rou = sqrt(rouSquare);
    alpha = acos((a * a + b * b - rouSquare) / (2 * a * b));
    theta = acos((a * a + rouSquare - b * b) / (2 * a * rou)) + (y >= 0 ? atan2(y, x) : (atan2(y, x) + M_PI * 2));
}

int main()
{
	using namespace std;
	double x, y, theta(0), alpha(0);
	cout << "Input x and y. " << endl;
	cin >> x >> y;
	scaraArmCal(x, y, theta, alpha);
	cout << "theta and alpha\n"
		 << theta << "\t" << alpha
		 << endl;
    return 0;
}