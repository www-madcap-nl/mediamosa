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
  * Installer for ftp_batch_asset.
  */

require_once 'mediamosa_ftp_batch_asset_db.inc';

/**
 * Implement hook_ininstall.
 */
function mediamosa_ftp_batch_asset_install() {
// Create our table.
  mediamosa::drupal_install_schema(mediamosa_ftp_batch_asset_db::TABLE_NAME);
}

/**
 * Implement hook_uninstall.
 */
function mediamosa_ftp_batch_asset_uninstall() {
// Create our table.
  mediamosa::drupal_uninstall_schema(mediamosa_ftp_batch_asset_db::TABLE_NAME);
}

/**
 * Implement hook_schema().
 */
function mediamosa_ftp_batch_asset_schema() {
  $a_schema[mediamosa_ftp_batch_asset_db::TABLE_NAME] = array(
    'description' => t('This FTP Batch asset link table.'),
    'fields' => array(
      mediamosa_ftp_batch_asset_db::ID => array(
        'type' => 'serial',
        'not null' => TRUE,
        'description' => t('Primary key.'),
      ),
      mediamosa_ftp_batch_asset_db::ASSET_ID => array(
        'type' => 'varchar',
        'length' => mediamosa_db::HASH_ID_LENGTH,
        'not null' => TRUE,
        'description' => t('The asset ID that links to mediamosa_asset table.'),
      ),
      mediamosa_ftp_batch_asset_db::BATCH_ID => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'description' => t('The batch ID that links to mediamosa_ftp_batch table.'),
      ),
    ),
    'primary key' => array(mediamosa_ftp_batch_asset_db::ID),
    'indexes' => array(
      'idx_' . mediamosa_ftp_batch_asset_db::BATCH_ID => array(mediamosa_ftp_batch_asset_db::BATCH_ID),
      'idx_' . mediamosa_ftp_batch_asset_db::ASSET_ID => array(mediamosa_ftp_batch_asset_db::ASSET_ID),
    )
  );

  return $a_schema;
}
