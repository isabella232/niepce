/*
 * niepce - ui/workspacecontroller.cpp
 *
 * Copyright (C) 2007-2009 Hubert Figuiere
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

#include <boost/any.hpp>
#include <boost/bind.hpp>

#include <glibmm/i18n.h>

#include <gtkmm/icontheme.h>
#include <gtkmm/box.h>

#include "fwk/base/debug.hpp"
#include "niepce/notifications.hpp"
#include "engine/db/librarytypes.hpp"
#include "libraryclient/libraryclient.hpp"
#include "fwk/toolkit/application.hpp"
#include "niepcewindow.hpp"
#include "workspacecontroller.hpp"


using fwk::Application;

namespace ui {


WorkspaceController::WorkspaceController()
    : fwk::UiController()
{
    static struct _Icons {
        int icon_id;
        const char *icon_name;
    } icons[] = {
        { ICON_FOLDER, "folder" },
        { ICON_PROJECT, "applications-accessories" },
        { ICON_ROLL, "emblem-photos" },
        { ICON_TRASH, "user-trash" },
        { ICON_KEYWORD, "application-certificate" },
        { 0, NULL }
    };
    
    Glib::RefPtr< Gtk::IconTheme > icon_theme(Application::app()->getIconTheme());
    int i = 0;
    while(icons[i].icon_name) {
        try {
            m_icons[icons[i].icon_id] = icon_theme->load_icon(
            Glib::ustring(icons[i].icon_name), 16, Gtk::ICON_LOOKUP_USE_BUILTIN);
        }
        catch(const Gtk::IconThemeError & e)
        {
            ERR_OUT("Exception %s.", e.what().c_str());
        }
        i++;
    }
}

libraryclient::LibraryClient::Ptr WorkspaceController::getLibraryClient()
{
    return	std::tr1::dynamic_pointer_cast<NiepceWindow>(m_parent.lock())->getLibraryClient();
}


void WorkspaceController::on_lib_notification(const eng::LibNotification &ln)
{
    DBG_OUT("notification for workspace");
    switch(ln.type) {
    case eng::Library::NOTIFY_ADDED_FOLDERS:
    {
        eng::LibFolder::ListPtr l 
            = boost::any_cast<eng::LibFolder::ListPtr>(ln.param);
        DBG_OUT("received added folders # %lu", l->size());
        for_each(l->begin(), l->end(), 
                 boost::bind(&WorkspaceController::add_folder_item, 
                             this, _1));
        break;
    }
    case eng::Library::NOTIFY_ADDED_KEYWORD:
    {
        eng::Keyword::Ptr k
            = boost::any_cast<eng::Keyword::Ptr>(ln.param);
        DBG_ASSERT(k, "keyword must not be NULL");
        add_keyword_item(k);
        break;
    }
    case eng::Library::NOTIFY_ADDED_KEYWORDS:
    {
        eng::Keyword::ListPtr l
            = boost::any_cast<eng::Keyword::ListPtr>(ln.param);
        DBG_ASSERT(l, "keyword list must not be NULL");
        for_each(l->begin(), l->end(), 
                 boost::bind(&WorkspaceController::add_keyword_item, 
                             this, _1));
        break;
    }
    case eng::Library::NOTIFY_FOLDER_COUNTED:
    {
        std::pair<eng::library_id_t,int> count(boost::any_cast<std::pair<eng::library_id_t,int> >(ln.param));
        DBG_OUT("count for folder %Ld is %d", count.first, count.second);
        std::map<int, Gtk::TreeIter>::iterator iter
            = m_folderidmap.find( count.first );
        if(iter != m_folderidmap.end()) {
            Gtk::TreeRow row = *(iter->second);
            row[m_librarycolumns.m_count] = boost::lexical_cast<Glib::ustring>(count.second);
        }

        break;
    }
    default:
        break;
    }
}

void WorkspaceController::on_count_notification(int)
{
    DBG_OUT("received NOTIFICATION_COUNT");
}


void WorkspaceController::on_libtree_selection()
{
    Glib::RefPtr<Gtk::TreeSelection> selection = m_librarytree.get_selection();
    Gtk::TreeModel::iterator selected = selection->get_selected();
    if((*selected)[m_librarycolumns.m_type] == FOLDER_ITEM)
    {
        int id = (*selected)[m_librarycolumns.m_id];
        getLibraryClient()->queryFolderContent(id);
    }
    else if((*selected)[m_librarycolumns.m_type] == KEYWORD_ITEM)
    {
        int id = (*selected)[m_librarycolumns.m_id];
        getLibraryClient()->queryKeywordContent(id);			
    }
    else 
    {
        DBG_OUT("selected something not a folder");
    }
}

void WorkspaceController::add_keyword_item(const eng::Keyword::Ptr & k)
{
    Gtk::TreeModel::iterator iter(add_item(m_treestore, m_keywordsNode->children(), 
                                           m_icons[ICON_KEYWORD], k->keyword(), k->id(), 
                                           KEYWORD_ITEM));
//		getLibraryClient()->countKeyword(f->id());
    m_keywordsidmap[k->id()] = iter;
}

void WorkspaceController::add_folder_item(const eng::LibFolder::Ptr & f)
{
    int icon_idx = ICON_ROLL;
    if(f->virtual_type() == eng::LibFolder::VIRTUAL_TRASH) {
        icon_idx = ICON_TRASH;
    }
    Gtk::TreeModel::iterator iter(add_item(m_treestore, 
                                           m_folderNode->children(), 
                                           m_icons[icon_idx], 
                                           f->name(), f->id(), FOLDER_ITEM));
    getLibraryClient()->countFolder(f->id());
    m_folderidmap[f->id()] = iter;
}

Gtk::TreeModel::iterator
WorkspaceController::add_item(const Glib::RefPtr<Gtk::TreeStore> &treestore,
                              const Gtk::TreeNodeChildren & childrens,
                              const Glib::RefPtr<Gdk::Pixbuf> & icon,
                              const Glib::ustring & label, 
                              eng::library_id_t id, int type) const
{
    Gtk::TreeModel::iterator iter;
    Gtk::TreeModel::Row row;
    iter = treestore->append(childrens);
    row = *iter;
    row[m_librarycolumns.m_icon] = icon;
    row[m_librarycolumns.m_label] = label; 
    row[m_librarycolumns.m_id] = id;
    row[m_librarycolumns.m_type] = type;
    row[m_librarycolumns.m_count] = "--";
    return iter;
}


Gtk::Widget * WorkspaceController::buildWidget(const Glib::RefPtr<Gtk::UIManager> &)
{
    if(m_widget) {
        return m_widget;
    }
    m_widget = &m_vbox;
    m_treestore = Gtk::TreeStore::create(m_librarycolumns);
    m_librarytree.set_model(m_treestore);

    m_folderNode = add_item(m_treestore, m_treestore->children(),
                            m_icons[ICON_FOLDER], 
                            Glib::ustring(_("Pictures")), 0,
                            FOLDERS_ITEM);
    m_projectNode = add_item(m_treestore, m_treestore->children(),
                             m_icons[ICON_PROJECT], 
                             Glib::ustring(_("Projects")), 0,
                             PROJECTS_ITEM);
    m_keywordsNode = add_item(m_treestore, m_treestore->children(),
                              m_icons[ICON_KEYWORD],
                              Glib::ustring(_("Keywords")), 0,
                              KEYWORDS_ITEM);

    m_librarytree.set_headers_visible(false);
    m_librarytree.append_column("", m_librarycolumns.m_icon);
    int num = m_librarytree.append_column("", m_librarycolumns.m_label);
    Gtk::TreeViewColumn * col = m_librarytree.get_column(num - 1);
    col->set_expand(true);
    num = m_librarytree.append_column("", m_librarycolumns.m_count);
    col = m_librarytree.get_column(num - 1);
    col->set_alignment(1.0f);

    // TODO make it a mnemonic
    m_label.set_text_with_mnemonic(Glib::ustring(_("_Workspace")));
    m_label.set_mnemonic_widget(m_librarytree);
    m_vbox.pack_start(m_label, Gtk::PACK_SHRINK);
    m_vbox.pack_start(m_librarytree);

    m_librarytree.get_selection()->signal_changed().connect (
        sigc::mem_fun(this, 
                      &WorkspaceController::on_libtree_selection));

    return m_widget;
}
	
void WorkspaceController::on_ready()
{
    libraryclient::LibraryClient::Ptr libraryClient = getLibraryClient();
    if(libraryClient)
    {
        libraryClient->getAllFolders();
        libraryClient->getAllKeywords();
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
