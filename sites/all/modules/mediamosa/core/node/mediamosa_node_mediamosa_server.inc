<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
  * hook_load / -update / -delete / -insert hooks for node type mediamosa_server.
  */

/**
 * Implementation of hook_load().
 */
function mediamosa_server_load(&$nodes) {

  // Gather all vids.
  $vids = array();
  foreach ($nodes as $node) {
    $vids[] = $node->vid;
  }

  // Gather all additional node properties.
  $result = mediamosa_db::db_query(
    'SELECT * FROM {#table_name} WHERE vid IN (:vids)',
    array(
      '#table_name' => mediamosa_server_db::TABLE_NAME,
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
function mediamosa_server_update($node) {

  // Update the new revision
  mediamosa_db::db_update(mediamosa_server_db::TABLE_NAME)
    ->fields(array(
      'vid' => $node->vid,
      mediamosa_server_db::SERVER_STATUS => $node->server_status,
      mediamosa_server_db::URI => (isset($node->uri)) ? $node->uri : NULL,
      mediamosa_server_db::DESCRIPTION => (isset($node->description)) ? $node->description : NULL,
      mediamosa_server_db::CONTAINERS => (isset($node->containers)) ? $node->containers : NULL,
      mediamosa_server_db::OBJECT_CODE => (isset($node->object_code)) ? $node->object_code : NULL,
      mediamosa_server_db::SERVER_NAME => (isset($node->server_name)) ? $node->server_name : NULL,
      mediamosa_server_db::SLOTS => (isset($node->slots)) ? $node->slots : NULL,
      mediamosa_server_db::TOOLS => (isset($node->tools)) ? $node->tools : NULL,
      mediamosa_server_db::URI_UPLOAD_PROGRESS => (isset($node->uri_upload_progress)) ? $node->uri_upload_progress : NULL,
    ))
    ->condition(mediamosa_server_db::NID, $node->nid)
    ->execute();
}

/**
 * Implementation of hook_insert().
 */
function mediamosa_server_insert($node) {
  $server_id = mediamosa_db::db_insert(mediamosa_server_db::TABLE_NAME)
    ->fields(array(
      'nid' => $node->nid,
      'vid' => $node->vid,
      mediamosa_server_db::SERVER_TYPE => $node->server_type,
      mediamosa_server_db::SERVER_STATUS => (isset($node->{mediamosa_server_db::SERVER_STATUS})) ? $node->{mediamosa_server_db::SERVER_STATUS} : 0, // 'Off' by default
      mediamosa_server_db::VERSION => (isset($node->{mediamosa_server_db::VERSION})) ? $node->{mediamosa_server_db::VERSION} : NULL,
      mediamosa_server_db::URI => (isset($node->uri)) ? $node->uri : NULL,
      mediamosa_server_db::CONTAINERS => (isset($node->containers)) ? $node->containers : NULL,
      mediamosa_server_db::CONTAINERS => (isset($node->containers)) ? $node->containers : NULL,
      mediamosa_server_db::OBJECT_CODE => (isset($node->object_code)) ? $node->object_code : NULL,
      mediamosa_server_db::SERVER_NAME => (isset($node->server_name)) ? $node->server_name : NULL,
      mediamosa_server_db::SLOTS => (isset($node->slots)) ? $node->slots : NULL,
      mediamosa_server_db::TOOLS => (isset($node->tools)) ? $node->tools : NULL,
      mediamosa_server_db::URI_UPLOAD_PROGRESS => (isset($node->uri_upload_progress)) ? $node->uri_upload_progress : NULL,
    ))
    ->execute();

    // Check type.
    switch ($node->server_type) {
      case mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR:
        if (isset($node->{mediamosa_server_tool_db::TABLE_NAME})) {
          foreach ($node->{mediamosa_server_tool_db::TABLE_NAME} as $a_tool) {
            mediamosa_db::db_insert(mediamosa_server_tool_db::TABLE_NAME)
              ->fields(array(
                'nid' => $node->nid,
                mediamosa_server_tool_db::TOOL => $a_tool[mediamosa_server_tool_db::TOOL]
              ))
              ->execute();
          }
        }
        break;

      case mediamosa_server_db::SERVER_TYPE_STREAMING:
        if (isset($node->{mediamosa_server_streaming_container_db::TABLE_NAME})) {
          foreach ($node->{mediamosa_server_streaming_container_db::TABLE_NAME} as $a_streaming) {
            mediamosa_db::db_insert(mediamosa_server_streaming_container_db::TABLE_NAME)
              ->fields(array(
                'nid' => $node->nid,
                mediamosa_server_streaming_container_db::CONTAINER => $a_streaming[mediamosa_server_streaming_container_db::CONTAINER]
              ))
              ->execute();
          }
        }
        break;

      default:
        break;
    }
}

/**
 * Implementation of hook_delete().
 */
function mediamosa_server_delete($node) {
  db_delete(mediamosa_server_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();
}
