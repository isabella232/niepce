/*
 * niepce - engine/db/keyword.cpp
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


#include "keyword.hpp"

extern "C" eng::Keyword* engine_db_keyword_new(eng::library_id_t id, const char* keyword);
extern "C" void engine_db_keyword_delete(eng::Keyword*);

namespace eng {

KeywordPtr keyword_new(eng::library_id_t id, const char* keyword) {
  return KeywordPtr(
    engine_db_keyword_new(id, keyword), &engine_db_keyword_delete);
}

}
