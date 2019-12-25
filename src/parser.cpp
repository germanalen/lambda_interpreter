#include "parser.h"
#include <map>
#include <cctype>
#include <stdexcept>

using std::cout;
using std::endl;

bool read_index(std::istream& in, size_t& n) {
    char c;
    if (!in.get(c)) return false;
    if (!isdigit(c)) return false;

    n = c - '0';
    while(in.get(c)) {
        if (isdigit(c)) {
            n = n * 10 + c - '0';
        } else {
            in.putback(c);
            break;
        }
    }
    return true;
}

void skip_whitespace(std::istream& in) {
    char c;
    while(in.get(c)) {
        if (!isspace(c)) {
            in.putback(c);
            break;
        }
    }
}

// for identifiers
bool isvalid(char c) {
    return 33 <= c && c <= 126 && 
        c != '$' && c != '(' && c != ')' && c != '#';
}

bool read_identifier(std::istream& in, std::string& identifier) {
    char c;
    if (!in.get(c)) return false;
    if (!isvalid(c)) return false;

    identifier = "";
    identifier.push_back(c);
    while(in.get(c)) {
        if (isvalid(c)) {
            identifier.push_back(c);
        } else {
            in.putback(c);
            break;
        }
    }
    return true;   
}

void read_token(std::istream& in, Parser::Token& token) {
    skip_whitespace(in);

    char c;
    if (!in.get(c)) {
        token.type = Parser::END;
        token.str = "end of file";
        return;
    }

    if (c == '#') {
        size_t index;
        if (!read_index(in, index))
            throw std::runtime_error("Index expected after #");
        token.type = Parser::INDEX;
        token.index = index;
        token.str = "#" + std::to_string(index);
    } else if (c == '(') {
        token.type = Parser::LEFT_PAREN;
        token.str = "(";
    } else if (c == ')') {
        token.type = Parser::RIGHT_PAREN;
        token.str = ")";
    } else if (c == '$') {
        token.type = Parser::LAMBDA;
        token.str = "$";
    } else if (isvalid(c)) {
        std::string identifier;
        in.putback(c);
        read_identifier(in, identifier);
        
        token.type = Parser::IDENTIFIER;
        token.identifier = identifier;
        token.str = identifier;
    } else {
        throw std::runtime_error("Invalid character: '" + 
                std::string(1, c) + "' (ascii " + std::to_string(int(c)) + ")");
    }

    return;
}



std::shared_ptr<Term> Parser::parse(std::istream& in, Context& context) {
    // SLR algorithm
    // http://www.cs.ecu.edu/karl/5220/spr16/Notes/Bottom-up/slr1.html
    // https://web.cs.dal.ca/~sjackson/lalr1.html
    // http://smlweb.cpsc.ucalgary.ca/
    // The tables were generated at
    // http://jsmachines.sourceforge.net/machines/lalr1.html 
    
    // Grammar
    // https://en.wikipedia.org/wiki/Lambda_calculus_definition#Syntax_definition_in_BNF
    // (0) S' -> E
    // (1) E -> L E
    // (2) E -> A
    // (3) A -> A I
    // (4) A -> I
    // (5) I -> x
    // (6) I -> ( E )

    enum {S, E, A, I};
    enum {L, X, LP, RP, END};
    enum {SHIFT, REDUCE, ACCEPT, REJECT};

    const int reduce_num[7] = {-1, -2, -1, -2, -1, -1, -3};
    const int reduce_nonterm[7] = {S, E, E, A, A, I, I};

    const int num_states = 11;
    const int null_state = -1;

    const int goto_table[num_states][4] = {
        {-1,  1,  3,  4}, //0
        {-1, -1, -1, -1}, //1
        {-1,  7,  3,  4}, //2
        {-1, -1, -1,  8}, //3
        {-1, -1, -1, -1}, //4
        {-1, -1, -1, -1}, //5
        {-1,  9,  3,  4}, //6
        {-1, -1, -1, -1}, //7
        {-1, -1, -1, -1}, //8
        {-1, -1, -1, -1}, //9
        {-1, -1, -1, -1}, //10
    }; // S,  E,  A,  I

    const int action_table[num_states][5][2] = {
        {{ SHIFT,2},{ SHIFT,5},{ SHIFT,6},{REJECT,0},{REJECT,0}}, //0
        {{REJECT,0},{REJECT,0},{REJECT,0},{REJECT,0},{ACCEPT,0}}, //1
        {{ SHIFT,2},{ SHIFT,5},{ SHIFT,6},{REJECT,0},{REJECT,0}}, //2
        {{REJECT,0},{ SHIFT,5},{ SHIFT,6},{REDUCE,2},{REDUCE,2}}, //3
        {{REJECT,0},{REDUCE,4},{REDUCE,4},{REDUCE,4},{REDUCE,4}}, //4
        {{REJECT,0},{REDUCE,5},{REDUCE,5},{REDUCE,5},{REDUCE,5}}, //5
        {{ SHIFT,2},{ SHIFT,5},{ SHIFT,6},{REJECT,0},{REJECT,0}}, //6
        {{REJECT,0},{REJECT,0},{REJECT,0},{REDUCE,1},{REDUCE,1}}, //7
        {{REJECT,0},{REDUCE,3},{REDUCE,3},{REDUCE,3},{REDUCE,3}}, //8
        {{REJECT,0},{REJECT,0},{REJECT,0},{SHIFT,10},{REJECT,0}}, //9
        {{REJECT,0},{REDUCE,6},{REDUCE,6},{REDUCE,6},{REDUCE,6}}, //10
    }; // L          X          LP         RP         END


    std::vector<int> stack = {0};
    std::vector<Parser::Token> token_stack;
    std::vector<std::shared_ptr<Term>> term_stack;
    size_t lambda_distance = 0;

    auto next_token = [&]() {
        Parser::Token token;
        read_token(in, token);
        token_stack.push_back(token);
        switch(token.type) {
            case Parser::LEFT_PAREN:
                return LP;
            case Parser::RIGHT_PAREN:
                return RP;
            case Parser::LAMBDA:
                lambda_distance++;
                return L;
            case Parser::INDEX:
                return X;
            case Parser::IDENTIFIER:
                return X;
            case Parser::Type::END:
                return END;
        }
        return END;
    };

    int t = next_token();

    // define and comments are not in the grammar, but added as special cases
    //////////////////////////////////////////////////
    if (t == END)
        return nullptr;

    bool define = false;
    std::string define_identifier;

    if (token_stack.back().type == IDENTIFIER && token_stack.back().identifier[0] == ';')
        return nullptr;

    if (token_stack.back().type == IDENTIFIER && token_stack.back().identifier == "define") {
        define = true;
        t = next_token();
        if (token_stack.back().type == IDENTIFIER) {
            define_identifier = token_stack.back().identifier;
            t = next_token();
        } else {
            throw std::runtime_error("Expected an identifier after define");
        }
    }
    //////////////////////////////////////////////////

    bool looping = true;
    while (looping) {
        int state = stack.back();
        int action = action_table[state][t][0];
        int action_num = action_table[state][t][1];

        //for (int s : stack)
        //    cout << s << ' ';
        //cout << endl;
        //switch (t) {
        //    case L:
        //        cout << 'L';
        //        break;
        //    case X:
        //        cout << 'x';
        //        break;
        //    case LP:
        //        cout << '(';
        //        break;
        //    case RP:
        //        cout << ')';
        //        break;
        //    case END:
        //        cout << '$';
        //        break;
        //}
        //cout << " | ";

        switch (action) {
            case SHIFT:
                //cout << "s" << action_num;
                t = next_token();
                stack.push_back(action_num);
                break;
            case REDUCE: {
                //cout << "r" << action_num;
                stack.resize(stack.size() + reduce_num[action_num]);
                int nonterm = reduce_nonterm[action_num];
                int new_state = goto_table[stack.back()][nonterm];
                if (new_state == null_state)
                    throw std::runtime_error("Unexpected '" + token_stack.back().str + "'");

                stack.push_back(new_state);
                break;}
            case ACCEPT:
                //cout << "a";
                looping = false;
                break;
            case REJECT:
                //cout << "r";
                throw std::runtime_error("Unexpected '" + token_stack.back().str + "'");
                break;
        }
        //cout << endl << endl;

        if (action == REDUCE) {
            assert(token_stack.size() >= 1);
            Parser::Token lookahead = token_stack.back();
            token_stack.pop_back();


            // (1) E -> L E
            if (action_num == 1) {
                assert(term_stack.size() >= 1);
                auto body = term_stack.back();
                term_stack.pop_back();
                lambda_distance--;

                term_stack.push_back(std::make_shared<AbstractionTerm>(body));
            }
            // (3) A -> A I
            if (action_num == 3) {
                assert(term_stack.size() >= 2);
                auto right = term_stack.back();
                term_stack.pop_back();
                auto left = term_stack.back();
                term_stack.pop_back();
               
                term_stack.push_back(std::make_shared<ApplicationTerm>(left, right));
            }
            // (5) I -> x
            if (action_num == 5) {
                assert(token_stack.size() >= 1);
                Parser::Token token = token_stack.back();
                token_stack.pop_back();

                //cout << token.str << endl;
                assert(token.type == Parser::INDEX || token.type == Parser::IDENTIFIER);

                if (token.type == Parser::IDENTIFIER) {
                    if (token.identifier == "define")
                        throw std::runtime_error("'define' can't be a variable name");

                    std::shared_ptr<Term> definition = context.get_definition(token.identifier);
                    if (definition == nullptr) { // just a variable
                        token.index = context.push_identifier(token.identifier);
                        token.index += lambda_distance;
                        term_stack.push_back(std::make_shared<VariableTerm>(token.index));
                    } else { // a definition
                        term_stack.push_back(definition->lift(0, lambda_distance));
                    }
                } else {
                    term_stack.push_back(std::make_shared<VariableTerm>(token.index));
                }
            }


            token_stack.push_back(lookahead);
        }
    }

    //for (auto t : term_stack) {
    //    t->print(cout, context, 0);
    //    cout << ", ";
    //}
    //cout << endl;

    assert(term_stack.size() == 1);

    if (define) {
        context.define(define_identifier, term_stack.back());
        return nullptr;
    }

    return term_stack.back();
}





