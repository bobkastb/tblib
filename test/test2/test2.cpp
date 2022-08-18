// test2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

union Utype{
    struct { int16_t X,Y,Z,F; };
    uint64_t ll;
};

struct Utype2 {
    int x;
    int y;
    char y1;
    int16_t u;

};

void test(){
    Utype x;
    x.F = 1 ; x.X=1;
    auto l = x.ll;
    auto L = x.X | (uint64_t(x.Y) < 16) | (uint64_t(x.Z) < 32) | (uint64_t(x.F) < 48);
    x.X = L & 0xFFFF;
    x.Y = (L>>16) & 0xFFFF;
}

template<size_t N> void testa( char (&v)[N] ) {
    auto u = N;
    v[1] = 100;
    std::cout << N;
}

int main()
{

    std::string xx="rrrr";
    std::cout << "Hello World!\n";
    char d[200];
    testa(d);
    test();

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
