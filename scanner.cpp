/*
 * scanner.cpp
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */
#include "scanner.h"
#include <ctype.h>	// for isspace()

namespace cxx {
namespace peg {

	scanner::scanner(const std::string& input) :
			data_(input), next_(0)
	{
	}

	int scanner::top() const
	{
		return next_ < data_.size() ? data_[next_] : 0;
	}

	int scanner::pop()
	{
		int c = top();
		advance(1);
		return c;
	}

	size_t scanner::pos() const
	{
		return next_;
	}

	void scanner::pos(size_t p)
	{
		next_ = p;
	}

	bool scanner::eof() const
	{
		return next_ >= data_.size();
	}

	std::string scanner::str(size_t begin, size_t end) const
	{
		if(begin < data_.size()) {
			if(end == (size_t)-1) {
				return data_.substr(begin);
			}
			else if(end <= data_.size() && begin <= end) {
				return data_.substr(begin, end - begin);
			}
		}
		return "";
	}

	void scanner::advance(size_t n)
	{
		next_ += n;
	}

}
}



