#include "term.h"
#include "parser.h"
#include <sstream>
#include <fstream>
#include <unistd.h>

using namespace std;

shared_ptr<Term> eval(shared_ptr<Term> t, const Context& context) {
    shared_ptr<Term> prev = nullptr;
    while(prev == nullptr || !prev->alpha_equivalent(t)) {
        prev = t;
        t = t->beta_reduce();
        //t->print(cout, context, 0); cout<<endl;
    }
    return t;
}


void repl() {
    Context context;

    istringstream sin("none");

    context.define("out", Parser::parse(sin, context));

    while (true) {
        cout << "> ";
        std::string command;
        std::getline(cin, command);

        if (command == "quit")
            break;

        if (command == "context") {
            context.print(cout);
            cout << endl;
            continue;
        }

        if (command == "help") {
            cout << "Commands:" << endl;
            cout << "context" << endl;
            cout << "quit" << endl;
            cout << "help" << endl;
            cout << "lambda_expression" << endl;
            cout << "define identifier lambda_expression" << endl;
            cout << endl;
            cout << "Examples\n"
                << "\t> define identity $ #0\n"
                << "\t> identity x\n"
                << "\tx\n"
                << endl
                << "This expression reduces to itself\n"
                << "\t> ($ #0 #0) ($ #0 #0)\n"
                << "\t($ #0 #0) ($ #0 #0)\n"
                << endl
                << "Enter 'out' to reduce the previous expression further\n"
                << "Y combinator\n"
                << "\t> $ ($ #1 (#0 #0)) ($ #1 (#0 #0))\n"
                << "\t$ #0 (($ #1 (#0 #0)) ($ #1 (#0 #0)))\n"
                << "\t> out\n"
                << "\t$ #0 (#0 (($ #1 (#0 #0)) ($ #1 (#0 #0))))\n"
                << "\t> out\n"
                << "\t$ #0 (#0 (#0 (($ #1 (#0 #0)) ($ #1 (#0 #0)))))\n";
            cout << endl;
            continue;
        }


        std::shared_ptr<Term> exp = nullptr;
        try {
            sin = istringstream(command);
            exp = Parser::parse(sin, context);
        } catch (const std::runtime_error& e) {
            cout << "Error: " << e.what();
        }

        if (exp) {
            exp = exp->beta_reduce();
            exp->print(cout, context, 0);
            context.define("out", exp);
        }
        cout << endl;
    }
}


void run(std::istream& in) {
    Context context;
    std::string line;
    size_t line_number = 1;
    while(std::getline(in, line)) {
        if (in.eof())
            break;

        istringstream sin(line);
        std::shared_ptr<Term> exp = nullptr;
        try {
            exp = Parser::parse(sin, context);
        } catch (const std::runtime_error& e) {
            cout << "Error on line " << line_number << ": ";
            cout << e.what() << endl;
            break;
        }

        if (exp) {
            exp = eval(exp, context);
            exp->print(cout, context, 0);
            cout << endl;
        }
        line_number++;
    }
}


int main(int argc, char **argv) {
    if (argc == 2) {
        std::ifstream fin(argv[1]);

        if (!fin.good()) {
            cout << "Couldn't open '" << argv[1] << "'" << endl;
            return -1;
        }
        run(fin);
    } else {
        repl();
    }

    return 0;
}
