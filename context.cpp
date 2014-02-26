/*
 * context.cpp
 *
 *  Created on: Feb 13, 2014
 *      Author: wilbur
 */
#include "expr.h"
#include "context.h"
#include <cassert>
#include <stdio.h>

namespace cxx {
namespace peg {

	context::~context()
	{
	}

	_context::_context(const expr& whitespace) :
			space_(_util::get_expr(whitespace))
	{
	}

	_expr* _context::ws() const
	{
		return space_;
	}

	void _context::add_node(_node* n)
	{
		nodes_.push_back(n);
	}

	_node* _context::get_node()
	{
		_node* n = 0;
		if(!nodes_.empty()) {
			n = nodes_.back();
			nodes_.pop_back();
		}
		return n;
	}

	size_t _context::node_len() const
	{
		return nodes_.size();
	}

	void _context::add_error(const error& e)
	{
		error_.push_back(e);
	}

	size_t _context::get_error(std::vector<error >& e)
	{
		e.clear();
		e.swap(error_);
		return e.size();
	}

	size_t _context::backup() const
	{
		return nodes_.size();
	}

	void _context::unwind(size_t cp)
	{
		assert(nodes_.size() >= cp);
		while(nodes_.size() > cp) {
			nodes_.pop_back();
		}
	}

	void _context::add_cache(size_t begin, const _expr* r, size_t end)
	{
		if(cache_.find(std::make_pair(begin, r)) != cache_.end()) {
			printf("add_cache(%d, %p, %d) exist\n", begin, r, end);
		}
		cache_.insert(std::make_pair(std::make_pair(begin, r), end));
	}

	size_t _context::get_cache(size_t begin, const _expr* r)
	{
		memory_t::iterator it = cache_.find(std::make_pair(begin, r));
		if(it != cache_.end())
			return it->second;
		return -1;
	}

}
}



