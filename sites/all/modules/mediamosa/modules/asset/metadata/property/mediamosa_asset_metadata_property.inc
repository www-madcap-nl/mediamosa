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
  * The function used for asset media metadata property.
  */

class mediamosa_asset_metadata_property {

  // 1.x name: media_management_metadata_definition_get.
  function get_property_list($offset, $limit) {

    return mediamosa_db::db_query_range(
      "SELECT * FROM #table_name",
      $offset,
      $limit,
      array(
        '#table_name' => mediamosa_asset_metadata_property_db::TABLE_NAME,
      )
    );

    /*
  vpx_funcparam_add($a_funcparam, $a_args, 'app_id', VPX_TYPE_INT, TRUE);

  vpx_funcparam_add($a_funcparam, $a_args, 'limit', VPX_TYPE_INT, FALSE, 0, 1, MAX_RESULT_COUNT);
  vpx_funcparam_add($a_funcparam, $a_args, 'offset', VPX_TYPE_INT);
  vpx_funcparam_add($a_funcparam, $a_args, 'order_by', VPX_TYPE_ALPHANUM);
  vpx_funcparam_add($a_funcparam, $a_args, 'order_direction', VPX_TYPE_ORDER_DIRECTION);

  vpx_funcparam_add($a_funcparam, $a_args, 'include_default_definitions', VPX_TYPE_BOOL, FALSE, 'TRUE');
  vpx_funcparam_add($a_funcparam, $a_args, 'name', VPX_TYPE_ALPHANUM_UNDERSCORE);

  $app_id = vpx_funcparam_get_value($a_funcparam, 'app_id');

  $limit = vpx_funcparam_get_value($a_funcparam, 'limit', 0);
  $offset = vpx_funcparam_get_value($a_funcparam, 'offset', 0);
  $order_by = vpx_funcparam_get_value($a_funcparam, 'order_by');
  $order_direction = vpx_funcparam_get_value($a_funcparam, 'order_direction');

  $include_default_definitions = vpx_shared_boolstr2bool(vpx_funcparam_get_value($a_funcparam, 'include_default_definitions'));
  $name = vpx_funcparam_get_value($a_funcparam, 'name');

  $a_attach = array();

  // Check order by param
  if ($order_by != "") {
    $a_order_by = array('prop_id', 'name', 'type', 'created', 'changed');

    if (array_search($order_by, $a_order_by) === FALSE) {
      throw new vpx_exception_error_sort_field_error(array('@field' => $order_by));
    }

    $a_attach[] = sprintf('ORDER BY %s%s', $order_by, ($order_direction != '' ? ' ' . $order_direction : '')); // no db encoding we can trust the value (been checked)
  }

  if ($limit) {
    $a_attach[] = sprintf('LIMIT %d', $limit);

    if ($offset) {
      $a_attach[] = sprintf('OFFSET %d', $offset);
    }
  }

  $a_where = $a_where_and = array();
  $a_where[] = sprintf('app_id = %d', $app_id);
  if ($include_default_definitions) {
    $a_where[] = 'app_id = 0';
  }

  if (!empty($name)) {
    $a_where_and[] = sprintf("name = '%s'", $name);
  }

  $a_where_and[] = '(' . implode(' OR ', $a_where) . ')';

  db_set_active('data');
  $db_result = db_query("SELECT apd.*, apg.name AS group_name FROM {assetprop_definition} AS apd JOIN {assetprop_group} AS apg USING(propgroup_id) WHERE " . implode(' AND ', $a_where_and) . (count($a_attach) ? ' ' . implode(' ', $a_attach) : ''));
  db_set_active();

  if (!db_affected_rows($db_result)) {
    throw new vpx_exception_error_empty_result();
  }

  $o_rest_response = new rest_response(vpx_return_error(ERRORCODE_OKAY));

  while ($dbrow_prop = db_fetch_array($db_result)) {
    $o_rest_response->add_item(array(
      'prop_id' => $dbrow_prop['prop_id'],
      'name' => $dbrow_prop['name'],
      'prop_group' => $dbrow_prop['group_name'],
      'type' => $dbrow_prop['type'],
      'created' => $dbrow_prop['created'],
      'changed' => $dbrow_prop['changed'],
    ));
  }

  return $o_rest_response;*/
  }

}