/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * node-index.h
 * Copyright (C) LAR 2014-2019
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CAN_H_
#define _CAN_H_



typedef struct _Index Index;


Index*    string_index               ( guint id, guint index, guint subindex, const gcher *value);
Index*    int_index                  ( guint id, guint index, guint subindex, guint value );

gboolean  index_read                 ( Index *index,GError **error);
gboolean  index_write                ( Index *index,GError **error);


#endif /* _INDEX */

