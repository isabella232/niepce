/*
 * niepce - fwk/utils/files.cpp
 *
 * Copyright (C) 2007 Hubert Figuiere
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

#include <boost/algorithm/string.hpp>

#include <giomm/file.h>

#include "fwk/base/debug.hpp"
#include "files.hpp"
#include "pathutils.hpp"


namespace fwk {

	bool filter_none(const Glib::RefPtr<Gio::FileInfo> & )
	{
		return true;
	}


	bool filter_xmp_out(const Glib::RefPtr<Gio::FileInfo> & file)
	{
		std::string ext = fwk::path_extension(file->get_name());
		boost::to_lower(ext);
		if(ext == ".xmp") {
			return false;
		}
		return true;
	}


	FileList::FileList( const _impltype_t & v )
		: _impltype_t( v )
	{
	}

	FileList::Ptr FileList::getFilesFromDirectory(const FileList::value_type & p, boost::function<bool (const Glib::RefPtr<Gio::FileInfo> &)> filter)
	{
//		if(!exists( p ) ) {
//			DBG_OUT( "directory %s do not exist", p.c_str() );
//			return Ptr();
//		}
		try
		{
			FileList::Ptr l( new FileList() );
			
      Glib::RefPtr<Gio::File> dir = Gio::File::create_for_path(p);
      Glib::RefPtr<Gio::FileEnumerator> enumerator = dir->enumerate_children();

			for( Glib::RefPtr<Gio::FileInfo> itr = enumerator->next_file();
            itr ; itr = enumerator->next_file() )
			{
        Gio::FileType ftype = itr->get_file_type();
				if ((ftype == Gio::FILE_TYPE_REGULAR)  || (ftype == Gio::FILE_TYPE_SYMBOLIC_LINK))
				{
					if( filter(itr) ) {
            std::string fullname = Glib::build_filename(dir->get_path(), itr->get_name());
						l->push_back(fullname);
						DBG_OUT( "found file %s", fullname.c_str() );
					}
				}
			}
      enumerator->close();
			l->sort();
			return l;
		}
    catch(const Glib::Error & e)
    {
			ERR_OUT( "Exception: %s", e.what().c_str() );
    }
		catch(const std::exception & e )
		{
			ERR_OUT( "Exception: %s", e.what() );
		}

		return Ptr();
	}


}

