/*
 * parser.h
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */

#ifndef CXX_PEG_PARSER_H_
#define CXX_PEG_PARSER_H_
#include <string>
#include "node.h"
#include "rule.h"

namespace cxx {
namespace peg {

	class _parser;

	class parser {
	public:
		explicit parser(const std::string& input);
		~parser();

		_node* parse(rule& r, rule& ws, std::vector<error >& e);
	private:
		_parser*	parser_;
	};


}
}



#endif /* CXX_PEG_PARSER_H_ */
