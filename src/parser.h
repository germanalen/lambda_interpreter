#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <utility>
#include "term.h"
#include "context.h"


namespace Parser {
    enum Type {
        LEFT_PAREN,
        RIGHT_PAREN,
        LAMBDA,
        INDEX,
        IDENTIFIER,
        END
    };

    struct Token {
        Type type;
        size_t index;
        std::string identifier;
        std::string str;
    };

    std::shared_ptr<Term> parse(std::istream& in, Context& context);
}

#endif
