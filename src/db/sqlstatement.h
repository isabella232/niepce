/* -*- Mode: C++; indent-tabs-mode:nil; c-basic-offset:4; -*- */
/*
 *This file is part of the Nemiver Project.
 *
 *Nemiver is free software; you can redistribute
 *it and/or modify it under the terms of
 *the GNU General Public License as published by the
 *Free Software Foundation; either version 2,
 *or (at your option) any later version.
 *
 *Nemiver is distributed in the hope that it will
 *be useful, but WITHOUT ANY WARRANTY;
 *without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *See the GNU General Public License for more details.
 *
 *You should have received a copy of the
 *GNU General Public License along with Nemiver;
 *see the file COPYING.
 *If not, write to the Free Software Foundation,
 *Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *See COPYRIGHT file copyright information.
 */
#ifndef __NEMIVER_SQL_STATEMENT_H__
#define __NEMIVER_SQL_STATEMENT_H__

#include <stdint.h>

//#pragma GCC visibility push(default)
#include <vector>
//#pragma GCC visibility pop
#include <string>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>


namespace db
{

class Column
{
    std::string m_name ;
    std::string m_value ;
    bool m_auto_increment ;

public:

    Column (const std::string &a_name="empty:empty",
            const std::string &a_value="empty:empty",
            bool a_auto_increment=false):
            m_name (a_name),
            m_value (a_value),
            m_auto_increment (a_auto_increment)
    {}

    Column (const std::string &a_name,
            uint64_t a_value,
            bool a_auto_increment=false):
            m_name (a_name),
            m_value (boost::lexical_cast<std::string>(a_value)),
            m_auto_increment (a_auto_increment)
    {}

    const std::string& get_name ()
    {
        return m_name ;
    }
    void set_name (const std::string &a_name)
    {
        m_name = a_name;
    }
    const std::string& get_value ()
    {
        return m_value ;
    }
    void set_value (const std::string &a_value)
    {
        m_value = a_value ;
    }
    void set_auto_increment (bool a_auto)
    {
        m_auto_increment = a_auto;
    }
    bool get_auto_increment ()
    {
        return m_auto_increment;
    }
};

typedef std::vector<Column> ColumnList ;

struct SQLStatementPriv ;

class SQLStatement
{
    friend class Connection ;
    friend struct SQLStatementPriv ;

    SQLStatementPriv *m_priv ;

public:

    SQLStatement (const std::string &a_sql_string="") ;
    SQLStatement (const boost::format & _format);

    SQLStatement (const SQLStatement &) ;

    SQLStatement& operator= (const SQLStatement &) ;

    virtual ~SQLStatement () ;
    virtual const std::string& to_string () const ;

    static std::string escape_string (const std::string &a_sql_string) ;

    operator const char* () const ;
    friend std::iostream& operator<< (std::iostream&, const SQLStatement&) ;
};

std::ostream & operator<< (std::ostream &, const SQLStatement &) ;

}//end namespace db

#endif //__NEMIVER_SQL_STATEMENT_H__
