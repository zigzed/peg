/*
 * scanner.h
 *
 *  Created on: Feb 7, 2014
 *      Author: wilbur
 */

#ifndef CXX_PEG_SCANNER_H_
#define CXX_PEG_SCANNER_H_
#include <string>

namespace cxx {
namespace peg {

	class scanner {
	public:
		explicit scanner(const std::string& input);
		int			top() const;
		int			pop();
		size_t		pos() const;
		bool		eof() const;
		std::string str(size_t begin, size_t end = -1) const;
		void		advance(size_t n);
		void		pos(size_t p);
	private:
		std::string data_;
		size_t		next_;
	};

}
}




#endif /* CXX_PEG_SCANNER_H_ */
