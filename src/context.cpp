#include "context.h"
#include "term.h"

const std::string& Context::get_identifier(size_t index) const {
    assert(index < identifiers.size());
    return identifiers[index];
}

size_t Context::push_identifier(const std::string& identifier) {
    for (size_t i = 0; i < identifiers.size(); ++i) {
        if (identifiers[i] == identifier)
            return i;
    }

    if (definitions.find(identifier) != definitions.end())
        throw std::runtime_error("'" + identifier + "' is taken");

    identifiers.push_back(identifier);
    return identifiers.size() - 1;
}


std::shared_ptr<Term> Context::get_definition(const std::string& identifier) const {
    auto iter = definitions.find(identifier);
    if (iter == definitions.end())
        return nullptr;
    return iter->second;
}


void Context::define(const std::string& identifier, std::shared_ptr<Term> t) {
    for (size_t i = 0; i < identifiers.size(); ++i) {
        if (identifiers[i] == identifier)
            throw std::runtime_error("'" + identifier + "' is taken");
    }

    definitions[identifier] = t;
}

void Context::print(std::ostream& out) const {
    out << "identifiers: ";
    for (auto& id : identifiers)
        out << id << ", ";
    out << std::endl;
    out << "definitions: " << std::endl;
    for (auto it : definitions) {
        out << "\t" <<  it.first << " = "; 
        it.second->print(out, *this, 0);
        out << std::endl;
    }
}
