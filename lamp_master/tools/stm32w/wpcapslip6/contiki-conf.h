<<<<<<< HEAD
#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__
#include <stdint.h>
#define CCIF
#define CLIF

/* These names are deprecated, use C99 names. */
typedef uint8_t   u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef  int32_t s32_t;

typedef unsigned short uip_stats_t;

#define UIP_CONF_UIP_IP4ADDR_T_WITH_U32 1

typedef unsigned long clock_time_t;
#define CLOCK_CONF_SECOND 1000

#endif /* __CONTIKI_CONF_H__ */
=======
#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__
#include <stdint.h>
#define CCIF
#define CLIF

/* These names are deprecated, use C99 names. */
typedef uint8_t   u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef  int32_t s32_t;

typedef unsigned short uip_stats_t;

#define UIP_CONF_UIP_IP4ADDR_T_WITH_U32 1

typedef unsigned long clock_time_t;
#define CLOCK_CONF_SECOND 1000

#endif /* __CONTIKI_CONF_H__ */
>>>>>>> 1c8a365a388f3826ae65a4404b1caaf07e71bb24
