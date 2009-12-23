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
  * The maintenance module.
  */

/**
 * Implementation of hook_view().
 */
function mediamosa_server_view($node, $build_mode = 'full') {
  // Default parameters
  $rows = array(
    array(t('Title'), check_plain($node->title)),
    array(t('Status'), ($node->server_status) ? t('On') : t('Off')),
    array(t('Server type'), $node->server_type),
  );

  foreach (array(
    'uri' => t('URI'),
    'uri_upload_progress' => t('URI upload progress'),
    'containers' => t('Containers'),
    'server_name' => t('Server name'),
    'ip_address' => t('IP Address'),
    'slots' => t('Slots'),
    'tools' => t('Tools'),
  ) as $key => $title) {
    if (!is_null($node->{$key})) {
      $rows[] = array($title, check_plain($node->{$key}));
    }
  }

  // Add timestamps
  $rows[] = array(t('Created'), format_date($node->created, 'short'));
  $rows[] = array(t('Changed'), format_date($node->changed, 'short'));

  $node->content['transcode_profile'] = array(
    '#markup' => theme('table', array(t('Parameter'), t('Value')), $rows),
  );

  return $node;
}

/**
 * Implementation of hook_form().
 */
function mediamosa_server_form($node, $form_state) {
  // If we're inserting a new nod, set some defaults:
  $form = array();

  $form['fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('MediaMosa server'),
    '#collapsible' => FALSE,
  );

  $form['fieldset']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#description' => t('Description of the server.'),
    '#default_value' => $node->title,
    '#required' => TRUE,
  );

  if (!isset($node->nid)) {
    $form['fieldset']['#description'] = t('Create a new MediaMosa server.');
    $form['fieldset']['server_type'] = array(
      '#type' => 'select',
      '#title' => t('Server type'),
      '#options' => array(
        mediamosa_server_db::SERVER_TYPE_DOWNLOAD => t('Download server'),
        mediamosa_server_db::SERVER_TYPE_STILL => t('Still server'),
        mediamosa_server_db::SERVER_TYPE_STREAMING => t('Streaming server'),
        mediamosa_server_db::SERVER_TYPE_TRANSCODE => t('Transcode server'),
        mediamosa_server_db::SERVER_TYPE_UPLOAD => t('upload server'),
      ),
      '#description' => t('The server type cannot be changed once the server is created.'),
    );
  }
  else {
    // Alter the fieldset title and description
    $form['fieldset']['#title'] = check_plain($node->title);
    $form['fieldset']['#description'] = t('Server type: @type', array('@type' => $node->server_type));

    // Get all optional server property options
    $options = _mediamosa_server_get_server_options($node->server_type);

    // Optional server properties
    if (in_array('uri', $options)) {
      $form['fieldset']['uri'] = array(
        '#type' => 'textfield',
        '#title' => t('URI'),
        '#description' => t('{TICKET} is available for use in the URI.'),
        '#default_value' => $node->uri,
        '#required' => TRUE,
        '#maxlength' => mediamosa_server_db::URI_LENGTH,
      );
    }
    if (in_array('containers', $options)) {
      $form['fieldset']['containers'] = array(
        '#type' => 'textfield',
        '#title' => t('Containers'),
        '#description' => t('Set containers for this server in the following format: asf|flv|mpeg|avi|ogg|mov;mp4;m4a;3gp;3g2;mj2|mp3.'),
        '#default_value' => $node->containers,
        '#required' => TRUE,
        '#maxlength' => mediamosa_server_db::CONTAINERS_LENGTH,
      );
    }
    if (in_array('object_code', $options)) {
      $form['fieldset']['object_code'] = array(
        '#type' => 'textarea',
        '#title' => t('Object code'),
        '#description' => t('{WIDTH}, {HEIGHT}, {HEIGHT_PLUS_20}, {MEDIAFILE_ID}, {TICKET_URI}, {AUTOPLAY} (true or false), {AUTOPLAY_NUM} (1 or 0), {IF_START}, {/IF_START}, {IF_DURATION}, {/IF_DURATION} are available for use in this field.'),
        '#default_value' => $node->object_code,
      );
    }
    if (in_array('server_name', $options)) {
      $form['fieldset']['server_name'] = array(
        '#type' => 'textfield',
        '#title' => t('Server name'),
        '#description' => t('Name of server.'),
        '#default_value' => $node->server_name,
        '#required' => TRUE,
        '#maxlength' => mediamosa_server_db::SERVER_NAME_LENGTH,
      );
    }
    if (in_array('ip_address', $options)) {
      $form['fieldset']['ip_address'] = array(
        '#type' => 'textfield',
        '#title' => t('IP address'),
        '#description' => t('IP address of server.'),
        '#default_value' => $node->ip_address,
        '#required' => TRUE,
        '#maxlength' => mediamosa_server_db::IP_ADDRESS_LENGTH,
      );
    }
    if (in_array('slots', $options)) {
      $form['fieldset']['slots'] = array(
        '#type' => 'select',
        '#options' => drupal_map_assoc(range(1, 64)),
        '#title' => t('Slots'),
        '#description' => t('The number of slots for this server.'),
        '#default_value' => $node->slots,
      );
    }
    if (in_array('tools', $options)) {
      $form['fieldset']['tools'] = array(
        '#type' => 'textfield', // @TODO: convert to select list?
        '#title' => t('Tools'),
        '#description' => t('Set tools for this server in the following format: ffmpeg|STILL|lin2win.'),
        '#default_value' => $node->tools,
        '#required' => TRUE,
        '#maxlength' => mediamosa_server_db::TOOLS_LENGTH,
      );
    }
    if (in_array('uri_upload_progress', $options)) {
      $form['fieldset']['uri_upload_progress'] = array(
        '#type' => 'textfield',
        '#title' => t('URI upload progress'),
        '#description' => t('Optional URI for client side upload progress information.'),
        '#default_value' => $node->uri_upload_progress,
        '#maxlength' => mediamosa_server_db::URI_UPLOAD_PROGRESS_LENGTH,
      );
    }
  }

  // New servers are disabled by default
  if (isset($node->nid)) {
    $form['fieldset']['server_status'] = array(
      '#type' => 'select',
      '#title' => t('Server status'),
      '#options' => array(1 => t('On'), 0 => t('Off')),
      '#description' => t('Availability of server.'),
      '#default_value' => $node->server_status,
    );
  }

  return $form;
}

/**
 * Implementation of hook_update().
 */
function mediamosa_server_update($node) {
  // Delete the old revision
  db_delete(mediamosa_server_db::TABLE_NAME)
    ->condition('vid', $node->vid)
    ->execute();

  // Insert the new revision
  db_insert(mediamosa_server_db::TABLE_NAME)
    ->fields(array(
      'nid' => $node->nid,
      'vid' => $node->vid,
      mediamosa_server_db::SERVER_STATUS => $node->server_status,
      mediamosa_server_db::URI => (isset($node->uri)) ? $node->uri : NULL,
      mediamosa_server_db::CONTAINERS => (isset($node->containers)) ? $node->containers : NULL,
      mediamosa_server_db::OBJECT_CODE => (isset($node->object_code)) ? $node->object_code : NULL,
      mediamosa_server_db::SERVER_NAME => (isset($node->server_name)) ? $node->server_name : NULL,
      mediamosa_server_db::IP_ADDRESS => (isset($node->ip_address)) ? $node->ip_address : NULL,
      mediamosa_server_db::SLOTS => (isset($node->slots)) ? $node->slots : NULL,
      mediamosa_server_db::TOOLS => (isset($node->tools)) ? $node->tools : NULL,
      mediamosa_server_db::URI_UPLOAD_PROGRESS => (isset($node->uri_upload_progress)) ? $node->uri_upload_progress : NULL,
    ))
    ->execute();
}

/**
 * Implementation of hook_insert().
 */
function mediamosa_server_insert($node) {
  db_insert(mediamosa_server_db::TABLE_NAME)
    ->fields(array(
      'nid' => $node->nid,
      'vid' => $node->vid,
      mediamosa_server_db::SERVER_ID => $node->nid,
      mediamosa_server_db::SERVER_TYPE => $node->server_type,
      mediamosa_server_db::SERVER_STATUS => 0, // 'Off' by default
    ))
    ->execute();
}

/**
 * Implementation of hook_delete().
 */
function mediamosa_server_delete($node) {
  db_delete(mediamosa_server_db::TABLE_NAME)
    ->condition('nid', $node->nid)
    ->execute();
}

/**
 * Helper function to fetch the optional parameters per server type.
 */
function _mediamosa_server_get_server_options($server_type) {
  $form_elements = array();
  switch ($server_type) {
    case mediamosa_server_db::SERVER_TYPE_STILL:
    case mediamosa_server_db::SERVER_TYPE_DOWNLOAD:
      $form_elements = array('uri');
      break;

    case mediamosa_server_db::SERVER_TYPE_STREAMING:
      $form_elements = array('uri', 'containers', 'object_code');
      break;

    case mediamosa_server_db::SERVER_TYPE_TRANSCODE:
      $form_elements = array('server_name', 'ip_address', 'slots', 'tools');
      break;

    case mediamosa_server_db::SERVER_TYPE_UPLOAD:
      $form_elements = array('uri', 'uri_upload_progress');
      break;
  }

  return $form_elements;
}
