<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
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
 * PDF tool functions.
 */

/**
 * URL: job/$job_id/exec/pdf
 * Method: GET
 */
class mediamosa_rest_call_job_exec_pdf extends mediamosa_rest_call_job_exec {

  // ------------------------------------------------------- Functions (public).
  public function get_var_setup() {
    // Get the parent's variables.
    $var_setup = parent::get_var_setup();

    return $var_setup;
  }

  public function do_call() {
    // Do the basic parent's function.
    parent::do_call();

    $job_type = 'mediamosa_tool_pdf';

    // Execution string.
    // Go to the subdirectory.
    // Create swf from pdf.
    // Copy the result to transcode directory.
    // Remove the temporary directory (and files).
    $execution_string = strtr('cd @file_location; pdf2swf -b @mediafile_location -o @mediafile_id -T 9 -f; cp @mediafile_id ../@mediafile_id.tool; cd ..; rm @file_location -r;', array(
      '@file_location' => $this->file_location,
      '@mediafile_location' => mediamosa_configuration_storage::mediafile_filename_get($this->mediafile),
      '@mediafile_id' => $this->mediafile_id,
    ));

    parent::do_call_end($job_type, $execution_string);
  }
}
