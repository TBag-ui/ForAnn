#ifndef FORCEINLINE_H
#define FORCEINLINE_H


// inline keyword in c++ is freely ignored by the compiler. Using macro to enforce
// it's usage in a cross-platform compatible way.

#if !defined (FORCE_INLINE)
#	if defined(_MSC_VER)
#		define FORCE_INLINE __forceinline
#	elif defined(__clang__)
#		define FORCE_INLINE __inline__ __attribute__((always_inline))
#	elif defined(__GNUC__)
#		define FORCE_INLINE __inline__ __attribute__((always_inline))
#	else
#		define FORCE_INLINE CHECK_INLINE_NOT_SUPPORTED
#	endif
#endif //!defined (FORCE_INLINE)


#endif // FORCEINLINE_H
