#include <iostream>

#include "smeagol/Ports.hpp"

using namespace smeagol;

int foo() { return 9; }

struct Foo {
    int bar() const { return b; }
    void setBar(int v) { b = v; }
    int setBarReturn(int v)
    {
        b = v;
        return b;
    }
    int b = 10;
};

int main(int argc, char* argv[])
{
    // Constant variable output port
    int vConst = 7;
    OutputPort<int> varConstOutPort(vConst);
    std::cout << varConstOutPort.val() << std::endl;
    vConst = 8;
    std::cout << varConstOutPort.val() << std::endl << std::endl;

    // Reference variable output port
    int vRef = 7;
    OutputPort<int> varRefOutPort(&vRef);
    std::cout << varRefOutPort.val() << std::endl;
    vRef = 8;
    std::cout << varRefOutPort.val() << std::endl;

    // Function output port
    OutputPort<int> fnRefOutPort(&foo);
    std::cout << fnRefOutPort.val() << std::endl;

    // Instance function output port
    Foo f;
    OutputPort<int> objFnRefOutPort(std::bind(&Foo::bar, &f));
    std::cout << objFnRefOutPort.val() << std::endl;
    f.b += 1;
    std::cout << objFnRefOutPort.val() << std::endl;

    // Variable input port
    int varTarget = -1;
    InputPort<int> varConstInPort(&varTarget);
    std::cout << varTarget << std::endl;
    varConstInPort.notify(1);
    std::cout << varTarget << std::endl;

    // Member function input port
    InputPort<int> objFnInPort(
        std::bind(&Foo::setBar, &f, std::placeholders::_1));
    std::cout << f.b << std::endl;
    objFnInPort.notify(++f.b);
    std::cout << f.b << std::endl;

    // Port connections
    OutputPort<int> connectOut(7);
    int result = 0;
    InputPort<int> connectIn(&result);
    connectOut.connect(connectIn);
    std::cout << result << std::endl;
    connectIn.update();
    std::cout << result << std::endl;

    // Test connecting to int function with non-void return type
    // Show alternate bind method via lambdas
    InputPort<int> connectInObject([&f](int i) { return f.setBarReturn(i); });
    std::cout << f.bar() << std::endl;
    connectOut.connect(connectInObject);
    connectInObject.update();
    std::cout << f.bar() << std::endl;
}