<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
 * The node type client application.
 * hook_load / -update / -delete / -insert hooks for node type
 * mediamosa_client_application.
 */

/**
 * Implementation of hook_load().
 */
function mediamosa_client_application_load(&$nodes) {
  return mediamosa_node_revision::load($nodes);
}

/**
 * Implementation of hook_update().
 */
function mediamosa_client_application_update($node) {

  if ($node->revision) {
    mediamosa_client_application_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_app_db::get_fields();

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_app_db::TABLE_NAME, $names);
  }
}

/**
 * Implementation of hook_insert().
 */
function mediamosa_client_application_insert($node) {

  // Get the field names of the table.
  $names = mediamosa_app_db::get_fields();

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_app_db::TABLE_NAME, $names);
}

/**
 * Implementation of hook_delete().
 */
function mediamosa_client_application_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete($node, mediamosa_app_db::TABLE_NAME);
}
