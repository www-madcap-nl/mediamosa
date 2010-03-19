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
  * Include file for core node types. Add new node type includes here.
  * This file is included on several places to ensure including all node
  * creation code.
  */

class mediamosa_node {
  // ------------------------------------------------------------------ Consts.
  // Node types.
  const MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE = 'mediamosa_transcode_profile';
  const MEDIAMOSA_NODE_TYPE_SERVER = 'mediamosa_server';
  const MEDIAMOSA_NODE_TYPE_APP = 'mediamosa_app';

  // ------------------------------------------------------------------ Functions.
  /**
   * Get all node type of mediamosa.
   */
  static public function get_all_node_types() {
    return array_keys(mediamosa_node_info());
  }

  /**
   * Create basic node object to fill.
   *
   * @param $type
   * @param $title
   */
  static public function create_basic_node($type, $title) {
    $node = new stdClass();
    $node->uid = 1; // Admin
    $node->status = 1; // Published
    $node->type = $type;
    $node->language = LANGUAGE_NONE;
    $node->title = $title;
    return $node;
  }

  /**
   * Changes the form for mediamosa node type.
   *
   * @param array $form
   * @param array $form_state
   */
  static public function common_form(&$form, &$form_state) {
    $form['revision_information']['revision']['#default_value'] = '1';
    $form['revision_information']['log']['#default_value'] = ' ';

    // Remove the options block.
    unset($form['options']);

    // Add hidden values.
    $form['status'] = array(
      '#type' => 'hidden',
      '#value' => 1,
    );

    $form['sticky'] = array(
      '#type' => 'hidden',
      '#value' => 0,
    );

    $form['promote'] = array(
      '#type' => 'hidden',
      '#value' => 0,
    );

    // Make revision top item.
    $form['revision_information']['#weight'] = -100;

    // Remove the preview button for now (it doesn't work well atm).
    unset($form['actions']['preview']);
  }

  /**
   * Helper setting the default value.
   *
   * @param string $name
   * @param object $node
   */
  static public function default_value($name, $node, $default = '') {
    return isset($node->{$name}) ? $node->{$name} : $default;
  }
}

/**
 * Implementation of hook_node_info().
 */
function mediamosa_node_info() {
  return array(
    mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE => array(
      'name' => t('MediaMosa transcode profile'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE,
      'has_body' => FALSE,
      'description' => t('The transcode profile contains information about the codec type and settings for a video transcoding.'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER => array(
      'name' => t('MediaMosa server'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_SERVER,
      'has_body' => FALSE,
      'description' => t('MediaMosa servers execute jobs for transcoding, uploading and other related work.'),
    ),
    mediamosa_node::MEDIAMOSA_NODE_TYPE_APP => array(
      'name' => t('MediaMosa client application'),
      'base' => mediamosa_node::MEDIAMOSA_NODE_TYPE_APP,
      'has_body' => FALSE,
      'description' => t('MediaMosa client applications are clients that can login onto MediaMosa and execute REST calls. All data stored is stored under its client application ID; giving each client application its own database of assets, mediafiles etc.'),
    ),
  );
}

/**
 * Confirm node deletion.
 */
function _mediamosa_node_delete_confirm($form, &$form_state, $node) {

  // Call the node delete confirm form.
  return node_delete_confirm($form, $form_state, $node);
}

/**
 * Execute node deletion
 */
function _mediamosa_node_delete_confirm_submit($form, &$form_state) {

  // Call the node confirm submit function.
  node_delete_confirm_submit($form, $form_state);
}

/**
 * Implementation of hook_form_alter().
 */
function mediamosa_form_mediamosa_transcode_profile_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);
}

/**
 * Implementation of hook_form_alter().
 */
function mediamosa_form_mediamosa_server_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);
}

/**
 * Implementation of hook_form_alter().
 */
function mediamosa_form_mediamosa_app_node_form_alter(&$form, &$form_state) {
  // Change the form.
  mediamosa_node::common_form($form, $form_state);

  $form['#redirect'] = 'admin/mediamosa/config/app';
  $form_state['redirect'] = 'admin/mediamosa/config/app';
}
