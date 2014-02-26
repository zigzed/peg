/**
 * SQL 表达式解析
 * TODO:
 * 	子查询的支持
 * 	非select的支持（例如 insert/update/create/delete等）
 */
#include "rule.h"
#include "parser.h"
#include <stdio.h>

namespace test {



}

void test_sql()
{
	using namespace cxx::peg;

	rule expressions, expression;
	rule end_of_line	= _set("\r\n");
	rule comment		= _str("--") >> *(!end_of_line >> _any());

	// 注，下面采用宏，而不是 rule digit = xxx 的方式是为了避免将 digit/letter 作为 rule
	// 因为对于 rule 会自动处理期间的空格，会导致 identifier 解析错误
	// 如果采用 expr digit = xxx 的方式的话，可能会导致底层的指针重复释放。这是因为 expr 底层
	// 并没有跟踪指针的生存期。
#define	DIGIT			_range('0', '9')
#define LETTER			_range('a', 'z') | _range('A', 'Z')

	rule ws				= *_set(" \t\r\n") | comment;
	rule integer_const	= +DIGIT;
	rule double_const	= -(_set("+-")) >> +DIGIT >> -('.' >> +DIGIT >> -(_set("eE") >> -_set("+-") >> +DIGIT));
	rule boolean_const	= _str("true", true) | _str("false", true);
	rule string_const	= _char('"') >> *(!_char('"') >> _any()) >> _char('"');
	rule constant		=
			double_const |
			integer_const |
			boolean_const |
			string_const;

	rule begin_paren	= expr('(');
	rule end_paren		= expr(')');

	rule SELECT			= _str("SELECT", true);
	rule FROM			= _str("FROM", true);
	rule WHERE			= _str("WHERE", true);
	rule GROUP			= _str("GROUP BY", true);
	rule ORDER			= _str("ORDER BY", true);
	rule LIMIT			= _str("LIMIT", true);
	rule DISTINCT		= _str("DISTINCT", true);
	rule ASC			= _str("ASC", true);
	rule DESC			= _str("DESC", true);
	rule CASE			= _str("CASE", true);
	rule WHEN			= _str("WHEN", true);
	rule THEN			= _str("THEN", true);
	rule ELSE			= _str("ELSE", true);
	rule END			= _str("END", true);
	rule AND			= _str("AND", true);
	rule OR				= _str("OR", true);
	rule BETWEEN		= _str("BETWEEN", true);
	rule NOT			= _str("NOT", true);


	rule keyword		= SELECT | FROM | WHERE| GROUP | ORDER | LIMIT |
			DISTINCT | ASC | DESC;
	rule identifier		= +LETTER >> *('_' | LETTER | DIGIT);

	rule function		= identifier >> begin_paren >> expressions >> end_paren;
	rule atom_expr		=
			function |
			constant |
			'(' >> expression >> ')' |
			identifier |
			'*';
	rule unary_expr		= -_set("+-!") >> atom_expr;
	rule mul_op		= unary_expr >> *('*' >> unary_expr);
	rule div_op		= unary_expr >> *('/' >> unary_expr);
	rule mul_expr	= mul_op | div_op;
	rule add_op		= mul_expr >> *('+' >> mul_expr);
	rule sub_op		= mul_expr >> *('-' >> mul_expr);
	rule add_expr	= add_op | sub_op;
//	rule mul_expr		= TRACE_PEG(unary_expr) >> *(_set("*/") >> unary_expr);
//	rule add_expr		= TRACE_PEG(mul_expr) >> *(_set("+-") >> mul_expr);
	rule cmp_expr		= add_expr >>
			*((_str("=") | _str("<>") | _str(">") | _str("<") | _str(">=") | _str("<=")) >>
					add_expr
					);
	rule and_expr		= cmp_expr >> *(AND >> cmp_expr);
	rule or_expr		= and_expr >> *(OR >> and_expr);
	rule case_expr		=
			CASE >> -expression >>
			+(WHEN >> expression >> THEN >> expression) >>
			-(ELSE >> expression) >>
			END;
	rule between_expr	=
			BETWEEN >> expression >>
			AND >> expression;
	rule isnull_expr	=
			_str("ISNULL", true) |
			_str("NOTNULL", true) |
			_str("NOT NULL", true);
	rule is_expr		=
			_str("IS", true) >> -_str("NOT", true) >> expression;
	rule regex_expr		=
			-_str("NOT", true) >> 
			(_str("LIKE", true) |
			 _str("GLOB", true) |
			 _str("REGEXP", true) |
			 _str("MATCH", true)) >>
			expression >>
			-(_str("ESCAPE") >> expression);
	rule tail_expr		=
			between_expr |
			isnull_expr |
			is_expr |
			regex_expr;

	rule func_expr		=
			case_expr |
			between_expr;

	expression			= TRACE_PEG((case_expr | or_expr) >> -tail_expr);
	expressions			= expression >> *(',' >> expression);

	rule table_name		= identifier;
	rule fields_name	= -(table_name >> '.') >> TRACE_PEG(identifier);
	// expression 必须在 fields_name 前，因为函数的匹配规则和字段名类似
	rule fields_expr	= expression | fields_name;
	rule fields_list	= (fields_expr >> *(',' >> fields_expr));
	rule select_stmt	= SELECT >> fields_list;
	rule from_stmt		= FROM >> table_name;
	rule order_pair		= fields_name >> -(ASC | DESC);
	rule order_list		= order_pair >> *(',' >> order_pair);
	rule order_stmt		= ORDER >> order_list;
	rule group_list		= fields_name >> *(',' >> fields_name);
	rule group_stmt		= GROUP >> group_list;
	rule limit_stmt		= LIMIT >> integer_const >> -(',' >> integer_const);
	rule where_stmt		= WHERE >> expression;

	rule select_from_stmt	=
			TRACE_PEG(select_stmt) >>
			TRACE_PEG(from_stmt) >>
			TRACE_PEG(-where_stmt) >>
			TRACE_PEG(-group_stmt) >>
			TRACE_PEG(-order_stmt) >>
			TRACE_PEG(-limit_stmt);


	std::vector<error > e;
	_node*	r;
	{
		printf("SELECT M FROM a ...\n");
		parser	p("SELECT M FROM a");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT M FROM a ... done\n");
	}
	{
		printf("SELECT * FROM a ...\n");
		parser	p("SELECT * FROM a");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT * FROM a ... done\n");
	}
	{
		printf("SELECT 3 FROM a ...\n");
		parser	p("SELECT 3 FROM a");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT 3 FROM a ... done\n");
	}
	{
		printf("SELECT 3, 1 + 2 FROM a\n");
		parser	p("SELECT 3, 1 + 2 FROM a");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT 3, 1 + 2 FROM a ... done\n");
	}
	{
		printf("SELECT 3, 1 + 2 FROM a WHERE b=1\n");
		parser	p("SELECT 3, 1 + 2, M FROM a WHERE b=1");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT 3, 1 + 2 FROM a WHERE b=1 ... done\n");
	}
	{
		printf("SELECT \"nothing 'aaa\" FROM a WHERE b=1\n");
		parser	p("SELECT \"nothing 'aaa\" FROM a WHERE b=1");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT \"nothing 'aaa\" FROM a WHERE b=1\n");
	}
	{
		printf("SELECT a, count(1) FROM a WHERE b=1 group by a\n");
		parser	p("SELECT a, count(1) FROM a WHERE b=1 group by a");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT a, count(1) FROM a WHERE b=1 group by a\n");
	}
	{
		printf("SELECT a, count(*) FROM a WHERE b=1 group by a\n");
		parser	p("SELECT a, count(*) FROM a WHERE b=1 group by a");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT a, count(*) FROM a WHERE b=1 group by a\n");
	}
	{
		printf("select 1, case a + b when 1 then a when 2 then b else 0 end from x\n");
		parser	p("select 1, case a + b when 1 then a when 2 then b else 0 end from x");
		r = p.parse(select_from_stmt, ws, e);
		printf("select 1, case a + b when 1 then a when 2 then b else 0 end from x\n");
	}
	{
		printf("SELECT a FROM x WHERE b ISNULL\n");
		parser	p("SELECT a FROM x WHERE b ISNULL");
		r = p.parse(select_from_stmt, ws, e);
		printf("SELECT a FROM x WHERE b ISNULL\n");
	}
}
