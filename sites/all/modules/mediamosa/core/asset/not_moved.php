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
 * Code that we will not moved (yet).
 */

/*
+--------+--------------------------+---------------+--------+----------------------------------+--------+-------------+
| uri_id | uri                      | response_type | method | function_call                    | status | vpx_version |
+--------+--------------------------+---------------+--------+----------------------------------+--------+-------------+
|     71 | internal/check_existence | text/xml      | GET    | media_management_check_existence | active | 1.1.0       |
+--------+--------------------------+---------------+--------+----------------------------------+--------+-------------+
 */
function media_management_check_existence($a_args) {
  $a_parameters = array(
    'table' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'table'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'cell' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'cell'),
      'type' => 'alphanum',
      'required' => TRUE,
    ),
    'id' => array(
      'value' => vpx_get_parameter_2($a_args['get'], 'id'),
      'type' => 'skip',
      'required' => TRUE,
    ),
  );

// valideer alle parameters op aanwezigheid en type
  $result = vpx_validate($a_parameters);
  if (vpx_check_result_for_error($result)) {
    return new rest_response($result);
  }

  if (vpx_count_rows($a_parameters['table']['value'], array($a_parameters['cell']['value'], $a_parameters['id']['value']))) {
    return new rest_response(vpx_return_error(ERRORCODE_OKAY));
  }
  else {
    switch ($a_parameters['table']['value']) {
      case "mediafile": return new rest_response(vpx_return_error(ERRORCODE_MEDIAFILE_NOT_FOUND, array('@mediafile_id' => $a_parameters['id']['value']))); break;
      case "asset": return new rest_response(vpx_return_error(ERRORCODE_ASSET_NOT_FOUND, array('@asset_id' => $a_parameters['id']['value']))); break;
      default: return new rest_response(vpx_return_error(ERRORCODE_EMPTY_RESULT));
    }
  }
}


/**
 * Controleer of er lopende jobs zijn voor een asset.
 * Als alle jobs gestopt zijn / of nog niet begonnen, verwijder die jobs dan.
 *
 * Code is verbeterd en verplaatst naar vpx_jobs.
 *
 */
function _media_management_delete_jobs($asset_id) {

  return _vpx_jobs_cancel_job_by_asset($asset_id);
}


