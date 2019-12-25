#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>
#include <map>
#include <memory>
#include <cassert>

class Term;

class Context {
public:
    const std::string& get_identifier(size_t index) const;
    size_t push_identifier(const std::string& identifier);

    std::shared_ptr<Term> get_definition(const std::string& identifier) const;
    void define(const std::string& identifier, std::shared_ptr<Term> t);

    void print(std::ostream& out) const;
private:
    std::vector<std::string> identifiers;

    std::map<std::string, std::shared_ptr<Term>> definitions;
};


#endif
