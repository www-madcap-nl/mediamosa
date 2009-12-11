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

function _mediamosa_restcall($method, $uri, $data = array(), $report_errors = TRUE) {
  // Merge in defaults
  $data += array(
    'app_id' => 1,
  );

  // Prepare the uri
  $uri = url(MEDIAMOSA_MENU_PATH . $uri, array('absolute' => TRUE));
  if ($method == 'GET') {
    $uri .= '?' . http_build_query($data);
  }

  // Use the current session ID in the request
  foreach ($_COOKIE as $key => $value) {
    if (strpos($key, 'SESS') === 0) {
      $cookie = array('Cookie' => $key .'='. $value);
      break;
    }
  }

  // Perform the actual request
  $result = drupal_http_request(
    $uri,
    array(
      'method' => $method,
      'data' => http_build_query($data),
      'headers' => $cookie,
    )
  );
  try {
    $xml = new SimpleXMLElement($result->data);
    if ($report_errors && $xml->header->request_result != 'success') {
      drupal_set_message($uri);
      drupal_set_message($xml->header->request_result_id .': '. $xml->header->request_result_description, 'error');
    }
    return $xml;
  }
  catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
    return FALSE;
  }
}

/**
 * This function returns a themed pager
 */
function _mediamosa_browse_pager($page, $item_count_total, $item_limit, $element = 0) {
  global $pager_page_array, $pager_total, $pager_total_items;
  $pager_page_array = array($element => $page);
  $pager_total_items[$element] = $item_count_total; // aantal items
  if (!is_array($element)) {
    $pager_total[$element] = ceil($item_count_total / $item_limit); // aantal weer te geven page links
  }
  return theme('pager', array('quantity' => 10));
}
