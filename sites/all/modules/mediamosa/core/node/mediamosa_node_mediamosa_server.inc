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
  mediamosa_node_revision::delete_ex($node, mediamosa_server_db::TABLE_NAME);

  // Remove streaming containers.
  db_delete(mediamosa_server_streaming_container_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();

  // Remove tools.
  db_delete(mediamosa_server_tool_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();
}

/**
 * Implementation of hook_view().
 */
function mediamosa_server_view($node, $view_mode = 'full') {

  // Default parameters
  $rows = array(
    array(t('Name'), check_plain($node->title)),
    array(t('Status'), ($node->server_status) ? t('On') : t('Off')),
    array(t('Server type'), $node->server_type),
  );

  foreach (array(
    'uri' => t('URI'),
    'uri_upload_progress' => t('URI upload progress'),
    'containers' => t('Containers'),
    'server_name' => t('Server name'),
    'slots' => t('Slots'),
    'tools' => t('Tools'),
  ) as $key => $title) {
    if (!is_null($node->{$key})) {
      $rows[] = array($title, check_plain($node->{$key}));
    }
  }

  // Add timestamps.
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['app'] = array(
    '#markup' => theme('table', array('header' => array(t('Parameter'), t('Value')), 'rows' => $rows)),
  );

  return $node;
}

/**
 * Implementation of hook_form().
 */
function mediamosa_server_form($node, &$form_state) {

  // Take type from node if available.
  if (isset($node->{mediamosa_server_db::SERVER_TYPE})) {
    $server_type = $node->{mediamosa_server_db::SERVER_TYPE};
  }

  // Else take it from the uri.
  if (!isset($server_type)) {
    $server_type = arg(5);
  }

  // Default server status options.
  $server_status_options =  array(mediamosa_server_db::SERVER_STATUS_ON => t('On'), mediamosa_server_db::SERVER_STATUS_OFF => t('Off'));


  switch ($server_type) {
    default:
    case mediamosa_server_db::SERVER_TYPE_DOWNLOAD:
      $title = t('New download server');
      $description = t('A download server handles download requests of mediafiles.');
      break;
    case mediamosa_server_db::SERVER_TYPE_STILL:
      $title = t('New still server');
      $description = t('A still server serves and generates stills from mediafiles.');
      break;
    case mediamosa_server_db::SERVER_TYPE_STREAMING:
      $title = t('New streaming server');
      $description = t('A streaming server will stream mediafiles for you.');
      break;
    case mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR:
      $title = t('New processor server');
      $description = t('A processor server processes jobs like still generation, transcoding of mediafiles.');
      $server_status_options =  array(mediamosa_server_db::SERVER_STATUS_ON => t('On'), mediamosa_server_db::SERVER_STATUS_CLOSE => t('Close'), mediamosa_server_db::SERVER_STATUS_OFF => t('Off'));
      break;
    case mediamosa_server_db::SERVER_TYPE_UPLOAD:
      $title = t('New upload server');
      $description = t('A upload server allows users to upload files to MediaMosa.');
      break;
  }

  $form = array();

  $form['new_server'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => $title,
    '#description' => $description,
  );

  $form['new_server']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Server name'),
    '#description' => t('The name for the server.'),
    '#required' => TRUE,
    '#default_value' => mediamosa_node::default_value('title', $node),
    '#maxlength' => mediamosa_server_db::SERVER_NAME_LENGTH,
  );

  $form['new_server'][mediamosa_server_db::DESCRIPTION] = array(
    '#type' => 'textarea',
    '#required' => TRUE,
    '#title' => t('Description'),
    '#description' => t('Description of the server.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_server_db::DESCRIPTION, $node),
    '#maxlength' => mediamosa_server_db::DESCRIPTION_LENGTH,
  );

  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_DOWNLOAD, mediamosa_server_db::SERVER_TYPE_STILL, mediamosa_server_db::SERVER_TYPE_STREAMING))) {
    $form['new_server'][mediamosa_server_db::URI] = array(
      '#type' => 'textfield',
      '#required' => TRUE,
      '#title' => t('URI'),
      '#description' => t('{TICKET} is available for use in the URI.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::URI, $node),
      '#maxlength' => mediamosa_server_db::URI_LENGTH,
    );
  }

  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_JOB_PROCESSOR))) {
    $form['new_server'][mediamosa_server_db::IP_ADDRESS] = array(
      '#type' => 'textfield',
      '#title' => t('IP address'),
      '#description' => t('IP address of server.'),
      '#required' => TRUE,
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::IP_ADDRESS, $node),
      '#maxlength' => mediamosa_server_db::IP_ADDRESS_LENGTH,
    );

    $form['new_server'][mediamosa_server_db::SLOTS] = array(
      '#type' => 'textfield',
      '#title' => t('Slots'),
      '#required' => TRUE,
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::SLOTS, $node),
    );

    $form['new_server'][mediamosa_server_db::TOOLS] = array(
      '#type' => 'textfield',
      '#title' => t('Tools'),
      '#description' => t('Set tools for this server in the following format: ffmpeg|STILL|lin2win.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::TOOLS, $node),
    );
  }

  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_UPLOAD))) {
    $form['new_server'][mediamosa_server_db::URI_UPLOAD_PROGRESS] = array(
      '#type' => 'textfield',
      '#title' => t('URI upload progress'),
      '#required' => TRUE,
      '#description' => t('Optional URI for client side upload progress information.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::URI_UPLOAD_PROGRESS, $node),
      '#maxlength' => mediamosa_server_db::URI_UPLOAD_PROGRESS_LENGTH,
    );
  }

  if (in_array($server_type, array(mediamosa_server_db::SERVER_TYPE_STREAMING))) {
    $form['new_server'][mediamosa_server_db::CONTAINERS] = array(
      '#type' => 'textfield',
      '#title' => t('Containers'),
      '#required' => TRUE,
      '#description' => t('Set containers for this server in the following format: asf|flv|mpeg|avi|ogg|mov;mp4;m4a;3gp;3g2;mj2|mp3.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::CONTAINERS, $node),
      '#maxlength' => mediamosa_server_db::CONTAINERS_LENGTH,
    );

    $form['new_server'][mediamosa_server_db::OBJECT_CODE] = array(
      '#type' => 'textarea',
      '#title' => t('Object code'),
      '#description' => t('{WIDTH}, {HEIGHT}, {HEIGHT_PLUS_20}, {MEDIAFILE_ID}, {TICKET_URI}, {AUTOPLAY} (true or false), {AUTOPLAY_NUM} (1 or 0), {IF_START}, {/IF_START}, {IF_DURATION}, {/IF_DURATION} are available for use in this field.'),
      '#default_value' => mediamosa_node::default_value(mediamosa_server_db::OBJECT_CODE, $node),
    );
  }
  else {
    $form['new_server'][mediamosa_server_db::CONTAINERS] = array(
      '#type' => 'hidden',
      '#value' => '',
    );
    $form['new_server'][mediamosa_server_db::OBJECT_CODE] = array(
      '#type' => 'hidden',
      '#value' => '',
    );
  }

  $form['new_server'][mediamosa_server_db::SERVER_STATUS] = array(
    '#type' => 'select',
    '#required' => TRUE,
    '#options' => $server_status_options,
    '#title' => t('Status'),
    '#description' => t('Availability of server.'),
    '#default_value' => mediamosa_node::default_value(mediamosa_server_db::SERVER_STATUS, $node),
  );

  $form['#redirect'] = 'admin/mediamosa/config/server';

  return $form;
}
