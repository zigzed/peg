/*
 * testpeg.cpp
 *
 *  Created on: Feb 13, 2014
 *      Author: wilbur
 */
#include "parser.h"
#include "rule.h"
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <cassert>
#include "sqlgrammar.h"

template<typename T >
struct expected {
};

template<>
struct expected<double > {
	bool operator()(double A, double B) {
		if((A - B) / B < 0.000001)
			return true;

		std::ostringstream msg;
		msg << "test case failed, expecting "
				<< A << " actual " << B;
		throw std::string(msg.str());
		return false;
	}
};

namespace test {
	using namespace cxx::peg;

	class expr_t : public _node {
	public:
		virtual double 	evaluate() = 0;
		virtual void	showtree() = 0;
	};

	class binary_expr_t : public expr_t {
	public:
		void	pusharg(const char* token, context& ctx) {
			assert(ctx.node_len() >= 2);
			rhs = dynamic_cast<expr_t* >(ctx.get_node());
			lhs = dynamic_cast<expr_t* >(ctx.get_node());
		}
	protected:
		expr_t*	lhs;
		expr_t* rhs;
	};

	class add_t : public binary_expr_t {
	public:
		double	evaluate() {
			return lhs->evaluate() + rhs->evaluate();
		}
		void	showtree() {
			printf("+\n");
			printf("  ");
			lhs->showtree();
			printf("  ");
			rhs->showtree();
		}
	};

	class sub_t : public binary_expr_t {
	public:
		double	evaluate() {
			return lhs->evaluate() - rhs->evaluate();
		}
		void	showtree() {
			printf("-\n");
			printf("  ");
			lhs->showtree();
			printf("  ");
			rhs->showtree();
		}
	};

	class mul_t : public binary_expr_t {
	public:
		double	evaluate() {
			return lhs->evaluate() * rhs->evaluate();
		}
		void	showtree() {
			printf("*\n");
			printf("  ");
			lhs->showtree();
			printf("  ");
			rhs->showtree();
		}
	};

	class div_t : public binary_expr_t {
	public:
		double	evaluate() {
			return lhs->evaluate() / rhs->evaluate();
		}
		void	showtree() {
			printf("/\n");
			printf("  ");
			lhs->showtree();
			printf("  ");
			rhs->showtree();
		}
	};

	class val_t : public expr_t {
	public:
		void	pusharg(const char* token, context& ctx) {
			val_ = atof(token);
		}
		double	evaluate() {
			return val_;
		}
		void	showtree() {
			printf("%f\n", val_);
		}
	private:
		double val_;
	};

}

void test1()
{
	using namespace cxx::peg;

	rule ws		= (*expr(' '));
	rule digit	= TRACE_PEG(_range('0', '9'));
	rule number	= TRACE_PEG(-(_set("+-")) >> +digit >> -('.' >> +digit >> -(_set("eE") >> -_set("+-") >> +digit)));

	node_builder<test::val_t >	ast_num(number);

	std::vector<error > e;
	_node*	r;
	{
		parser	p("3");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3, d);
	}
	{
		parser	p("3.14");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.14, d);
	}
	{
		parser	p("3.14e18");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.14e18, d);
	}
	{
		parser	p("3.14e-3");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.14e-3, d);
	}
	{
		parser	p("3.14e+18");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.14e18, d);
	}
	{
		parser	p(" 3.14");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.14, d);
	}
	{
		parser	p("3.14 ");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.14, d);
	}
	{
		parser	p(" 3.14 ");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.14, d);
	}
	{
		parser	p(" -3.14");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(-3.14, d);
	}
	{
		parser	p("+3.14 ");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.14, d);
	}
	{
		parser	p(" -3.14 ");
		r = p.parse(number, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(-3.14, d);
	}
}

void test2()
{
	using namespace cxx::peg;

	rule expression;
	rule ws		= *expr(' ');
	rule digit		= _range('0', '9');
	rule number	= -(_set("+-")) >> +digit >> -('.' >> +digit >> -(_set("eE") >> -_set("+-") >> +digit));
	rule value		= number | '(' >> expression >> ')';

	rule mul_op	= '*' >> value;
	rule div_op	= '/' >> value;
	rule mul		= value >> *(mul_op | div_op);
	rule add_op	= '+' >> mul;
	rule sub_op	= '-' >> mul;
	rule add	= mul >> *(add_op | sub_op);
	expression = add;

	node_builder<test::val_t >	ast_num(number);
	node_builder<test::add_t >	ast_add(add_op);
	node_builder<test::sub_t >	ast_sub(sub_op);
	node_builder<test::mul_t >	ast_mul(mul_op);
	node_builder<test::div_t >	ast_div(div_op);

	std::vector<error>		e;
	_node* r;
	{
		printf("1+2\n");
		parser	p("1+2");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.0, d);
	}
	{
		printf(" 1 + 2 \n");
		parser	p(" 1 + 2 ");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.0, d);
	}
	{
		printf("1+(2)\n");
		parser	p("1+(2)");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.0, d);
	}
	{
		printf(" 1 +(2) \n");
		parser	p(" 1 +(2) ");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.0, d);
	}
	{
		printf("(1+2)+3\n");
		parser	p("(1+2)+3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(6.0, d);
	}
	{
		printf("1+2+3\n");
		parser	p("1+2+3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(6.0, d);
	}
	{
		printf("1+ (2+ 3)\n");
		parser	p("1+ (2+ 3)");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(6.0, d);
	}
	{
		printf("1+2*3\n");
		parser	p("1+2*3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(7.0, d);
	}
	{
		printf("(1+2)*3\n");
		parser	p("(1+2)*3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(9.0, d);
	}
	{
		printf("1*(2+3)\n");
		parser	p("1*(2+3)");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(5.0, d);
	}
	{
		printf("3/(1+2)\n");
		parser	p("3/(1+2)");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(1.0, d);
	}
	{
		printf("3/1+2\n");
		parser	p("3/1+2");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(5.0, d);
	}
	{
		printf("3-1/2\n");
		parser	p("3-1/2");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(2.5, d);
	}
	{
		printf("1-2-3\n");
		parser	p("1-2-3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(-4.0, d);
	}
}
//
namespace test {

	using namespace cxx::peg;

	class func_t : public _node {
	public:
		virtual std::string	evaluate() { return ""; }
	};

	class begin_paren_t : public _node {
	public:
		void pusharg(const char* token, context& ctx) {}
	};

	class end_paren_t : public _node {
	public:
		void pusharg(const char* token, context& ctx) {}
	};

	class iden_t : public func_t {
	public:
		void	pusharg(const char* token, context& ctx) {
			name_ = token;
		}
		std::string evaluate() {
			return name_;
		}
	private:
		std::string	name_;
	};

	class call_t : public func_t {
	public:
		void	pusharg(const char* token, context& ctx) {
			assert(ctx.node_len() >= 3);
			end_paren_t* e = dynamic_cast<end_paren_t* >(ctx.get_node());
			assert(e != NULL);

			while(ctx.node_len() > 0) {
				_node* arg = ctx.get_node();
				if(dynamic_cast<begin_paren_t* >(arg) != 0) {
					break;
				}
				else {
					args_.push_back(arg);
				}
			}

			_node* n = ctx.get_node();
//			printf("%s\n", ((func_t* )n)->evaluate().c_str());
			iden_t* name = dynamic_cast<iden_t* >(n);
			assert(name != NULL);
			name_ = name->evaluate();
		}

		std::string evaluate() {
			std::string res = name_;
			res += "(";
			for(size_t i = 0; i < args_.size(); ++i) {
				func_t* f = dynamic_cast<func_t* >(args_[args_.size() - i - 1]);
				if(f != NULL) {
					res += f->evaluate();
					if(i != args_.size() - 1) {
						res += ",";
					}
				}
			}
			res += ")";
			return res;
		}
	private:
		std::string				name_;
		std::vector<_node* >	args_;
	};

}

void test3()
{
	using namespace cxx::peg;

	rule expression;
	rule ws				= *expr(' ');
	rule letter			= _range('A', 'Z') | _range('a', 'z');
	rule digit			= _range('0', '9');
	rule identifier	= +letter >> *('_' | letter | digit);
	rule begin_paren	= expr('(');
	rule end_paren		= expr(')');
	rule function		= TRACE_PEG(identifier) >> TRACE_PEG(begin_paren) >> *(expression >> *(',' >> expression)) >> TRACE_PEG(end_paren);
	expression			= TRACE_PEG(function);

	// 让函数中的()也能够生成节点，这样可以支持变参数的函数
	node_builder<test::begin_paren_t> 		ast_begin_paren(begin_paren);
	node_builder<test::end_paren_t >		ast_end_paren(end_paren);
	node_builder<test::iden_t >				ast_ids(identifier);
	node_builder<test::call_t >				ast_fun(function);

	std::vector<error >		e;
	_node*	r;
	{
		parser	p("a()");
		r = p.parse(expression, ws, e);
		std::string d = (dynamic_cast<test::func_t* >(r))->evaluate();
	}
	{
		parser	p("a(b())");
		r = p.parse(expression, ws, e);
		std::string d = (dynamic_cast<test::func_t* >(r))->evaluate();
	}
	{
		parser	p("a(b(), c())");
		r = p.parse(expression, ws, e);
		std::string d = (dynamic_cast<test::func_t* >(r))->evaluate();
	}
	{
		parser	p("a(b(), c(d()))");
		r = p.parse(expression, ws, e);
		std::string d = (dynamic_cast<test::func_t* >(r))->evaluate();
	}
	{
		parser	p("a(b(e()), c(d()))");
		r = p.parse(expression, ws, e);
		std::string d = (dynamic_cast<test::func_t* >(r))->evaluate();
	}
	{
		parser	p("a( b ( e( )), c(d()))");
		r = p.parse(expression, ws, e);
		std::string d = (dynamic_cast<test::func_t* >(r))->evaluate();
	}
}

void test4()
{
	using namespace cxx::peg;

	rule expression, add_op, sub_op, mul_op, div_op, prod;
	rule ws		= *_set(" \t\r\n");
	rule digit	= _range('0', '9');
	rule number	= -(_set("+-")) >> +digit >> -('.' >> +digit >> -(_set("eE") >> -_set("+-") >> +digit));
	rule value	= number | '(' >> expression >> ')';

	expression 	= TRACE_PEG(add_op) | TRACE_PEG(sub_op) | prod;
	add_op 		= prod >> '+' >> expression;
	sub_op 		= prod >> '-' >> expression;
	prod		= TRACE_PEG(mul_op) | TRACE_PEG(div_op) | TRACE_PEG(value);
	mul_op		= value >> '*' >> prod;
	div_op		= value >> '/' >> prod;

//	rule expression, mul, add;
//	rule ws		= *_set(" \t\r\n");
//	rule digit	= _range('0', '9');
//	rule number	= -(_set("+-")) >> +digit >> -('.' >> +digit >> -(_set("eE") >> -_set("+-") >> +digit));
//	rule value	= number | '(' >> expression >> ')';
//
//	rule mul_op = mul >> '*' >> value;
//	rule div_op = mul >> '/' >> value;
//	mul = TRACE_PEG(value) | TRACE_PEG(mul_op) | TRACE_PEG(div_op);
//
//	rule add_op = add >> '+' >> mul;
//	rule sub_op = add >> '-' >> mul;
//	add = TRACE_PEG(mul) | TRACE_PEG(add_op) | TRACE_PEG(sub_op);
//	expression = TRACE_PEG(add);

	node_builder<test::val_t >	ast_num(number);
	node_builder<test::add_t >	ast_add(add_op);
	node_builder<test::sub_t >	ast_sub(sub_op);
	node_builder<test::mul_t >	ast_mul(mul_op);
	node_builder<test::div_t >	ast_div(div_op);

	std::vector<error > e;
	_node*	r;
	{
		parser	p("1+2");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.0, d);
	}
	{
		parser	p(" 1 + 2 ");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.0, d);
	}
	{
		parser	p("1+(2)");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.0, d);
	}
	{
		parser	p(" 1 +(2) ");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(3.0, d);
	}
	{
		parser	p("(1+2)+3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(6.0, d);
	}
	{
		parser	p("1+2+3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(6.0, d);
	}
	{
		parser	p("1+ (2+ 3)");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(6.0, d);
	}
	{
		parser	p("1+2*3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(7.0, d);
	}
	{
		parser	p("(1+2)*3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(9.0, d);
	}
	{
		parser	p("1*(2+3)");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(5.0, d);
	}
	{
		parser	p("3/(1+2)");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(1.0, d);
	}
	{
		parser	p("3/1+2");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(5.0, d);
	}
	{
		parser	p("3-1/2");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(2.5, d);
	}
	{
		parser	p("1-2-3");
		r = p.parse(expression, ws, e);
		double d = (dynamic_cast<test::expr_t* >(r))->evaluate();
		expected<double >()(-4.0, d);
	}
}

namespace test {

	using namespace cxx::peg;

	class sql_t : public _node {
	public:
		virtual std::string	evaluate() { return ""; }
	};

	class decimal_t : public sql_t {
	public:
		void pusharg(const char* token, context& ctx) {
			val_ = token;
		}
		std::string evaluate() {
			return val_;
		}
	private:
		std::string val_;
	};

	class select_t : public sql_t {
	public:
		void pusharg(const char* token, context& ctx) {}
	private:
		std::vector<std::string >	columns_;
		std::string					from_;
		std::vector<std::string >	groups_;
		std::vector<std::string >	orders_;

	};

	class from_t : public sql_t {
	public:
		void pusharg(const char* token, context& ctx) {}
	};

	class where_t : public sql_t {
	public:
		void pusharg(const char* token, context& ctx) {}
	};

	class group_t : public sql_t {
	public:
		void pusharg(const char* token, context& ctx) {}
	};

	class order_t : public sql_t {
	public:
		void pusharg(const char* token, context& ctx) {}
	};

	class limit_t : public sql_t {
	public:
		void pusharg(const char* token, context& ctx) {}
	};
}

//void test6()
//{
//	using namespace cxx::peg;
//
//	rule sql_stmt;
//	rule select			= _str("SELECT", true);
//	rule from			= _str("FROM", true);
//	rule where			= _str("WHERE", true);
//	rule group			= _str("GROUP", true);
//	rule order			= _str("ORDER", true);
//	rule limit			= _str("LIMIT", true);
//	rule by				= _str("BY", true);
//
//	rule ws				= *expr(' ');
//	rule digit			= _range('0', '9');
//	rule decimal		= +digit;
//	rule letter			= _range('a', 'z') | _range('A', 'Z');
//	rule identifier		= +letter >> *('_' | letter | digit);
//	rule database_name	= identifier;
//	rule table_name		= identifier;
//
//	rule table			= -(database_name >> '.') >> table_name;
//
//	rule unary_op, binary_op;
//	rule predicate		= identifier | decimal | unary_op | binary_op;
//	rule unary_op		= ('-' | '+' | '!' | _str("NOT", true)) >> predicate;
//	rule binary_op		= predicate >> ('+' | '-' | '*' | '/' | '%') >> predicate;
//
//	rule result_column	= '*' | identifier;
//	rule result_columns	= result_column >> *(',' >> result_column);
//	rule where_stmt		= where >> predicate;
//	rule from_stmt		= from >> table;
//	rule group_stmt		= group >> by >> result_columns;
//	rule order_stmt		= order >> by >> result_columns;
//	rule limit_stmt		= limit >> decimal >> -(',' >> decimal);
//
//	rule select_stmt	= select >>
//			(result_column >> *(',' >> result_column)) >>
//			-from_stmt >>
//			-where_stmt >>
//			-group_stmt
//			-order_stmt
//			-limit_stmt;
//
//	node_builder<test::decimal_t >	ast_decimal(decimal);
//	node_builder<test::select_t >	ast_select(select_stmt);
//	node_builder<test::from_t >		ast_from(from_stmt);
//	node_builder<test::where_t >	ast_where(where_stmt);
//	node_builder<test::group_t >	ast_group(group_stmt);
//	node_builder<test::order_t >	ast_order(order_stmt);
//	node_builder<test::limit_t >	ast_limit(limit_stmt);
//
//	std::vector<error > e;
//	_node*	r;
//
//	{
//		parser	p("select A from B");
//		r = p.parse(select_stmt, ws, e);
//		(dynamic_cast<test::expr_t* >(r))->evaluate();
//	}
//}

void test7()
{
	using namespace cxx::peg;

	rule expression, add_op, sub_op, mul_op, div_op, prod;
	rule ws		= *_set(" \t\r\n");
	rule digit	= _range('0', '9');
	rule number	= +digit;

	/**
	 * @NOTE: 注意下面的表达式是错误的，因为 '+' | '-' 会调用 C++ 的 bitwise or 操作：
	 * rule add1	= number >> *(('+' | '-') >> number);
	 * 正确的方式是采用 _set 或者用下面的方式：
	 * rule add1	= number >> *((expr('+') | expr('-')) >> number);
	 * 或者：
	 * rule add1	= number >> *((_char('+') | _char('-')) >> number);
	 */
	rule add1	= number >> *((expr('+') | expr('-')) >> number);
	rule add2	= number >> *(-_set("+-") >> number);

	std::vector<error >	e;
	_node*				r;
	{
		std::vector<error >	e1;
		std::vector<error >	e2;
		parser	p("1+2");
		_node* r1 = p.parse(add1, ws, e1);
		_node* r2 = p.parse(add2, ws, e2);
		printf("r1:%p, e1:%d, r2:%p, e2:%d\n", r1, e1.size(), r2, e2.size());
		for(size_t i = 0; i < e1.size(); ++i)
			printf("e1[%d]: %s\n", i, e1[i].msg.c_str());
		for(size_t i = 0; i < e2.size(); ++i)
			printf("e2[%d]: %s\n", i, e2[i].msg.c_str());
	}

}

int main(int argc, char* argv[])
{
//	test1();
//	test2();
//	test3();
//	test4();
////	test6();
//	test7();
	test_sql();
	return 0;
}



