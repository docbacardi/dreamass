
#ifndef __DA_STDINT_H__
#define __DA_STDINT_H__


#ifdef __BCC__

typedef uint8_t		unsigned char;
typedef uint16_t	unsigned short;
typedef uint32_t	unsigned long;

typedef int8_t		char;
typedef int16_t		short;
typedef int32_t		long;

#else	/* __BCC__ */

#include <stdint.h>

#endif	/* __BCC__ */


#endif	/* __DA_STDINT_H__ */
