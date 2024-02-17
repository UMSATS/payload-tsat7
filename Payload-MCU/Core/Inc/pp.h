/**
 * @file pp.h
 * A collection of useful preprocessor macros.
 *
 * @date Jan 15, 2024
 * @author Logan Furedi
 */

#ifndef INC_PP_H_
#define INC_PP_H_

////////////////////////////////////////////////////////
/// PUBLIC MACROS
////////////////////////////////////////////////////////

// expands args before concatenation.
#define CONCAT(a, b) CONCAT_(a, b)

// 1 when one argument passed. 0 otherwise.
#define IS_ONE_ARG(...)IS_ONE_ARG_(__VA_ARGS__,0,0,0,0,0,0,0,0,0,1,)

// get number of arguments in variadic macro (max 62 args)
#define NUM_ARGS(...) NARG_(_0, ## __VA_ARGS__, RSEQ_())

// for compile-time assertions.
#define CASSERT(condition, file) CASSERT_LINE_(condition,__LINE__,file)

////////////////////////////////////////////////////////
/// INTERNAL MACROS
////////////////////////////////////////////////////////

#define CONCAT_(a, b) a ## b

#define IS_ONE_ARG_(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,...)_10

#define NARG_(...) ARG_(__VA_ARGS__)

#define ARG_( \
	_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
	_11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
	_21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
	_31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
	_41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
	_51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
	_61,_62,_63,N,...) N

#define RSEQ_()          62,61,60, \
	59,58,57,56,55,54,53,52,51,50, \
	49,48,47,46,45,44,43,42,41,40, \
	39,38,37,36,35,34,33,32,31,30, \
	29,28,27,26,25,24,23,22,21,20, \
	19,18,17,16,15,14,13,12,11,10, \
	9,8,7,6,5,4,3,2,1,0

#define CASSERT_LINE_(condition, line, file) \
    typedef char CONCAT(assertion_failed_##file##_,line)[2*!!(condition)-1];

#endif /* INC_PP_H_ */
