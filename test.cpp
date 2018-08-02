#include <iostream>
#include <cstring>

int main()
{
    using namespace std;
    string *w;
    strcpy(w, "12345678");
    cout << w << endl;
    strcat(w, "asdfghj");
    cout << w << endl;
    strcat(w, "qwertyu");
    cout << w << endl;
    return 0;
}