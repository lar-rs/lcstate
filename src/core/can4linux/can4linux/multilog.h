/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/

/* Index memory adress
 * Two meory adresses are defined
 * first the first address hast to be written with the adress of
 *    the CAN register to be addressed
 * second on the second adress read or write the value
 *
 * multilog is using CAN_INDEXED_MEM_IO
 */


#ifdef CAN_INDEXED_MEM_IO

#if defined(MMC_SJA1000)
# define REGOFFSET 1
#elif defined(MULTILOG_SJA1000)
# define REGOFFSET 4
#endif

static inline unsigned indexed_inb(void __iomem *base, u32 adr) {
unsigned val;
    writeb(adr, (void __iomem *)base);
    val = readb(base + REGOFFSET);

# ifdef IODEBUG
  printk("CAN_IN: base: %x adr: %x, got: %x\n",
  	(u32)base, (u8)adr, (u8)val);
# endif
    return val;
}

# ifdef IODEBUG

#  define CAN_OUT(bd, adr, v)	do {\
        printk("CAN_OUT bd:%x base:%p reg:%x val:%x\n", \
                bd, (void __iomem *)can_iobase[bd], \
		(u32) &regbase->adr, v); \
        writeb((u32) &regbase->adr, (void __iomem *)can_iobase[bd]); \
        writeb(v, (void __iomem *)can_iobase[bd] + REGOFFSET); \
  } while(0)

#  define CAN_IN(bd, adr) 		\
	indexed_inb((void __iomem *)can_iobase[bd], (u32)&regbase->adr)

#  define CAN_SET(bd, adr, m)  do { \
        unsigned val; \
        val = indexed_inb((void __iomem *)can_iobase[bd], (u32)&regbase->adr);\
        writeb((u32)&regbase->adr, (void __iomem *)can_iobase[bd]); \
        writeb(val | m, (void __iomem *)can_iobase[bd] + REGOFFSET); \
	} while(0)

#  define CAN_RESET(bd, adr, m)  do {\
        unsigned val; \
        val = indexed_inb((void __iomem *)can_iobase[bd], (u32)&regbase->adr);\
        writeb((u32)&regbase->adr, (void __iomem *)can_iobase[bd]); \
        writeb(val & ~m, (void __iomem *)can_iobase[bd] + REGOFFSET); \
	} while(0)
/* not used, not filled, causes error at compile time */
#  define CAN_TEST(bd, adr, m) (x)

# else   /* IODEBUG */

#  define CAN_OUT(bd, adr, v)	do {\
        writeb((u32) &regbase->adr, (void __iomem *)can_iobase[bd]); \
        writeb(v, (void __iomem *)can_iobase[bd] + REGOFFSET); \
	} while(0)

#  define CAN_IN(bd, adr)		\
	indexed_inb((void __iomem *)can_iobase[bd], (u32)&regbase->adr)

#  define CAN_SET(bd, adr, m)  do {\
        unsigned val; \
        val = indexed_inb((void __iomem *)can_iobase[bd], (u32) &regbase->adr);\
        writeb((u32) &regbase->adr, (void __iomem *)can_iobase[bd]); \
        writeb(val | m, (void __iomem *)can_iobase[bd] + REGOFFSET); \
	} while(0)

#  define CAN_RESET(bd, adr, m)  do {\
        unsigned val; \
        val = indexed_inb((void __iomem *)can_iobase[bd], (u32) &regbase->adr);\
        writeb((u32) &regbase->adr, (void __iomem *)can_iobase[bd]); \
        writeb(val & ~m, (void __iomem *)can_iobase[bd] + REGOFFSET); \
	} while(0)

/* not used, not filled, causes error at compile time */
#  define CAN_TEST(bd, adr, m) (x)

# endif /* IODEBUG */
#endif /* CAN_INDEXED_MEM_IO */

