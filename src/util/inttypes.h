/*
	$Id$
*/

#if !defined ( __Cpp_Header__util_inttypes_h__ )
#define __Cpp_Header__util_inttypes_h__

#if !defined ( int16_t )
typedef short int16_t;
#endif

#if !defined ( uint16_t )
typedef unsigned short uint16_t;
#endif

#if !defined ( int32_t )
typedef long int32_t;
#endif

#if !defined ( uint32_t )
typedef unsigned long uint32_t;
#endif

#if !defined ( int64_t )
typedef long long int64_t;
#endif

#if !defined ( uint64_t )
typedef unsigned long long uint64_t;
#endif

#if !defined ( byte_t )
typedef unsigned char byte_t;
#endif

static inline void ___print_inttypes_size___()
{
	printf("byte_t %d\n", sizeof(byte_t));
	printf("int16_t %d\n", sizeof(int16_t));
	printf("int32_t %d\n", sizeof(int32_t));
	printf("int64_t %d\n", sizeof(int64_t));
	printf("uint16_t %d\n", sizeof(uint16_t));
	printf("uint32_t %d\n", sizeof(uint32_t));
	printf("uint64_t %d\n", sizeof(uint64_t));
}

#endif 
