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
 * REST call for downloading mediafiles created by tickets.
 */

/**
 * URI: /still/$ticket_id
 * Method: GET / POST
 *
 * replacement for still/index.php.
 */
class mediamosa_rest_call_download_mediafile_still extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const TICKET_ID = 'ticket_id';

    // ------------------------------------------------------------------ Get Var Setup.
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::TICKET_ID => array(
          self::VAR_TYPE => mediamosa_type::TYPE_TICKET_ID,
          self::VAR_DESCRIPTION => 'The ticket ID.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
      )
    );

    // All we need.
    return $a_var_setup;
  }

  // ------------------------------------------------------------------ Do Call.
  public function do_call() {
    $o_mediamosa = mediamosa::get();

    // Get the ticket ID.
    $ticket_id = $this->get_param_value(self::TICKET_ID);

    // Get filename.
    $file = mediamosa_configuration_storage::ticket_still_location_get() . $ticket_id;

    if (!file_exists($file)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_INVALID_STILL_TICKET, array('@ticket_id' => $ticket_id));
    }

    // Set headers.
    header('Content-Type: image/jpeg');

    // Make sure we send the header now.
    ob_clean();
    flush();

    // Download the image and delete the ticket.
    if (@readfile($file) !== FALSE) {
      mediamosa_io::unlink($file);
    }

    // Set the result, reponse object will know what to do.
    $o_mediamosa->set_result(mediamosa_response::SUCCESS, 200, $file);
  }
}
