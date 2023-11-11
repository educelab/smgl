#include "smgl/Ports.hpp"

using namespace smgl;

///////////////////////////
///// Port Connection /////
///////////////////////////

void smgl::connect(Output& op, Input& ip)
{
    // op->ip verifies that ports are compatible, so do it first
    op.connect(&ip);
    ip.connect(&op);
}
void smgl::disconnect(Output& op, Input& ip)
{
    op.disconnect(&ip);
    ip.disconnect(&op);
}

void smgl::operator>>(Output& op, Input& ip) { connect(op, ip); }

void smgl::operator<<(Input& ip, Output& op) { connect(op, ip); }

////////////////
///// Port /////
////////////////

Port::Port(Port::State s) : state_{s} {}

void Port::setParent(Node* p) { parent_ = p; }

Port::State Port::state() const { return state_; }

void Port::setState(State s) { state_ = s; }

//////////////////
///// Output /////
//////////////////

Output::Output() : Port(State::Waiting) {}

/////////////////
///// Input /////
/////////////////

Input::Input() : Port(State::Idle) {}

Input::~Input()
{
    if (src_) {
        src_->disconnect(this);
    }
}

std::vector<Connection> Input::getConnections() const
{
    if (src_) {
        return {{src_->parent_, src_, parent_, const_cast<Input*>(this)}};
    } else {
        return {};
    }
}

size_t Input::numConnections() const { return (src_) ? 1 : 0; }

void Input::connect(Output* op) { src_ = op; }

void Input::disconnect(Output* op)
{
    if (src_ and src_ == op) {
        src_ = nullptr;
    }
}

Input& Input::operator=(Output& op)
{
    smgl::connect(op, *this);
    return *this;
}
