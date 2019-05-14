/**
 * @defgroup Ultra
 * @defgroup UltraChannelDiff
 * @ingroup  UltraChannelDiff
 * @{
 * @file  ultra-channel-diff.h	Valve diff header
 * @brief Pump diff header file.
 * 	 Copyright (C) LAR 2013 <asmolkov@lar.com>
 *
 * @author A.Smolkov
 *
 * $Id: $ $URL: $
 */

#ifndef _ULTRA_CHANNEL_DIFF_H_
#define _ULTRA_CHANNEL_DIFF_H_



#include <glib.h>
#include <gio/gio.h>
#include <mktbus.h>
#include <mktlib.h>
#include "ultimate-channel.h"

G_BEGIN_DECLS

#define ULTRA_TYPE_CHANNEL_DIFF             (ultra_channel_diff_get_type ())
#define ULTRA_CHANNEL_DIFF(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), ULTRA_TYPE_CHANNEL_DIFF, UltraChannelDiff))
#define ULTRA_CHANNEL_DIFF_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  ULTRA_TYPE_CHANNEL_DIFF, UltraChannelDiffClass))
#define ULTRA_IS_CHANNEL_DIFF(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ULTRA_TYPE_CHANNEL_DIFF))
#define ULTRA_IS_CHANNEL_DIFF_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  ULTRA_TYPE_CHANNEL_DIFF))
#define ULTRA_CHANNEL_DIFF_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  ULTRA_TYPE_CHANNEL_DIFF, UltraChannelDiffClass))

typedef struct _UltraChannelDiffClass         UltraChannelDiffClass;
typedef struct _UltraChannelDiff              UltraChannelDiff;
typedef struct _UltraChannelDiffPrivate       UltraChannelDiffPrivate;

struct _UltraChannelDiffClass
{
	ChannelsObjectSkeletonClass                                 parent_class;

};

struct _UltraChannelDiff
{
	ChannelsObjectSkeleton                                      parent_instance;
	UltraChannelDiffPrivate                                  *priv;
};

GType                                                   ultra_channel_diff_get_type                  ( void ) G_GNUC_CONST;


G_END_DECLS

#endif /* _ULTRA_CHANNEL_DIFF_H_ */


/** @} */

