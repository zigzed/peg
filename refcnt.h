/*
 * refcnt.h
 *
 *  Created on: Feb 14, 2014
 *      Author: wilbur
 */

#ifndef CXX_PEG_REFCNT_H_
#define CXX_PEG_REFCNT_H_

namespace cxx {
namespace peg {

	class refcnt {
	public:
		refcnt() : cnt_(new int(1)) {}
		~refcnt() {
			release();
		}
		refcnt(const refcnt& rhs) : cnt_(rhs.cnt_) {
			++(*cnt_);
		}
		refcnt& operator= (const refcnt& rhs) {
			if(this == &rhs)
				return *this;

			release();
			cnt_ = rhs.cnt_;
			++(*cnt_);
			return *this;
		}

		bool unique() const {
			return *cnt_ == 1;
		}
	private:
		void release() {
			if(--(*cnt_) == 0) {
				delete cnt_;
			}
		}
		int*	cnt_;
	};

}
}



#endif /* CXX_PEG_REFCNT_H_ */
