/*
 * sqlgrammar.h
 *
 *  Created on: Feb 21, 2014
 *      Author: wilbur
 */

#ifndef SQLGRAMMAR_H_
#define SQLGRAMMAR_H_

#include <vector>

void test_sql();

namespace test {

	class sql_field {

	};

	class sql_table {

	};

	// TODO: 需要考虑表达式的组合（AND/OR/NOT）
	class sql_exprs {
		// 表达式需要的字段
		std::vector<sql_field > fields;

	};

	// TODO: 需要考虑表达式的组合（AND/OR/NOT）
	class sql_preds {
		enum OPERATOR { LT, LE, EQ, GE, GT, NE };
		struct pred {
			sql_exprs	exprs;
			OPERATOR	op;
			sql_exprs	value;
		};
		std::vector<pred > 	fields;
	};

	/**
	 * 排序的字段信息，包括字段名称和排序方式（顺序或者逆序）
	 */
	class sql_order {
		enum DIRECTION { ASC, DESC };
		struct order {
			sql_field	field;
			DIRECTION	dir;
		};
	};

	class ast_select {
		std::vector<sql_exprs >	fields;
		std::vector<sql_table > tables;
		std::vector<sql_preds > conds;
		std::vector<sql_exprs > groups;
		std::vector<sql_order > orders;
	};

}


#endif /* SQLGRAMMAR_H_ */
