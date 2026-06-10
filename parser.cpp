#include <fstream>
using namespace std;

int main()
{
    double current, voltage;
    ifstream data("datos_358.txt");
    ofstream out("datos_358.csv");
    out << "corriente, voltaje\n";
    while (data >> current >> voltage)
    {
        out << current << ", " << voltage << "\n";
    }
}