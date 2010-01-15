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
  * hook_load / -update / -delete / -insert hooks for node type mediamosa_transcode_profile.
  */

/**
 * Implementation of hook_load().
 */
function mediamosa_transcode_profile_load(&$nodes) {

  // Gather all vids.
  $vids = array();
  foreach ($nodes as $node) {
    $vids[] = $node->vid;
  }

  // Gather all additional node properties.
  $result = mediamosa_db::db_query(
    'SELECT * FROM {#table_name} WHERE vid IN (:vids)',
    array(
      '#table_name' => mediamosa_transcode_profile_db::TABLE_NAME,
      ':vids' => $vids,
    )
  );

  foreach ($result as $record) {
    foreach ($record as $key => $value) {
      if ($key == 'options') {
        $value = unserialize($value);
      }
      $nodes[$record['nid']]->$key = $value;
    }
  }

  return $nodes;
}

/**
 * Implementation of hook_update().
 */
function mediamosa_transcode_profile_update($node) {
  $options = array();
  foreach ($node as $key => $value) {
    if (strpos($key, 'options_') === 0) {

      // Get the tmid and strip the value
      $tmid = substr($key, strlen('options_'));
      $value = trim($value);

      // Gather all set options
      if ($value != '') {
        $options[$tmid] = $value;
      }
    }
  }

  // Delete the old revision
  db_delete(mediamosa_transcode_profile_db::TABLE_NAME)
    ->condition('vid', $node->vid)
    ->execute();

  // Insert the new revision
  db_insert(mediamosa_transcode_profile_db::TABLE_NAME)
    ->fields(array(
      'nid' => $node->nid,
      'vid' => $node->vid,
      mediamosa_transcode_profile_db::APP_ID => $node->app_id,
      mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE => $node->is_default_profile,
      mediamosa_transcode_profile_db::TOOL => $node->tool,
      mediamosa_transcode_profile_db::FILE_EXTENSION => $node->file_extension,
      mediamosa_transcode_profile_db::OPTIONS => serialize($options),
      mediamosa_transcode_profile_db::CHANGED => mediamosa::db_current_timestamp_now(),
    ))
    ->execute();
}

/**
 * Implementation of hook_insert().
 */
function mediamosa_transcode_profile_insert($node) {
  db_insert(mediamosa_transcode_profile_db::TABLE_NAME)
    ->fields(array(
      'nid' => $node->nid,
      'vid' => $node->vid,
      mediamosa_transcode_profile_db::APP_ID => $node->app_id,
      mediamosa_transcode_profile_db::IS_DEFAULT_PROFILE => $node->is_default_profile,
      mediamosa_transcode_profile_db::TOOL => $node->tool,
      mediamosa_transcode_profile_db::FILE_EXTENSION => $node->file_extension,
      mediamosa_transcode_profile_db::CHANGED => mediamosa::db_current_timestamp_now(),
      mediamosa_transcode_profile_db::CREATED => mediamosa::db_current_timestamp_now(),
    ))
    ->execute();
}

/**
 * Implementation of hook_delete().
 */
function mediamosa_transcode_profile_delete($node) {
  db_delete(mediamosa_transcode_profile_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();
}
