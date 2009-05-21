/*
 * niepce - niepce/notificationcenter.hpp
 *
 * Copyright (C) 2009 Hubert Figuiere
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


#ifndef __NIEPCE_NOTIFICATIONCENTER_HPP_
#define __NIEPCE_NOTIFICATIONCENTER_HPP_

#include <tr1/memory>
#include <sigc++/signal.h>

#include "fwk/toolkit/notificationcenter.hpp"
#include "engine/db/library.hpp"
#include "engine/library/thumbnailnotification.hpp"


namespace niepce {


class NotificationCenter
  : public fwk::NotificationCenter
{
public:
  typedef std::tr1::shared_ptr<NotificationCenter> Ptr;
  NotificationCenter();

  sigc::signal<void, const eng::LibNotification &> signal_lib_notification;
  sigc::signal<void, const eng::ThumbnailNotification &> signal_thumbnail_notification;

private:
  void dispatch_notification(const fwk::Notification::Ptr &n);
};

}

#endif