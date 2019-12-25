#include "term.h"

//void Term::lift(std::shared_ptr<Term> term, size_t border, size_t distance) {
//    if (term->get_type() == Term::VARIABLE) {
//        auto t = std::static_pointer_cast<VariableTerm>(term);
//        if (t->index >= border) {
//            t->index += distance;
//        }
//    } else if (term->get_type() == Term::ABSTRACTION) {
//        auto t = std::static_pointer_cast<AbstractionTerm>(term);
//        lift(t->body, border+1, distance);
//    } else { // if (Term::APPLICATION)
//        auto t = std::static_pointer_cast<ApplicationTerm>(term);
//        lift(t->left, border, distance);
//        lift(t->right, border, distance);
//    }
//}
//
//std::shared_ptr<Term> 
//Term::subst(std::shared_ptr<Term> term, size_t index, std::shared_ptr<Term> value, size_t lifting) {
//    assert(value != nullptr);
//    if (term->get_type() == Term::VARIABLE) {
//        auto t = std::static_pointer_cast<VariableTerm>(term);
//        if (t->index < index) {
//            return t;
//        }
//        if (t->index > index) {
//            t->index -= 1;
//            return t;
//        }
//        Term::lift(value, 0, lifting);
//        return value;
//    } else if (term->get_type() == Term::ABSTRACTION) {
//        auto t = std::static_pointer_cast<AbstractionTerm>(term);
//        subst(t->body, index+1, value, lifting+1);
//        return t;
//    } else { // if (Term::APPLICATION)
//        auto t = std::static_pointer_cast<ApplicationTerm>(term);
//        subst(t->left, index, value, lifting);
//        subst(t->right, index, value, lifting);
//        return t;
//    }
//}
//
//std::pair<std::shared_ptr<Term>, bool> Term::beta_reduce(std::shared_ptr<Term> term) {
//    if (term->get_type() == Term::VARIABLE) {
//        auto t = std::static_pointer_cast<VariableTerm>(term);
//        return {t, false};
//    } else if (term->get_type() == Term::ABSTRACTION) {
//        auto t = std::static_pointer_cast<AbstractionTerm>(term);
//        bool reduced;
//        std::tie(t->body, reduced) = beta_reduce(t->body);
//        return {t, reduced};
//    } else { // if (Term::APPLICATION)
//        auto t = std::static_pointer_cast<ApplicationTerm>(term);
//        if (t->left->get_type() == Term::ABSTRACTION) {
//            auto left = std::static_pointer_cast<AbstractionTerm>(t->left);
//            left->body = Term::subst(left->body, 0, t->right, 0);
//            return {left->body, true};
//        }
//        bool left_reduced, right_reduced;
//        std::tie(t->left, left_reduced) = beta_reduce(t->left);
//        std::tie(t->right, right_reduced) = beta_reduce(t->right);
//        return {t, left_reduced || right_reduced};
//    }
//}
//
///*
//    if (term->get_type() == Term::VARIABLE) {
//        auto t = std::static_pointer_cast<VariableTerm>(term);
//    } else if (term->get_type() == Term::ABSTRACTION) {
//        auto t = std::static_pointer_cast<AbstractionTerm>(term);
//    } else { // if (Term::APPLICATION)
//        auto t = std::static_pointer_cast<ApplicationTerm>(term);
//    }
//*/
