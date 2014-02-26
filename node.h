/*
 * node.h
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */

#ifndef CXX_PEG_NODE_H_
#define CXX_PEG_NODE_H_
#include "rule.h"

namespace cxx {
namespace peg {

	class _node {
	public:
		virtual ~_node() {};
		virtual void pusharg(const char* token, context& ctx) = 0;

		cursor	begin;
		cursor	end;
	};

}
}

#endif /* CXX_PEG_NODE_H_ */
