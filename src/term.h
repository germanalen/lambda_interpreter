#ifndef TERM_H
#define TERM_H

#include <iostream>
#include <string>
#include <memory>
#include <cassert>
#include "context.h"
#include <utility>

//#define TERM_PRINT_ALL_PAREN

class Term {
public:
    enum Type {
        VARIABLE,
        ABSTRACTION,
        APPLICATION
    };
private:
    Type type;
protected:
    Term(Type type) : type{type} {}
public:
    Type get_type() const { return type; }

    virtual bool alpha_equivalent(std::shared_ptr<Term> other) const = 0;

    // these make changed copies
    virtual std::shared_ptr<Term> lift(size_t border, size_t distance) const = 0;
    virtual std::shared_ptr<Term>
    subst(size_t index, std::shared_ptr<Term> value, size_t lifting) const = 0;
    virtual std::shared_ptr<Term> beta_reduce() const = 0;
    

    // This didn't work
    //// these work in-place
    //static void lift(std::shared_ptr<Term> term, size_t border, size_t distance);
    //// returns new_expression
    //static std::shared_ptr<Term> 
    //    subst(std::shared_ptr<Term> term, size_t index, std::shared_ptr<Term> value, size_t lifting);
    //// returns {new_expression, reduced}
    //// reduction to itself counts too
    //static std::pair<std::shared_ptr<Term>, bool> beta_reduce(std::shared_ptr<Term> term);


    virtual void print(std::ostream& out, const Context& context, size_t distance) const = 0;

    Term(const Term& o) = delete;
    void operator=(const Term& o) = delete;
};



class VariableTerm : public Term {
public:
    VariableTerm(size_t index) : Term(VARIABLE), index{index} {}

    bool alpha_equivalent(std::shared_ptr<Term> other) const {
        assert(other != nullptr);
        if (other->get_type() != get_type()) return false;

        auto other_ = std::static_pointer_cast<VariableTerm>(other);

        return index == other_->index;
    }

    std::shared_ptr<Term> lift(size_t border, size_t distance) const override {
        if (index >= border)
            return std::make_shared<VariableTerm>(index + distance);
        return std::make_shared<VariableTerm>(index);
    }

    std::shared_ptr<Term> 
    subst(size_t index, std::shared_ptr<Term> value, size_t lifting) const override {
        assert(value != nullptr);
        if (this->index < index)
            return std::make_shared<VariableTerm>(this->index);
        if (this->index > index)
            return std::make_shared<VariableTerm>(this->index-1);
        return value->lift(0, lifting);
    }

    std::shared_ptr<Term> beta_reduce() const override {
        return std::make_shared<VariableTerm>(index);
    }

    void print(std::ostream& out, const Context& context, size_t distance) const override {
        if (index >= distance)
            //out << "'";
            out << context.get_identifier(index - distance);
            //out << "'";
        else
            out << '#' << index;
    }

    size_t index;
};


class AbstractionTerm : public Term {
public:
    AbstractionTerm(std::shared_ptr<Term> body) : 
        Term(ABSTRACTION),
        body{body}
    {
        assert(body != nullptr);
    }

    bool alpha_equivalent(std::shared_ptr<Term> other) const {
        assert(other != nullptr);
        if (other->get_type() != get_type()) return false;

        auto other_ = std::static_pointer_cast<AbstractionTerm>(other);

        return body->alpha_equivalent(other_->body);
    }

    std::shared_ptr<Term> lift(size_t border, size_t distance) const override {
        auto body_ = body->lift(border+1, distance);
        return std::make_shared<AbstractionTerm>(body_);
    }

    std::shared_ptr<Term> 
    subst(size_t index, std::shared_ptr<Term> value, size_t lifting) const override {
        assert(value != nullptr);
        auto body_ = body->subst(index+1, value, lifting+1);
        return std::make_shared<AbstractionTerm>(body_);
    }

    std::shared_ptr<Term> beta_reduce() const override {
        return std::make_shared<AbstractionTerm>(body->beta_reduce());
    }

    void print(std::ostream& out, const Context& context, size_t distance) const override {
        out << '$';
        if (body->get_type() != ABSTRACTION)
            out << ' ';

#ifdef TERM_PRINT_ALL_PAREN
        out << '(';
#endif

        body->print(out, context, distance+1);

#ifdef TERM_PRINT_ALL_PAREN
        out << ')';
#endif
    }

    std::shared_ptr<Term> body;
};


class ApplicationTerm : public Term {
public:
    ApplicationTerm(std::shared_ptr<Term> left, std::shared_ptr<Term> right) : 
        Term(APPLICATION), left{left}, right{right}
    {
        assert(left != nullptr);
        assert(right != nullptr);
    }

    bool alpha_equivalent(std::shared_ptr<Term> other) const {
        assert(other != nullptr);
        if (other->get_type() != get_type()) return false;

        auto other_ = std::static_pointer_cast<ApplicationTerm>(other);

        return left->alpha_equivalent(other_->left) && right->alpha_equivalent(other_->right);
    }

    std::shared_ptr<Term> lift(size_t border, size_t distance) const override {
        auto left_ = left->lift(border, distance);
        auto right_ = right->lift(border, distance);
        return std::make_shared<ApplicationTerm>(left_, right_);
    }

    std::shared_ptr<Term> 
    subst(size_t index, std::shared_ptr<Term> value, size_t lifting) const override {
        assert(value != nullptr);
        auto left_ = left->subst(index, value, lifting);
        auto right_ = right->subst(index, value, lifting);
        return std::make_shared<ApplicationTerm>(left_, right_);
    }

    std::shared_ptr<Term> beta_reduce() const override {
        if (left->get_type() == ABSTRACTION) {
            auto left_ = std::static_pointer_cast<AbstractionTerm>(left);
            return left_->body->subst(0, right, 0);
        }
        return std::make_shared<ApplicationTerm>(left->beta_reduce(), right->beta_reduce());
    }

    void print(std::ostream& out, const Context& context, size_t distance) const override {
        bool left_paren = left->get_type() == ABSTRACTION;
        bool right_paren = right->get_type() != VARIABLE;

#ifdef TERM_PRINT_ALL_PAREN
        left_paren = right_paren = true;
#endif

        if (left_paren) out << '(';
        left->print(out, context, distance);
        if (left_paren) out << ')';

        out << ' ';

        if (right_paren) out << '(';
        right->print(out, context, distance);
        if (right_paren) out << ')';
    }

    std::shared_ptr<Term> left, right;
};








#endif
