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
  * hook_load / -update / -delete / -insert hooks for node type mediamosa_server.
  *
  * There is no need to revision the tools and containers for transcode or
  * streaming server. These values are taken from one field in the main object.
  */

/**
 * Implementation of hook_load().
 */
function mediamosa_server_load(&$nodes) {
  return mediamosa_node_revision::load($nodes);
}

/**
 * Implementation of hook_update().
 */
function mediamosa_server_update($node) {

  if ($node->revision) {
    mediamosa_server_insert($node);
  }
  else {
    // Get the field names of the table.
    $names = mediamosa_server_db::get_fields();

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_server_db::TABLE_NAME, $names);
  }
}

/**
 * Implementation of hook_insert().
 */
function mediamosa_server_insert($node) {

  // Get the field names of the table.
  $names = mediamosa_server_db::get_fields();

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_server_db::TABLE_NAME, $names);

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
  // Delete revisions.
  mediamosa_node_revision::delete($node, mediamosa_app_db::TABLE_NAME);

  // Remove streaming containers.
  db_delete(mediamosa_server_streaming_container_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();

  // Remove tools.
  db_delete(mediamosa_server_tool_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();
}
