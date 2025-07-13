#include "enigma.hpp"

std::set<char> dorayaki::SetStringVisitor::operator()(const std::string &str) {
    return std::set<char>(str.cbegin(), str.cend()); 
}

std::set<char> dorayaki::SetStringVisitor::operator()(const std::set<char> &s) {
    return s; 
}

