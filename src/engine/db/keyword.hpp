/*
 * niepce - eng/db/keyword.hpp
 *
 * Copyright (C) 2007-2017 Hubert Figuiere
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


#pragma once

#include <vector>
#include <memory>

#include "engine/db/librarytypes.hpp"

// just a rust interface.
namespace eng {
class Keyword;
typedef std::shared_ptr<Keyword> KeywordPtr;
typedef std::vector<KeywordPtr> KeywordList;
typedef std::shared_ptr<KeywordList> KeywordListPtr;

KeywordPtr keyword_new(eng::library_id_t id, const char* keyword);
}

extern "C" eng::library_id_t engine_db_keyword_id(const eng::Keyword*);
extern "C" const char* engine_db_keyword_keyword(const eng::Keyword*);

