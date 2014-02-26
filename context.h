/*
 * context.h
 *
 *  Created on: Feb 12, 2014
 *      Author: wilbur
 */

#ifndef CXX_PEG_CONTEXT_H_
#define CXX_PEG_CONTEXT_H_
#include <vector>
#include <map>
#include "rule.h"

namespace cxx {
namespace peg {

	class _context : public context {
	public:
		explicit _context(const expr& ws);
		void 	add_node(_node* n);
		_node*	get_node();
		size_t	node_len() const;

		void	add_error(const error& e);
		size_t	get_error(std::vector<error >& e);

		void	add_cache(size_t begin, const _expr* e, size_t end);
		size_t	get_cache(size_t begin, const _expr* e);

		size_t	backup() const;
		void	unwind(size_t cp);

		_expr*	ws() const;
	private:
		typedef std::vector<_node* >	_nodes_t;
		typedef std::vector<error >		errors_t;
		typedef std::pair<size_t, const _expr* >	key_t;
		/** memory_t 是一个开始位置、匹配规则的缓存（映射到结束位置）。缓存的目的是为了
		 * 提高性能，同时支持左递归 */
		typedef std::map<key_t, size_t>	memory_t;
		_nodes_t	nodes_;
		errors_t	error_;
		_expr*		space_;
		memory_t	cache_;
	};

}
}




#endif /* CXX_PEG_CONTEXT_H_ */
