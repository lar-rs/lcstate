/************************************************************************/
/* hardware access functions or macros */
/************************************************************************/

/* we are using the following functions
 static inline void __raw_writel(u32 b, volatile void __iomem *addr)
 static inline u32 __raw_readl(const volatile void __iomem *addr)

 do acess the iomem
*/

#ifndef  _IMX35_H_
#define  _IMX35_H_


/* Memory word access, 16 bit */
#define CANINW(bd, adr)          \
                (__raw_readw((void const volatile __iomem *)&((flexcan_t __iomem *)can_iobase[bd])->adr))


/* Memory long word access */
#if 0  
#warning "CANOUTL is using printk()"
/*debug version using printk to inform the programmer */

#define CANOUTL(bd, adr, v)       do { \
	printk(" write 0x%08x to %p\n", v, &((flexcan_t *)can_iobase[bd])->adr);\
                (__raw_writel(v, (void volatile __iomem *)&((flexcan_t __iomem *)can_iobase[bd])->adr));\
                } while(0)
#else

#define CANOUTL(bd, adr, v)	\
                (__raw_writel(v, (void volatile __iomem *)&((flexcan_t __iomem *)can_iobase[bd])->adr))

#endif

#define CANSETL(bd, adr, m)       \
        __raw_writel((__raw_readl((void const volatile __iomem *)&((flexcan_t __iomem *)can_iobase[bd])->adr)) \
                | (m), (void volatile __iomem *)&((flexcan_t __iomem *)can_iobase[bd])->adr)

#define CANRESETL(bd, adr, m)     \
        __raw_writel((__raw_readl((void const volatile __iomem *)&((flexcan_t __iomem *)can_iobase[bd])->adr)) \
                & ~(m), (void volatile __iomem *)&((flexcan_t __iomem *)can_iobase[bd])->adr)

#define CANINL(bd, adr)  (__raw_readl(       		\
	  (void const volatile __iomem *)&((flexcan_t __iomem *)(can_iobase[bd]))->adr)  			\
	)

#define CANTESTL(bd, adr, m)      \
        (__raw_readl((void const volatile __iomem *)&((flexcan_t *)can_iobase[bd])->adr) & (m))

#endif          /* _IMX35_H_ */

