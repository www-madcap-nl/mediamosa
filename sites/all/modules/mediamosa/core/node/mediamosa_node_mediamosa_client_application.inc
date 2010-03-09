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
function mediamosa_client_application_load(&$nodes, $types) {

  // Gather all vids.
  $vids = array();
  foreach ($nodes as $node) {
    $vids[] = $node->vid;
  }

  // Gather all additional node properties.
  $result = mediamosa_db::db_query(
    'SELECT * FROM {#table_name} WHERE vid IN (:vids)',
    array(
      '#table_name' => mediamosa_app_db::TABLE_NAME,
      ':vids' => $vids,
    )
  );

  foreach ($result as $record) {
    foreach ($record as $key => $value) {
      $nodes[$record['nid']]->$key = $value;
    }
  }

  return $nodes;
}

/**
 * Implementation of hook_update().
 */
function mediamosa_client_application_update($node) {

  if ($node->revision) {
    mediamosa_client_application_insert($node);
    return;
  }

  // Get the field names of the table.
  $names = mediamosa_app_db::get_fields();

  $a_fields = array();
  foreach ($names as $name) {
    if (in_array($name, array(mediamosa_app_db::NID))) {
      continue;
    }

    if (isset($node->{$name})) {
      $a_fields[$name] = $node->{$name};
    }
  }

  // Update the new revision
  mediamosa_db::db_update(mediamosa_app_db::TABLE_NAME)
    ->fields($a_fields)
    ->condition(mediamosa_app_db::VID, $node->vid)
    ->execute();
}

/**
 * Implementation of hook_insert().
 */
function mediamosa_client_application_insert($node) {

  // Get the field names of the table.
  $names = mediamosa_app_db::get_fields();

  // Collect the data we want to insert.
  $a_fields = array();
  foreach ($names as $name) {
    if (isset($node->{$name})) {
      $a_fields[$name] = $node->{$name};
    }
  }

  // Insert.
  mediamosa_db::db_insert(mediamosa_app_db::TABLE_NAME)
    ->fields($a_fields)
    ->execute();
}

/**
 * Implementation of hook_delete().
 */
function mediamosa_client_application_delete($node) {
  db_delete(mediamosa_app_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();
}
