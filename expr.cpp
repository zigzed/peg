/*
 * expr.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */
#include "expr.h"
#include "rule.h"
#include <stdio.h>

namespace cxx {
namespace peg {

	int expr_state_id::the_next_id = 1;

	int expr_state_id::getid()
	{
		return the_next_id++;
	}

	void expr_state_id::reset()
	{
		the_next_id = 1;
	}

	_expr::_expr() :
			identifier_(expr_state_id::getid())
	{
	}

	int _expr::getid() const
	{
		return identifier_;
	}

	////////////////////////////////////////////////////////////////////////////
	expr::expr(int c) :
			expr_(new _expr_chr(c))
	{
	}

	expr::expr(const char* s, bool nocase) :
			expr_(new _expr_str(s, nocase))
	{
	}

	expr::expr(const rule& r) :
			expr_(new _expr_ref(r))
	{
	}

	expr::expr(_expr* e) :
			expr_(e)
	{
	}

	expr expr::operator *() const
	{
		return _util::new_expr(new _expr_star(expr_));
	}

	expr expr::operator +() const
	{
		return _util::new_expr(new _expr_plus(expr_));
	}

	expr expr::operator -() const
	{
		return _util::new_expr(new _expr_optional(expr_));
	}

	expr expr::operator &() const
	{
		return _util::new_expr(new _expr_and(expr_));
	}

	expr expr::operator !() const
	{
		return _util::new_expr(new _expr_not(expr_));
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_chr::_expr_chr(int c) :
			char_(c)
	{
	}

	bool _expr_chr::match(scanner& s, context& c) const
	{
		if(!s.eof()) {
			int c = s.top();
			if(c == char_) {
				s.advance(1);
				return true;
			}
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////////////
	static int strcmp_nocase(const std::string& lhs, const std::string& rhs)
	{
		size_t ll = lhs.size();
		size_t lr = rhs.size();
		size_t c = 0;
		while(c < ll && c < lr) {
			int lc = std::toupper(lhs[c]);
			int rc = std::toupper(rhs[c]);
			if(lc == rc) {
				++c;
			}
			else {
				return lc - rc;
			}
		}
		return ((ll > lr) ? 1 : ((ll == lr) ? 0 : -1));
	}

	_expr_str::_expr_str(const char* s, bool nocase) :
			str_(s), nocase_(nocase)
	{
	}

	bool _expr_str::match(scanner& s, context& c) const
	{
		if(s.eof())
			return false;

		std::string str = s.str(s.pos(), s.pos() + str_.size());
		if((!nocase_ && str == str_) ||
				(nocase_ && strcmp_nocase(str, str_) == 0)){
			s.advance(str_.size());
			return true;
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_set::_expr_set(const char* s)
	{
		for(; *s; ++s)
			do_add(*s);
	}

	_expr_set::_expr_set(int lower, int upper)
	{
		set_.resize((size_t)upper + 1);
		for(int i = lower; i < upper; ++i)
			do_add(i);
	}

	void _expr_set::do_add(size_t c)
	{
		if(c >= set_.size())
			set_.resize(c + 1);
		set_[c] = true;
	}

	bool _expr_set::match(scanner& s, context& c) const
	{
		if(!s.eof()) {
			size_t c = s.top();
			if(c < set_.size() && set_[c]) {
				s.advance(1);
				return true;
			}
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_unary::_expr_unary(_expr* e) :
			e_(e)
	{
	}

	_expr_unary::~_expr_unary()
	{
		delete e_;
		e_ = NULL;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_binary::_expr_binary(_expr* e1, _expr* e2) :
			e1_(e1), e2_(e2)
	{
	}

	_expr_binary::~_expr_binary()
	{
		delete e1_;
		delete e2_;
		e1_ = NULL;
		e2_ = NULL;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_term::_expr_term(_expr* e) :
			_expr_unary(e)
	{
	}

	bool _expr_term::match(scanner& s, context& c) const
	{
		if(s.eof())
			return false;
		if(e_)
			return e_->match(s, c);
		return false;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_star::_expr_star(_expr* e) :
			_expr_unary(e)
	{
	}

	bool _expr_star::match(scanner& s, context& c) const
	{
		if(!s.eof()) {
			while(e_->match(s, c)) {
			}
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_plus::_expr_plus(_expr* e) :
			_expr_unary(e)
	{
	}

	bool _expr_plus::match(scanner& s, context& c) const
	{
		if(s.eof())
			return false;
		if(!e_->match(s, c))
			return false;
		if(!s.eof()) {
			while(e_->match(s, c)) {
			}
		}
		return true;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_optional::_expr_optional(_expr* e) :
			_expr_unary(e)
	{
	}

	bool _expr_optional::match(scanner& s, context& c) const
	{
		if(!s.eof())
			e_->match(s, c);
		return true;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_repeat::_expr_repeat(_expr* e, int N) :
			_expr_unary(e), repeat_(N)
	{
	}

	bool _expr_repeat::match(scanner& s, context& c) const
	{
		if(s.eof())
			return false;
		int N = 0;
		size_t p = s.pos();
		while(N < repeat_ && e_->match(s, c)) {
			N++;
		}
		if(N == repeat_) {
			return true;
		}
		else {
			s.pos(p);
			return false;
		}
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_seq::_expr_seq(_expr* e1, _expr* e2) :
			_expr_binary(e1, e2)
	{
	}

	bool _expr_seq::match(scanner& s, context& c) const
	{
		bool   r = false;
		size_t p = s.pos();
		if(!s.eof()) {
			if(e1_->match(s, c)) {
				r = e2_->match(s, c);
			}
		}
		if(!r)
			s.pos(p);

		return r;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_choice::_expr_choice(_expr* e1, _expr* e2) :
			_expr_binary(e1, e2)
	{
	}

	bool _expr_choice::match(scanner& s, context& c) const
	{
		if(s.eof())
			return false;
		if(e1_->match(s, c))
			return true;
		return e2_->match(s, c);
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_and::_expr_and(_expr* e) :
			_expr_unary(e)
	{
	}

	bool _expr_and::match(scanner& s, context& c) const
	{
		if(s.eof())
			return false;

		size_t 	p = s.pos();
		bool	r = e_->match(s, c);
		s.pos(p);
		return r;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_not::_expr_not(_expr* e) :
			_expr_unary(e)
	{
	}

	bool _expr_not::match(scanner& s, context& c) const
	{
		if(s.eof())
			return false;

		size_t 	p = s.pos();
		bool	r = e_->match(s, c);
		s.pos(p);
		return !r;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr_ref::_expr_ref(const rule& r) :
			r_(r)
	{
	}

	bool _expr_ref::match(scanner& s, context& c) const
	{
		if(s.eof())
			return false;

		return r_.match(s, c);
	}

	////////////////////////////////////////////////////////////////////////////
	bool _expr_any::match(scanner& s, context& c) const
	{
		if(!s.eof()) {
			s.pop();
			return true;
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////////////
	_trace_expr::_trace_expr(_expr* e, const char* msg) :
			e_(e), m_(msg)
	{
	}

	bool _trace_expr::match(scanner& s, context& c) const
	{
		static int LogMsgId = 0;
		int curLog = ++LogMsgId;
		size_t p = s.pos();
		printf("#%3d: matching expr %s for '%s'\n", curLog, m_.c_str(), s.str(p).c_str());
		bool b = e_->match(s, c);
		if(b) {
			printf("#%3d: matching expr %s for '%s' done\n", curLog, m_.c_str(), s.str(p, s.pos()).c_str());
		}
		else {
			printf("#%3d: matching expr %s for '%s' failed\n", curLog, m_.c_str(), s.str(p).c_str());
		}
		return b;
	}

	////////////////////////////////////////////////////////////////////////////
	_trace_rule::_trace_rule(const rule& r, const char* msg) :
				r_(r), m_(msg)
	{
	}

	bool _trace_rule::match(scanner& s, context& c) const
	{
		static int LogMsgId = 0;
		int curLog = ++LogMsgId;
		size_t p = s.pos();
		printf("#%3d: matching rule %s for '%s'\n", curLog, m_.c_str(), s.str(p).c_str());
		bool b = r_.match(s, c);
		if(b) {
			printf("#%3d: matching rule %s for '%s' done\n", curLog, m_.c_str(), s.str(p, s.pos()).c_str());
		}
		else {
			printf("#%3d: matching rule %s for '%s' failed\n", curLog, m_.c_str(), s.str(p).c_str());
		}
		return b;
	}

	////////////////////////////////////////////////////////////////////////////
	_expr* _util::get_expr(const expr& e)
	{
		return e.expr_;
	}

	_expr* _util::get_expr(const rule& r)
	{
		return r.expr_;
	}

	expr _util::new_expr(_expr* e)
	{
		return expr(e);
	}

	rule::FUNC _util::get_func(_expr* e)
	{
		_expr_ref* r = dynamic_cast<_expr_ref* >(e);
		if(r) {
			return r->r_.func_;
		}
		return 0;
	}

	////////////////////////////////////////////////////////////////////////////


}
}



