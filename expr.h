/*
 * expr.h
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */

#ifndef CXX_PEG_EXPR_H_
#define CXX_PEG_EXPR_H_
#include "rule.h"

namespace cxx {
namespace peg {

	class expr_state_id {
	public:
		static int  getid();
		static void reset();
	private:
		static int the_next_id;
	};

	class _expr {
	public:
		_expr();
		virtual ~_expr() {}
		int  		 		getid() const;
		virtual bool 		match(scanner& s, context& c) const = 0;
	private:
		int				identifier_;
	};

	class _expr_chr : public _expr {
	public:
		explicit _expr_chr(int c);
		bool match(scanner& s, context& c) const;
	private:
		int	char_;
	};

	class _expr_str : public _expr {
	public:
		_expr_str(const char* s, bool nocase);
		bool match(scanner& s, context& c) const;
	private:
		std::string str_;
		bool		nocase_;
	};

	class _expr_set : public _expr {
	public:
		explicit _expr_set(const char* s);
		_expr_set(int lower, int upper);
		bool match(scanner& s, context& c) const;
	private:
		void do_add(size_t c);

		std::vector<bool> set_;
	};

	class _expr_any : public _expr {
	public:
		bool match(scanner& s, context& c) const;
	};

	class _expr_unary : public _expr {
	public:
		_expr_unary(_expr* e);
		~_expr_unary();
	protected:
		_expr* e_;
	};

	class _expr_term : public _expr_unary {
	public:
		_expr_term(_expr* e);
		bool match(scanner& s, context& c) const;
	};

	class _expr_star : public _expr_unary {
	public:
		_expr_star(_expr* e);
		bool match(scanner& s, context& c) const;
	};

	class _expr_plus : public _expr_unary {
	public:
		_expr_plus(_expr* e);
		bool match(scanner& s, context& c) const;
	};

	class _expr_optional : public _expr_unary {
	public:
		_expr_optional(_expr* e);
		bool match(scanner& s, context& c) const;
	};

	class _expr_repeat : public _expr_unary {
	public:
		_expr_repeat(_expr* e, int N);
		bool match(scanner& s, context& c) const;
	private:
		int	repeat_;
	};

	/** 断言：用于判断后续的输入满足表达式 e，但是不实际处理输入 */
	class _expr_and : public _expr_unary {
	public:
		_expr_and(_expr* e);
		bool match(scanner& s, context& c) const;
	};

	/** 断言：用于判断后续的输入不满足表达式 e，但是不实际处理输入 */
	class _expr_not : public _expr_unary {
	public:
		_expr_not(_expr* e);
		bool match(scanner& s, context& c) const;
	};

	class _expr_binary : public _expr {
	public:
		_expr_binary(_expr* e1, _expr* e2);
		~_expr_binary();
	protected:
		_expr* e1_;
		_expr* e2_;
	};

	class _expr_seq : public _expr_binary {
	public:
		_expr_seq(_expr* e1, _expr* e2);
		bool match(scanner& s, context& c) const;
	};

	class _expr_choice : public _expr_binary {
	public:
		_expr_choice(_expr* e1, _expr* e2);
		bool match(scanner& s, context& c) const;
	};

	class _expr_ref : public _expr {
	public:
		_expr_ref(const rule& r);
		bool match(scanner& s, context& c) const;
	private:
		friend class _util;
		const rule& r_;
	};

	class _trace_expr : public _expr {
	public:
		_trace_expr(_expr* e, const char* msg);
		bool match(scanner& s, context& c) const;
	private:
		_expr*		e_;
		std::string m_;
	};

	class _trace_rule : public _expr {
	public:
		_trace_rule(const rule& r, const char* msg);
		bool match(scanner& s, context& c) const;
	private:
		const rule& r_;
		std::string m_;
	};

	class _util {
	public:
		static _expr* 		get_expr(const rule& r);
		static _expr* 		get_expr(const expr& e);
		static rule::FUNC	get_func(_expr* e);
		static expr			new_expr(_expr* e);
	};

}
}




#endif /* CXX_PEG_EXPR_H_ */
