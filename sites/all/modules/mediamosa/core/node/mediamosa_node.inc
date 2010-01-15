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
  * Include file for core node types. Add new node type includes here.
  * This file is included on several places to ensure including all node
  * creation code.
  */

// Our node type hooks.
require_once 'mediamosa_node_mediamosa_server.inc';
require_once 'mediamosa_node_mediamosa_transcode_profile.inc';


class mediamosa_node {
  // ------------------------------------------------------------------ Consts.
  // Node types.
  const MEDIAMOSA_NODE_TYPE_TRANSCODE_PROFILE = 'mediamosa_transcode_profile';
  const MEDIAMOSA_NODE_TYPE_SERVER = 'mediamosa_server';
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
  );
}
