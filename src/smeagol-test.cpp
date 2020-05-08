#include <iostream>

#include "smeagol/Ports.hpp"

using namespace smeagol;

int foo() { return 9; }

struct Foo {
    int bar() const { return b; }
    int b = 10;
};

int main(int argc, char* argv[])
{
    // Constant variable output port
    int vConst = 7;
    ValuedOutputPort<int> varConstPort(vConst);
    std::cout << varConstPort.val() << std::endl;
    vConst = 8;
    std::cout << varConstPort.val() << std::endl << std::endl;

    // Reference variable output port
    int vRef = 7;
    ValuedOutputPort<int> varRefPort(&vRef);
    std::cout << varRefPort.val() << std::endl;
    vRef = 8;
    std::cout << varRefPort.val() << std::endl;

    // Function output port
    ValuedOutputPort<int> fnRefPort(&foo);
    std::cout << fnRefPort.val() << std::endl;

    // Instance function output port
    Foo f;
    ValuedOutputPort<int> instanceFnRefPort(std::bind(&Foo::bar, &f));
    std::cout << instanceFnRefPort.val() << std::endl;
    f.b += 1;
    std::cout << instanceFnRefPort.val() << std::endl;
}