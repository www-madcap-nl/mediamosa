<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

 /**
  * @file
  * The user favorite module.
  */
class user_favorite {

  /**
   * Return the number of rows found.
   *
   * @param string $fav_type
   * @param string $fav_id
   * @return integer
   */
  static public function count_items($fav_type, $fav_id) {

    // Return the number of found rows.
    $query = db_select(mediamosa_user_favorite_db::TABLE_NAME);
    $query->condition(mediamosa_user_favorite_db::FAV_TYPE, $fav_type);
    $query->condition(mediamosa_user_favorite_db::FAV_ID, $fav_id);
    return $query->countQuery()->execute()->fetchField();
  }
}
