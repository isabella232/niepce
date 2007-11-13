/*
 * niepce - db/library.h
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



#ifndef _DB_LIBRARY_H_
#define _DB_LIBRARY_H_

#include <string>

#include <boost/shared_ptr.hpp>

#include "iconnectiondriver.h"
#include "iconnectionmanagerdriver.h"

namespace db {

	class Library
	{
	public:
		typedef boost::shared_ptr<Library> Ptr;

		Library(const std::string & dir);
		virtual ~Library();

		bool ok()
			{ return m_inited; }
		/** set the main library directory */
//		void setMainDir(const std::string & dir)
//			{ m_maindir = dir; }
		/** return the main directory */
		const std::string & mainDir() const
			{ return m_maindir; }
		const std::string & dbName() const
			{ return m_dbname; }

		/** add a file to the library
		 * @param folder the containing folder
		 * @param file the file path
		 * @param manage pass true it the library *manage* the file. Currently unsupported.
		 */
		int addFile2(const std::string & folder, const std::string & file, bool manage);
		/** add a file to the library
		 * @param folder_id the id of the containing folder
		 * @param file the file path
		 * @param manage pass true it the library *manage* the file. Currently unsupported.
		 */
		int addFile(int folder_id, const std::string & file, bool manage);
		
		/** Get a specific folder id from the library
		 * @param folder the folder path to check
		 * @return the id of the folder, -1 if not found
		 */
		int getFolder(const std::string & folder);
		/** Add a folder
		 * @param folder the folder path
		 */
		int addFolder(const std::string & folder);

		int checkDatabaseVersion();
		
		db::IConnectionDriver::Ptr dbDriver()
			{ return m_dbdrv; }
	private:
		bool init();
		bool _initDb();

		std::string                       m_maindir;
		std::string                       m_dbname;
		db::IConnectionManagerDriver::Ptr m_dbmgr;
		db::IConnectionDriver::Ptr        m_dbdrv;
		bool                              m_inited;
	};

}

#endif
