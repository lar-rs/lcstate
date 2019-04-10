/**
 * @defgroup LgdmLibrary
 * @defgroup LarMarket
 * @ingroup  LarMarket
 * @{
 * @file  lar-desktop-place.h object header
 * @brief This is LGDM genereted object header file.
 * 	 Copyright (C) LAR 2015
 *
 * @author A.Smolkov <asmolkov@lar.com>
 *
 * $Id: $ $URL: $
 */

#ifndef LAR_MARKET_H_
#define LAR_MARKET_H_
#include <gtk/gtk.h>
#include <glib.h>


G_BEGIN_DECLS


#define LAR_TYPE_MARKET    			    (lar_market_get_type())
#define LAR_MARKET(obj)			        (G_TYPE_CHECK_INSTANCE_CAST((obj),LAR_TYPE_MARKET, LarMarket))
#define LAR_MARKET_CLASS(klass)		    (G_TYPE_CHECK_CLASS_CAST((klass) ,LAR_TYPE_MARKET, LarMarketClass))
#define LAR_IS_MARKET(obj)		            (G_TYPE_CHECK_INSTANCE_TYPE((obj),LAR_TYPE_MARKET))
#define LAR_IS_MARKET_CLASS(klass)         (G_TYPE_CHECK_CLASS_TYPE((klass) ,LAR_TYPE_MARKET))

typedef struct _LarMarket			        LarMarket;
typedef struct _LarMarketClass		        LarMarketClass;
typedef struct _LarMarketPrivate            LarMarketPrivate;

struct _LarMarketClass
{
	GtkBoxClass                                  parent_class;
};

struct _LarMarket
{
	GtkBox                                       parent;
	LarMarketPrivate                            *priv;
};


GType 		         lar_market_get_type                ( void );


G_END_DECLS
#endif /* LAR_MARKET_H_ */

/** @} */
