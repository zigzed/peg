/*
 * parser.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */
#include "parser.h"
#include "rule.h"
#include "expr.h"
#include "context.h"

namespace cxx {
namespace peg {

	class _parser {
	public:
		explicit _parser(const std::string& input);
		_node* parse(const rule& r, const rule& ws, std::vector<error >& e);
	private:
		scanner	ifs_;
		cursor	pos_;
	};

	_parser::_parser(const std::string& input) :
			ifs_(input), pos_(&ifs_, 0)
	{
	}

	_node* _parser::parse(const rule& r, const rule& ws, std::vector<error >& e)
	{
		_context ctx(_util::new_expr(_util::get_expr(ws)));
		if(!r.match(ifs_, ctx))
			return NULL;

		if(!ifs_.eof()) {
			ctx.add_error(error(cursor(&ifs_, ifs_.pos()), "syntax error"));
			ctx.get_error(e);
			return NULL;
		}

		if(ctx.node_len() == 0) {
			ctx.get_error(e);
			return NULL;
		}

		ctx.get_error(e);
		return ctx.get_node();
	}

	////////////////////////////////////////////////////////////////////////////
	parser::parser(const std::string& input) :
			parser_(NULL)
	{
		parser_ = new _parser(input);
	}

	parser::~parser()
	{
		delete parser_;
		parser_ = NULL;
	}

	_node* parser::parse(rule& r, rule& ws, std::vector<error >& e)
	{
		return parser_->parse(r, ws, e);
	}

}
}



