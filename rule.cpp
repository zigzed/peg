/*
 * rule.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */
#include "rule.h"
#include "expr.h"
#include "context.h"

namespace cxx {
namespace peg {

	rule::rule() :
			expr_(0), func_(0)
	{
	}

	rule::rule(const expr& e) :
			expr_(_util::get_expr(e)), func_(0)
	{
		func_ = _util::get_func(expr_);
	}

	rule::rule(const rule& r) :
			expr_(new _expr_ref(r)), func_(r.func_), refc_(r.refc_)
	{
	}

	rule::~rule()
	{
		if(refc_.unique()) {
			delete expr_;
			expr_ = 0;
		}
	}

	void rule::setup(FUNC f)
	{
		func_ = f;
	}

	/**
	 * rule 和 expr 的差别：
	 * rule 会处理空格规则
	 * rule 会处理可能的绑定
	 */
	bool rule::match(scanner& s, context& c) const
	{
		_context& ctx = dynamic_cast<_context& >(c);
		ctx.ws()->match(s, c);

		if(expr_ && !s.eof()) {
			size_t p = s.pos();
			// 备份当前的节点栈，用于解析失败时的回退处理
			size_t m = ctx.backup();
			bool   r = expr_->match(s, c);
			if(r) {
				ctx.ws()->match(s, c);
				if(func_)
					func_(cursor(&s, p), cursor(&s, s.pos()), c);
			}
			else {
				// 解析失败的时候需要回退已经生成的节点
				ctx.unwind(m);
			}
			return r;
		}
		return false;

//		_context& ctx = dynamic_cast<_context& >(c);
//		ctx.ws()->match(s, c);
//
//		if(expr_ && !s.eof()) {
//			size_t p = s.pos();
//			// 备份当前的节点栈，用于解析失败时的回退处理
//			size_t m = ctx.backup();
//			bool   r = false;
//
//			size_t e = ctx.get_cache(p, expr_);
//			switch(e) {
//			case -1:
//			default:
//				r = expr_->match(s, c);
//				ctx.add_cache(p, expr_, (r ? s.pos() : -2));
//				break;
//			case -2:
//				break;
//			}
//
//			if(r) {
//				ctx.ws()->match(s, c);
//				if(func_)
//					func_(cursor(&s, p), cursor(&s, s.pos()), c);
//			}
//			else {
//				// 解析失败的时候需要回退已经生成的节点
//				ctx.unwind(m);
//			}
//			return r;
//		}
//		return false;
	}

	////////////////////////////////////////////////////////////////////////////
	expr rule::operator * () const
	{
		return _util::new_expr(new _expr_star(new _expr_ref(*this)));
	}

	expr rule::operator + () const
	{
		return _util::new_expr(new _expr_plus(new _expr_ref(*this)));
	}

	expr rule::operator - () const
	{
		return _util::new_expr(new _expr_optional(new _expr_ref(*this)));
	}

	expr rule::operator & () const
	{
		return _util::new_expr(new _expr_and(new _expr_ref(*this)));
	}

	expr rule::operator ! () const
	{
		return _util::new_expr(new _expr_not(new _expr_ref(*this)));
	}

	////////////////////////////////////////////////////////////////////////////
	expr operator>> (const expr& e1, const expr& e2)
	{
		return _util::new_expr(
				new _expr_seq(
						_util::get_expr(e1),
						_util::get_expr(e2)));
	}

	expr operator | (const expr& e1, const expr& e2)
	{
		return _util::new_expr(
				new _expr_choice(
						_util::get_expr(e1),
						_util::get_expr(e2)));
	}

	expr _range(int lower, int upper)
	{
		return _util::new_expr(
				new _expr_set(lower, upper));
	}

	expr _char(char c)
	{
		return _util::new_expr(new _expr_chr(c));
	}

	expr _set(const char* s)
	{
		return _util::new_expr(
				new _expr_set(s));
	}

	expr _str(const char* s, bool nocase)
	{
		return _util::new_expr(
				new _expr_str(s, nocase));
	}

	expr _any()
	{
		return _util::new_expr(
				new _expr_any());
	}

	expr _repeat(const expr& e, int N)
	{
		return _util::new_expr(
				new _expr_repeat(
						_util::get_expr(e), N));
	}

	expr _trace(const expr& e, const char* m)
	{
		return _util::new_expr(
				new _trace_expr(
						_util::get_expr(e), m));
	}

	expr _trace(const rule& r, const char* m)
	{
		return _util::new_expr(
				new _trace_rule(r, m));
	}

}
}



