<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2010 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * Integrity check PHP file
 */

// Get Drupal.
define('DRUPAL_ROOT', $_SERVER['DOCUMENT_ROOT']);
chdir($_SERVER['DOCUMENT_ROOT']);
include_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);

// Error reporting.
error_reporting(E_ALL);

// Check media record.
function check_media_records() {
  $result = mediamosa_db::db_query("
    SELECT m.#mediafile_id, m.#created, m.#changed, m.#asset_id_root, m.#app_id, m.#owner_id, mm.#filesize, mm.#mime_type
    FROM {#mediafile} AS m
    LEFT JOIN {#mediafile_metadata} AS mm USING (#mediafile_id)
    WHERE m.#is_still = :is_still_false", array(
    '#mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
    '#mediafile_metadata' => mediamosa_asset_mediafile_metadata_db::TABLE_NAME,
    '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
    '#created' => mediamosa_asset_mediafile_db::CREATED,
    '#changed' => mediamosa_asset_mediafile_db::CHANGED,
    '#asset_id_root' => mediamosa_asset_mediafile_db::ASSET_ID_ROOT,
    '#app_id' => mediamosa_asset_mediafile_db::APP_ID,
    '#owner_id' => mediamosa_asset_mediafile_db::OWNER_ID,
    '#filesize' => mediamosa_asset_mediafile_metadata_db::FILESIZE,
    '#mime_type' => mediamosa_asset_mediafile_metadata_db::MIME_TYPE,
    '#is_still' => mediamosa_asset_mediafile_db::IS_STILL,
    ':is_still_false' => mediamosa_asset_mediafile_db::IS_STILL_FALSE,
  ));

  foreach ($result as $mediafile) {
    // Check if file exists.
    if (!file_exists(mediamosa_configuration_storage::mediafile_id_filename_get($mediafile['mediafile_id']))) {
      mediamosa_db::db_query("
        INSERT INTO {#mediamosa_log_integrity_check}
          (#type, #object_id, #app_id, #owner_id, #created, #changed, #details) VALUES
          (:missing_mediafile, :object_id, :app_id, :owner_id, :created, :changed, :details)", array(
          '#mediamosa_log_integrity_check' => mediamosa_integrity_check_db::TABLE_NAME,
          '#type' => mediamosa_integrity_check_db::TYPE,
          '#object_id' => mediamosa_integrity_check_db::OBJECT_ID,
          '#app_id' => mediamosa_integrity_check_db::APP_ID,
          '#owner_id' => mediamosa_integrity_check_db::OWNER_ID,
          '#created' => mediamosa_integrity_check_db::CREATED,
          '#changed' => mediamosa_integrity_check_db::CHANGED,
          '#details' => mediamosa_integrity_check_db::DETAILS,
          ':missing_mediafile' => mediamosa_integrity_check_db::TYPE_MISSING_MEDIAFILE,
          ':object_id' => $mediafile['mediafile_id'],
          ':app_id' => $mediafile['app_id'],
          ':owner_id' => $mediafile['owner_id'],
          ':created' => $mediafile['created'],
          ':changed' => $mediafile['changed'],
          ':details' => $mediafile['filesize'] == '' ? 'Never succesfully analysed...' : 'Mime-type: ' . $mediafile['mime_type'],
      ));
    }
    // Sleep 0.01 seconds *100.000 mediafiles= 17 minuten.
    usleep(10000);
  }
}

// Check media files.
function check_media_files() {
  // Base folder.
  $dir = mediamosa_configuration_storage::get_data_location();
  $dh = opendir($dir);
  $missing_db_mediafiles = array();

  while (($folder = readdir($dh)) !== FALSE) {
    // Is it "." or ".."?
    if (!is_dir($dir . DIRECTORY_SEPARATOR . $folder) || strpos($folder, '..') === 0 || strpos($folder, '.') === 0 || drupal_strlen($folder) > 1) {
      continue;
    }

    // Open the sub directory.
    $fh = opendir($dir . DIRECTORY_SEPARATOR . $folder);
    while (($file = readdir($fh)) !== FALSE) {

      // Is it "." or ".."?
      if (strpos($file, '.') === 0 || strpos($file, '..') === 0) {
        continue;
      }

      // Check the file in the db.
      $result = mediamosa_db::db_query("
        SELECT COUNT(*)
        FROM {#mediafile}
        where #mediafile_id = :mediafile_id", array(
        '#mediafile' => mediamosa_asset_mediafile_db::TABLE_NAME,
        '#mediafile_id' => mediamosa_asset_mediafile_db::ID,
        ':mediafile_id' => $file,
      ));

      if ($result->fetchField() == 0) {
        // Collect the data.
        $file_path = $dir . DIRECTORY_SEPARATOR . $folder . DIRECTORY_SEPARATOR . $file;
        $finfo = stat($file_path);
        $more_info = exec('ls -sla ' . $file_path);
        // Make error message.
        mediamosa_db::db_query("
          INSERT INTO {#mediamosa_log_integrity_check}
            (#type, #object_id, #size, #mtime, #ctime, #details, #created) VALUES
            (:missing_mediarecord, :object_id, :size, :mtime, :ctime, :details, NOW())", array(
            '#mediamosa_log_integrity_check' => mediamosa_integrity_check_db::TABLE_NAME,
            '#type' => mediamosa_integrity_check_db::TYPE,
            '#object_id' => mediamosa_integrity_check_db::OBJECT_ID,
            '#size' => mediamosa_integrity_check_db::SIZE,
            '#mtime' => mediamosa_integrity_check_db::MTIME,
            '#ctime' => mediamosa_integrity_check_db::CTIME,
            '#details' => mediamosa_integrity_check_db::DETAILS,
            '#created' => mediamosa_integrity_check_db::CREATED,
            ':missing_mediarecord' => mediamosa_integrity_check_db::TYPE_MISSING_MEDIARECORD,
            ':object_id' => $file,
            ':size' => $finfo['size'],
            ':mtime' => $finfo['mtime'],
            ':ctime' => $finfo['ctime'],
            ':details' => $more_info,
        ));
      }
    }

    closedir($fh);
  }

  closedir($dh);
}

/*
function log_message($type, $items) {
  if (!is_array($items)) {
    $items = array($items);
  }
  $values = array();
  foreach ($items as $item) {
    $values[] = "('". $type ."', '". $item ."')";
  }
  if (count($values)) {
    db_query("INSERT INTO {mediamosa_log_integrity_check} (type, object) VALUES ". implode(', ', $values));
    ic_watchdog('integrity_check', $type . ': '. count($values) . ' found.');
  }
}

function check_still_records() {
  $missing_stills = array();
  $skip_files = array();

  $resource = db_query("SELECT mediafile_id AS still_id FROM {mediafile} WHERE is_still = 'TRUE'");
  while ($still = db_fetch_array($resource)) {
    // check if file exists
    $file = SAN_NAS_BASE_PATH. DS . STILL_LOCATION . DS . $still['still_id']{0} . DS . $still['still_id'];
    if (!file_exists($file)) {
      $missing_stills[] = $still['still_id'];
    }
    else {
      $skip_files[] = $still['still_id'];
    }
  }
  log_message('stillrecord', $missing_stills);
  return $skip_files;
}

function check_still_files($skip_files) {
  $dir = SAN_NAS_BASE_PATH . DS . STILL_LOCATION;
  $dh = opendir($dir);
  $missing_db_stills = array();
  while (($folder = readdir($dh)) !== FALSE) {
    if (!is_dir($dir . DS . $folder) || strpos($folder, '.') === 0 || drupal_strlen($folder) > 1) {
      continue;
    }
    $fh = opendir($dir . DS . $folder);
    while (($file = readdir($fh)) !== FALSE) {
      if (strpos($file, '.') === 0 || in_array($file, $skip_files)) {
        continue;
      }
      $missing_db_stills[] = $file;
    }
    closedir($fh);
  }
  closedir($dh);
  log_message('stillfile', $missing_db_stills);
}
*/

// Start.
watchdog('integrity_check', 'running...');
variable_set('mediamosa_integrity_run_date_start', date('c'));

// Empty log table.
db_query('TRUNCATE TABLE {mediamosa_log_integrity_check}');

// Run checks.
check_media_records();
check_media_files();

// TODO: broken, not ported.
// check_still_records();
// check_still_files();

// End.
variable_set('mediamosa_integrity_run_date_end', date('c'));
watchdog('integrity_check', 'ended...');
