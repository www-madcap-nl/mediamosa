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
  * hook_load / -update / -delete / -insert hooks for node type mediamosa_transcode_profile.
  *
  *
  */

/**
 * Implementation of hook_load().
 */
function mediamosa_transcode_profile_load(&$nodes) {
  $nodes = mediamosa_node_revision::load($nodes);
  /*
  // Gather all vids.
  $vids = array();
  foreach ($nodes as $node) {
    $vids[] = $node->vid;
  }

  // Gather all additional node properties.
  $result = mediamosa_db::db_query(
    'SELECT * FROM {#mediamosa_transcode_profile} AS mtp
     WHERE vid IN (:vids)',
    array(
      '#mediamosa_transcode_profile' => mediamosa_transcode_profile_db::TABLE_NAME,
      ':vids' => $vids,
    )
  )->fetchAll();

  // Get the tools.
  $a_tools = mediamosa_db::db_query(
    'SELECT * FROM {#mediamosa_transcode_mapping}',
    array(
      '#mediamosa_transcode_mapping' => mediamosa_transcode_mapping_db::TABLE_NAME,
    )
  )->fetchAllAssoc(mediamosa_transcode_mapping_db::TOOL);


  foreach ($result as $tool => $record) {
    // Set default of tools first.
    if (isset($a_tools[$tool])) {
      foreach ($a_tools[$tool] as $a_tool) {
        $nodes[$record['nid']]->commands{$a_tool[mediamosa_transcode_mapping_db::NICE_PARAMETER]} = mediamosa_transcode_mapping_db::DEFAULT_VALUE;
      }
    }

    foreach ($record as $key => $value) {
      if ($key == mediamosa_transcode_profile_db::COMMAND) {
        $nodes[$record['nid']]->{mediamosa_transcode_profile_db::COMMAND} = ($value == '' ? array() : mediamosa_lib::create_named_array($value, ';', ':'));
        continue;
      }

      $nodes[$record['nid']]->$key = $value;
    }
  }*/

  return $nodes;
}

/**
 * Implementation of hook_update().
 */
function mediamosa_transcode_profile_update($node) {

  if ($node->revision) {
    mediamosa_transcode_profile_insert($node);
  }
  else {
    $commands = array();
    foreach ($node as $key => $value) {
      if (strpos($key, 'options_') === 0) {

        // Get the tmid and strip the value
        $command = substr($key, strlen('options_'));
        $value = trim($value);

        // Gather all set options
        if ($value != '') {
          $commands[] = $command . ':' . $value;
        }
      }
    }

    // Store the commands.
    $node[mediamosa_transcode_profile_db::COMMAND] = implode(';', $command);

    // Get the field names of the table.
    $names = mediamosa_transcode_profile_db::get_fields();

    // Update.
    mediamosa_node_revision::update_ex($node, mediamosa_transcode_profile_db::TABLE_NAME, $names);
  }
}

/**
 * Implementation of hook_insert().
 */
function mediamosa_transcode_profile_insert($node) {

  // Get the field names of the table.
  $names = mediamosa_transcode_profile_db::get_fields();

  // Insert revision.
  mediamosa_node_revision::insert_ex($node, mediamosa_transcode_profile_db::TABLE_NAME, $names);
}

/**
 * Implementation of hook_delete().
 */
function mediamosa_transcode_profile_delete($node) {
  // Delete revisions.
  mediamosa_node_revision::delete($node, mediamosa_transcode_profile_db::TABLE_NAME);
}
