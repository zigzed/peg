/*
 * parser.h
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */

#ifndef CXX_PEG_RULE_H_
#define CXX_PEG_RULE_H_
#include <vector>
#include <string>
#include <cstddef>
#include "refcnt.h"
#include "scanner.h"

namespace cxx {
namespace peg {

	class _node;
	class _util;
	class _expr;
	class expr;
	class rule;
	class scanner;

	struct cursor {
		scanner*	ifs;
		size_t		pos;
		cursor() : ifs(0), pos(0) {}
		cursor(scanner* s, size_t p) :
			ifs(s), pos(p) {}
	};

	struct error {
		cursor		pos;
		std::string	msg;
		error(const cursor& p, const std::string& m) :
			pos(p), msg(m) {}
	};

	class context {
	public:
		virtual ~context();
		/** add an ast node */
		virtual void	add_node(_node* n) = 0;
		/** get the parsed ast node */
		virtual _node*	get_node() = 0;
		/** the ast node size */
		virtual size_t	node_len() const = 0;

		/** get the parsing error infomation */
		virtual size_t	get_error(std::vector<error >& e) = 0;
	};

	/**
	 * expr 是一个最基础的表达式
	 */
	class expr {
	public:
		expr(int c);
		expr(const char* s, bool nocase = false);
		expr(const rule& r);
		/// 重复0个或者多个表达式
		expr operator* () const;
		/// 重复1个或者多个表达式
		expr operator+ () const;
		/// 可选（0个或者1个）
		expr operator- () const;
		/// 断言表达式
		expr operator& () const;
		/// 非断言表达式
		expr operator! () const;
	private:
		friend class _util;

		_expr*	expr_;

		expr(_expr* e);
		expr& operator= (const expr& rhs);
	};

	/**
	 * rule 表示一条规则定义，rule 和 expr 的主要差别：
	 *  rule会处理空格规则
	 *  rule会处理绑定的函数规则，生成节点(expr不会生成节点)
	 */
	class rule {
	public:
		typedef void (*FUNC)(const cursor& begin, const cursor& end, context& ctx);

		rule();
		rule(const expr& e);
		rule(const rule& rhs);
		~rule();

		void setup(FUNC f);
		bool match(scanner& s, context& c) const;

		expr operator* () const;
		expr operator+ () const;
		expr operator- () const;
		expr operator& () const;
		expr operator! () const;
	private:
		friend class _util;

		_expr* 	expr_;
		FUNC	func_;
		refcnt	refc_;
	};

	/** sequence of e1 and e2 */
	expr operator >> (const expr& e1, const expr& e2);
	/** choice of e1 or e2 */
	expr operator |  (const expr& e1, const expr& e2);
	/** a range from lower to upper */
	expr _range(int lower, int upper);
	expr _char(char c);
	/** a set of characters */
	expr _set(const char* s);
	expr _str(const char* s, bool nocase = false);
	/** any one character */
	expr _any();
	/** repeat an expression for N times */
	expr _repeat(const expr& e, int N);

	expr _trace(const expr& e, const char* m);
	expr _trace(const rule& r, const char* m);

	/** node_builder 用户关联规则定义和表达式代码，只有通过 node_builder 定义的规则
	 *  才会生成对应的代码
	 */
	template<typename T >
	class node_builder {
	public:
		node_builder(rule& r) {
			r.setup(builder);
		}
	private:
		static void builder(const cursor& begin, const cursor& end, context& ctx) {
			T* ptr = new T;
			ptr->begin 	= begin;
			ptr->end	= end;
			ptr->pusharg(begin.ifs->str(begin.pos, end.pos).c_str(), ctx);
			ctx.add_node(ptr);
		}
	};

#define	TRACE_PEG(e)	_trace((e), #e)

}
}

#endif /* CXX_PEG_RULE_H_ */
