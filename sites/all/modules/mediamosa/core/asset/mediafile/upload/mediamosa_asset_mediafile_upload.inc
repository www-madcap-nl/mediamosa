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
  * Upload related function for mediafiles.
  */

class mediamosa_asset_mediafile_upload {






  /**
   * Handle PUT upload.
   *
   * 1.x: _vpx_upload_handle_file_put
   *
   * @param string $destination
   * @param $job_id
   */
  static public function handle_file_put($destination, $job_id) {
    // If mediafile exists, rename it.
    if (file_exists($destination)) {
      $destination_filename_after_upload = $destination;
      $destination .= '.upload_temp';
    }
/*
    // Put upload job to INPROGRESS.
  $a_args = array(
    'uri' => array(
      'job_id' => $job_id,
    ),
    'post' => array(
      'status' => JOBSTATUS_INPROGRESS,
    )
  );
  vpx_jobs_set_job_status($a_args);

  // Open the new file
  $destination_file = @fopen($destination, 'w');
  if (!$destination_file) {
    throw new vpx_exception(0, "Unable to write to '". $destination ."'.", VPX_EXCEPTION_SEVERITY_HIGH);
  }

  // Open the stream
  $stream = fopen('php://input', 'r');
  if ($stream === FALSE) {
    throw new vpx_exception(0, 'Unable to open stream', VPX_EXCEPTION_SEVERITY_HIGH);
  }

  $b_continue = TRUE;

  $i_byte_count = 0;
  $last_update = $i_timestamp = time();
  while (($kb = fread($stream, CHUNK_SIZE)) && $b_continue) {
    $b_continue = (@fwrite($destination_file, $kb, CHUNK_SIZE));
    $i_byte_count += CHUNK_SIZE;

    if ($last_update != time() && ((time() - $i_timestamp) % UPLOAD_PROGRESS_INTERVAL) == 0) {
      $a_args = array('uri' => array('job_id' => $job_id), 'post' => array('uploaded_file_size' => $i_byte_count));
      $last_update = time();
      vpx_jobs_set_upload_progress($a_args);
    }

    if (!$b_continue) {
      return FALSE;
    }
    header('HTTP/1.1 201 Created');
  }

  // Sluit de streams en files netjes af
  fclose($destination_file);
  fclose($stream);

  // indien de mediafile al bestond, hernoem de nieuwe dan naar mediafile_id
  if (isset($destination_filename_after_upload)) {
    rename($destination, $destination_filename_after_upload);
  }

  return TRUE;*/
  }
}