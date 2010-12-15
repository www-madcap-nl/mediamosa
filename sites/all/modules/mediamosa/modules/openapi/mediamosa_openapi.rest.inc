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
 * OpenAPI rest calls.
 */


/**
 * URI: /openapi/search
 * Method: GET
 */
class mediamosa_rest_call_openapi_search extends mediamosa_rest_call {
  // ------------------------------------------------------------------ Consts.
  // Rest vars;
  const QUERY = 'query';
  const QLIMIT = 'qlimit';
  const QOFFSET = 'qoffset';
  const QORDER = 'qorder';
  const QDIRECTION = 'qdirection';
  const SUBJECT = 'subject';
  const LANGUAGE = 'language';
  const LOCATION = 'location';
  const SINCE = 'since';
  const UNTIL = 'until';
  const AUTHOR = 'author';
  const CONTRIBUTOR = 'contributor';
  const PUBLISHER = 'publisher';
  const CATEGORY = 'category';
  const FORMAT = 'format';
  const DURATION = 'duration';
  const CALLBACK = 'callback';

  // Other vars;
  const DEFAULT_LIMIT = 10;
  const DEFAULT_OFFSET = 0;

            // ------------------------------------------------------------------ Functions (public).
  public function get_var_setup() {
    $a_var_setup = array();

    $a_var_setup = array(
      self::VARS => array(
        self::QUERY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'Search query.',
          self::VAR_IS_REQUIRED => self::VAR_IS_REQUIRED_YES,
        ),
        self::SUBJECT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The subject mapped to DC:subject.',
        ),
        self::LANGUAGE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The language mapped to DC:language.',
        ),
        self::LOCATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The location mapped to DC:coverage_spatial.',
        ),
        self::SINCE => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'The since mapped to videotimestamp (search from a date).',
        ),
        self::UNTIL => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_DATETIME_UTC,
          self::VAR_DESCRIPTION => 'The until mapped to videotimestamp (search till a date).',
        ),
        self::AUTHOR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The author mapped to DC:creator.',
        ),
        self::CONTRIBUTOR => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The contributor mapped to DC:contributor.',
        ),
        self::PUBLISHER => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The publisher mapeed to DC:publisher.',
        ),
        self::CATEGORY => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA_NUM,
          self::VAR_DESCRIPTION => 'The category mapped to coll_id.',
        ),
        self::FORMAT => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The format mapped to content type of mediafile.',
        ),
        self::DURATION => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_STRING,
          self::VAR_DESCRIPTION => 'The duration mappped to the duration of mediafile.',
        ),
        self::CALLBACK => array(
          self::VAR_TYPE => mediamosa_sdk::TYPE_ALPHA,
          self::VAR_DESCRIPTION => 'The callback for JSON.',
        ),
      )
    );

    // Include limit, offset vars.
    $a_var_setup = self::get_var_setup_range($a_var_setup);
    // Alter the params.
    $a_var_setup[self::VARS][self::QLIMIT] = $a_var_setup[self::VARS][self::LIMIT];
    $a_var_setup[self::VARS][self::QOFFSET] = $a_var_setup[self::VARS][self::OFFSET];
    unset($a_var_setup[self::VARS][self::LIMIT]);
    unset($a_var_setup[self::VARS][self::OFFSET]);

    // Include order by.
    $a_var_setup = self::get_var_setup_order_by($a_var_setup);
    // Alter the params.
    $a_var_setup[self::VARS][self::QORDER] = $a_var_setup[self::VARS][self::ORDER_BY];
    $a_var_setup[self::VARS][self::QDIRECTION] = $a_var_setup[self::VARS][self::ORDER_DIRECTION];
    unset($a_var_setup[self::VARS][self::ORDER_BY]);
    unset($a_var_setup[self::VARS][self::ORDER_DIRECTION]);

    // Enrich with required REST vars.
    return self::get_var_setup_default($a_var_setup);
  }

  public function do_call() {
    $mediamosa = mediamosa::get();

    $app_ids = $this->get_param_value_app();

    // Get params.
    $limit = $this->get_param_value(self::QLIMIT);
    $offset = $this->get_param_value(self::QOFFSET);
    $order_by = $this->get_param_value(self::QORDER);
    $order_direction = $this->get_param_value(self::QDIRECTION);
    $callback = $this->get_param_value(self::CALLBACK);

    $cql = '';

    // Get params with escape.
    $query = addslashes($this->get_param_value(self::QUERY));
    $subject = addslashes($this->get_param_value(self::SUBJECT));
    $language = addslashes($this->get_param_value(self::LANGUAGE));
    $location = addslashes($this->get_param_value(self::LOCATION));
    $since = addslashes($this->get_param_value(self::SINCE));
    $until = addslashes($this->get_param_value(self::UNTIL));
    $author = addslashes($this->get_param_value(self::AUTHOR));
    $contributor = addslashes($this->get_param_value(self::CONTRIBUTOR));
    $publisher = addslashes($this->get_param_value(self::PUBLISHER));
    $category = addslashes($this->get_param_value(self::CATEGORY));
    $format = addslashes($this->get_param_value(self::FORMAT));
    $duration = addslashes($this->get_param_value(self::DURATION));

    if ($query) {
      $cql = '(title="' . $query . '" OR description="' . $query . '")';
    }

    if ($subject) {
      $cql .= ($cql ? ' AND ' : '') . 'subject="' . $subject . '"';
    }

    if ($language) {
      $cql .= ($cql ? ' AND ' : '') . 'language="' . $language . '"';
    }

    if ($location) {
      $cql .= ($cql ? ' AND ' : '') . 'coverage_spatial="' . $location . '"';
    }

    if ($this->isset_param(self::SINCE)) {
      $cql .= ($cql ? ' AND ' : '') . 'videotimestamp>"' . $since . '"';
    }

    if ($this->isset_param(self::UNTIL)) {
      $cql .= ($cql ? ' AND ' : '') . 'videotimestamp<"' . $until . '"';
    }

    if ($author) {
      $cql .= ($cql ? ' AND ' : '') . 'creator="' . $author . '"';
    }

    if ($contributor) {
      $cql .= ($cql ? ' AND ' : '') . 'contributor="' . $contributor . '"';
    }

    if ($publisher) {
      $cql .= ($cql ? ' AND ' : '') . 'publisher="' . $publisher . '"';
    }

    if ($category) {
      $cql .= ($cql ? ' AND ' : '') . 'coll_id="' . $category . '"';
    }

    if ($format) {
      $cql .= ($cql ? ' AND ' : '') . 'mediafile_container_type="' . $format . '"';
    }

    if ($duration) {
      $cql .= ($cql ? ' AND ' : '') . 'mediafile_duration="' . $duration . '"';
    }

    // TODO: Implement callback.

    // Add possible order by to the CQL.
    if ($order_by != '' && $cql) {
      $cql .= (!empty($cql) ? ' ' : '') . 'sortby ' . $order_by;
      $cql .= (empty($order_direction) ? '' : '/' . (mediamosa_unicode::strtolower($order_direction) == mediamosa_type::ORDER_DIRECTION_ASC ? 'ascending' : 'descending'));
    }

    if ($cql) {
      $asset_ids = mediamosa_search::asset(array(
        'app_ids' => $app_ids,
        'cql' => $cql,
        'limit' => $limit,
        'offset' => $offset,
      ));

      // Fill response.
      $items = mediamosa_asset::asset_collect(
        $asset_ids['asset_ids'],
        $app_ids,
        '',
        '',
        array(),
        '',
        '',
        $this->get_param_value(self::IS_APP_ADMIN),
        TRUE,
        FALSE,
        FALSE
      );

      foreach ($items as $item) {
        $mediamosa->add_item($item);
      }
    }
  }
}
