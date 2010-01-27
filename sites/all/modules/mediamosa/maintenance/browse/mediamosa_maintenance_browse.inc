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
  */
/**
 * Execute a REST call.
 * @param string $uri
 *  The URI of the REST call.
 * @param string $method
 *  The Method (POST/GET/PUT).
 * @param array $data
 *  Array name => value.
 */
function _mediamosa_restcall($uri, $method = 'GET', array $data = array()) {
  $options = array(
    'data' => $data,
    'method' => $method,
  );

  // Create the mediamosa connector.
  $o_connector = new mediamosa_connector();

  // Do request will trigger login.
  $o_response = $o_connector->request_internal($uri, $options);

  if (!$o_response) {
    drupal_set_message('Unable to execute REST call, check MediaMosa connector settings.', 'error');
    return FALSE;
  }

  if ($o_response->xml->header->request_result != 'success') {
    drupal_set_message($uri);
    drupal_set_message($o_response->xml->header->request_result_id .': '. $o_response->xml->header->request_result_description, 'error');
  }

  return $o_response;
}

/**
 * This function returns a themed pager
 */
function _mediamosa_maintenance_browse_pager($page, $item_count_total, $item_limit, $element = 0) {
  global $pager_page_array, $pager_total, $pager_total_items;

  $pager_page_array = array($element => $page);

  // Total items.
  $pager_total_items[$element] = $item_count_total;

  if (!is_array($element)) {
    // Number of pagelinks.
    $pager_total[$element] = ceil($item_count_total / $item_limit);
  }
  return theme('pager', array('quantity' => 10));
}

function _mediamosa_maintenance_browse_clean_string($string) {
  return drupal_ucfirst(str_replace('_', ' ', $string));
}
