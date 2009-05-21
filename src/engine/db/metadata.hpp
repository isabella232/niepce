/*
 * niepce - eng/db/metadata.hpp
 *
 * Copyright (C) 2008 Hubert Figuiere
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef _DB_METADATA_H_
#define _DB_METADATA_H_

namespace eng {

enum {
    META_NS_XMPCORE = 0,
    META_NS_TIFF,
    META_NS_EXIF
};

/** Metadata for xmpcore. Combine with %META_NS_XMPCORE */
enum {
    META_XMPCORE_RATING = 0,
    META_XMPCORE_LABEL
};

/** Metadata for tiff. Combine with %META_NS_TIFF */
enum {
    META_TIFF_ORIENTATION = 0
};

enum {

};

/** make a metadata index with a namespace and a value */
#define MAKE_METADATA_IDX(x,y) (x<<16|y)
/** get the NS form the metadata index */
#define METADATA_NS(x) (x >>16)
/** get the data from the metadata index */
#define METADATA_DATA(x) (x & 0xffff)

}


#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/