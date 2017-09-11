/*
 * niepce - niepce/ui/selectioncontroller.cpp
 *
 * Copyright (C) 2008-2017 Hubert Figuiere
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

#include <gtkmm/iconview.h>
#include <gtkmm/treeiter.h>
#include <glibmm/i18n.h>

#include "fwk/base/autoflag.hpp"
#include "fwk/base/debug.hpp"
#include "fwk/toolkit/undo.hpp"
#include "fwk/toolkit/command.hpp"
#include "fwk/toolkit/application.hpp"
#include "engine/db/metadata.hpp"
#include "engine/db/properties.hpp"
#include "libraryclient/libraryclient.hpp"
#include "niepcewindow.hpp"
#include "selectioncontroller.hpp"

namespace ui {


SelectionController::SelectionController()
    : m_in_handler(false)
{
    m_imageliststore = ImageListStore::create();
}

void SelectionController::_added()
{
    m_imageliststore->set_parent_controller(m_parent);
}

void SelectionController::add_selectable(const IImageSelectable::WeakPtr & selectableWeak)
{
    auto selectable = selectableWeak.lock();
    DBG_OUT("added %p", (void*)selectable.get());
    m_selectables.push_back(selectableWeak);
    selectable->image_list()->signal_selection_changed().connect(
        [this, selectableWeak](){
            this->selected(selectableWeak);
        });
    selectable->image_list()->signal_item_activated().connect(
        sigc::bind(
            sigc::mem_fun(*this, &SelectionController::activated),
                   selectableWeak));
}


void SelectionController::activated(const Gtk::TreeModel::Path & path,
                                    const IImageSelectable::WeakPtr & /*selectable*/)
{
    fwk::AutoFlag f(m_in_handler);
    auto iter = m_imageliststore->get_iter(path);
    if(iter) {
        eng::LibFilePtr file = (*iter)[m_imageliststore->columns().m_libfile];
        if(file) {
            eng::library_id_t selection = engine_db_libfile_id(file.get());
            DBG_OUT("item activated %Ld", (long long)selection);
            signal_activated(selection);
        }
    }
}

void SelectionController::selected(const IImageSelectable::WeakPtr & selectableWeak)
{
    if(m_in_handler) {
        DBG_OUT("%p already in handler", (void*)this);
        return;
    }

    fwk::AutoFlag f(m_in_handler);

    auto selectable = selectableWeak.lock();
    if (!selectable) {
        // it can be, but at that point we should be terminating the app.
        ERR_OUT("selectable is null");
        return;
    }

    auto selection = selectable->get_selected();
    for(auto iterWeak : m_selectables) {
        auto iter = iterWeak.lock();
        if(iter && iter != selectable) {
            iter->select_image(selection);
        }
    }
    signal_selected(selection);
}


libraryclient::LibraryClientPtr SelectionController::getLibraryClient()
{
    ModuleShell::Ptr shell = std::dynamic_pointer_cast<ModuleShell>(m_parent.lock());
    DBG_ASSERT(static_cast<bool>(shell), "parent not a ModuleShell");
    return shell->getLibraryClient();
}

eng::library_id_t SelectionController::get_selection() const
{
    DBG_ASSERT(!m_selectables.empty(), "selectables list can't be empty");

    auto selectable = m_selectables[0].lock();
    DBG_ASSERT(static_cast<bool>(selectable), "selectable is null");

    return selectable->get_selected();
}

eng::LibFilePtr SelectionController::get_file(eng::library_id_t id) const
{
    auto iter = m_imageliststore->get_iter_from_id(id);
    if(iter) {
        return (*iter)[m_imageliststore->columns().m_libfile];
    }
    return eng::LibFilePtr();
}

void SelectionController::_selection_move(bool backwards)
{
    auto selection = get_selection();
    auto iter = m_imageliststore->get_iter_from_id(selection);
    if(!iter) {
        return;
    }

    if(backwards) {
        if(iter != m_imageliststore->children().begin()) {
            --iter;
        }
    }
    else {
        iter++;
    }
    if(iter) {
        // make sure the iterator is valid...
        eng::LibFilePtr libfile
            = (*iter)[m_imageliststore->columns().m_libfile];
        selection = engine_db_libfile_id(libfile.get());

        fwk::AutoFlag f(m_in_handler);

        std::for_each(m_selectables.begin(), m_selectables.end(),
                      [selection] (const IImageSelectable::WeakPtr & s) {
                          if (!s.expired()) {
                              s.lock()->select_image(selection);
                          }
                      });
        signal_selected(selection);
    }
}

/** select the previous image. Emit the signal */
void SelectionController::select_previous()
{
    _selection_move(true);
}


/** select the next image. Emit the signal */
void SelectionController::select_next()
{
    _selection_move(false);
}


void SelectionController::rotate(int angle)
{
    DBG_OUT("angle = %d", angle);
    auto selection = get_selection();
    if(selection >= 0) {
        Gtk::TreeIter iter = m_imageliststore->get_iter_from_id(selection);
        if(iter) {
            // @todo
        }
    }
}


bool SelectionController::_set_metadata(const std::string & undo_label,
                                        const eng::LibFilePtr & file,
                                        fwk::PropertyIndex meta,
                                        int old_value, int new_value)
{
    std::shared_ptr<fwk::UndoTransaction> undo =
        fwk::Application::app()->begin_undo(undo_label);

    auto libclient = getLibraryClient();
    auto file_id = engine_db_libfile_id(file.get());
    undo->new_command<void>(
        [libclient, file_id, meta, new_value] () {
            libclient->setMetadata(file_id, meta, fwk::PropertyValue(new_value));
        },
        [libclient, file_id, meta, old_value] () {
            libclient->setMetadata(file_id, meta, fwk::PropertyValue(old_value));
        });
    undo->execute();
    return true;
}

bool SelectionController::_set_metadata(const std::string & undo_label,
                                        const eng::LibFilePtr& file,
                                        const fwk::PropertyBag & props,
                                        const fwk::PropertyBag & old)
{
    auto undo = fwk::Application::app()->begin_undo(undo_label);
    for(auto iter : props) {
        fwk::PropertyValue value;
        auto result = old.get_value_for_property(iter.first);

        if (!result.empty()) {
            value = result.unwrap();
            if(value.get_variant().type() != typeid(fwk::EmptyValue)) {
                DBG_ASSERT(value.get_variant().type() == iter.second.get_variant().type(),
                           "Value type mismatch");
            }
        }

        auto libclient = getLibraryClient();
        auto file_id = engine_db_libfile_id(file.get());
        auto key = iter.first;
        auto new_value = iter.second;
        undo->new_command<void>(
            [libclient, file_id, key, new_value] () {
                libclient->setMetadata(file_id, key, new_value);
            },
            [libclient, file_id, key, value] () {
                libclient->setMetadata(file_id, key, value);
            });
    }
    undo->execute();
    return true;
}

void SelectionController::set_label(int label)
{
    set_property(eng::NpXmpLabelProp, label);
}


void SelectionController::set_rating(int rating)
{
    set_property(eng::NpXmpRatingProp, rating);
}

void SelectionController::set_flag(int flag)
{
    set_property(eng::NpNiepceFlagProp, flag);
}

void SelectionController::set_property(fwk::PropertyIndex idx, int value)
{
    DBG_OUT("property %u = %d", idx, value);
    eng::library_id_t selection = get_selection();
    if(selection >= 0) {
        Gtk::TreeIter iter = m_imageliststore->get_iter_from_id(selection);
        if(iter) {
            eng::LibFilePtr file = (*iter)[m_imageliststore->columns().m_libfile];
            DBG_OUT("old property is %d", engine_db_libfile_property(file.get(), idx));
            int32_t old_value = engine_db_libfile_property(file.get(), idx);
            const char *action = nullptr;
            switch(idx) {
            case eng::NpNiepceFlagProp:
                action = _("Set Flag");
                break;
            case eng::NpXmpRatingProp:
                action = _("Set Rating");
                break;
            case eng::NpXmpLabelProp:
                action = _("Set Label");
                break;
            default:
                action = _("Set Property");
                break;
            }
            _set_metadata(action, file, idx, old_value, value);
            // we need to set the property here so that undo/redo works
            // consistently.
            engine_db_libfile_set_property(file.get(), idx, value);
        }
    }
}

void SelectionController::set_properties(const fwk::PropertyBag & props,
                                         const fwk::PropertyBag & old)
{
    eng::library_id_t selection = get_selection();
    if(selection >= 0) {
        Gtk::TreeIter iter = m_imageliststore->get_iter_from_id(selection);
        if(iter) {
            eng::LibFilePtr file = (*iter)[m_imageliststore->columns().m_libfile];
            _set_metadata(_("Set Properties"), file, props, old);
        }
    }
}

void SelectionController::write_metadata()
{
    eng::library_id_t selection = get_selection();
    if(selection >= 0) {
         getLibraryClient()->write_metadata(selection);
    }
}

void SelectionController::move_to_trash()
{
    eng::library_id_t trash_folder = getLibraryClient()->trash_id();
    eng::library_id_t selection = get_selection();
    if(selection >= 0) {
        Gtk::TreeIter iter = m_imageliststore->get_iter_from_id(selection);
        if(iter) {
            eng::LibFilePtr file = (*iter)[m_imageliststore->columns().m_libfile];
            eng::library_id_t from_folder = engine_db_libfile_folderid(file.get());
            std::shared_ptr<fwk::UndoTransaction> undo =
                fwk::Application::app()->begin_undo(_("Move to Trash"));

            auto libclient = getLibraryClient();
            undo->new_command<void>(
                [libclient, selection, from_folder, trash_folder] () {
                    libclient->moveFileToFolder(selection, from_folder, trash_folder);
                },
                [libclient, selection, from_folder, trash_folder] () {
                    libclient->moveFileToFolder(selection, trash_folder, from_folder);
                });
            undo->execute();
        }
    }
}


}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
