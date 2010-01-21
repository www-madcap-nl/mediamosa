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
 * Single asset details.
 */
function mediamosa_maintenance_browse_asset_details($asset_id) {
  $response = _mediamosa_restcall('asset/' . $asset_id, 'GET', array('is_app_admin' => 'true'));

  $output = '';

  $asset = (array)$response->xml->items->item;

  // Asset attributes
  $rows = array();
  foreach ($asset as $key => $value) {
    if (in_array($key, array('@attributes', 'czp', 'dublin_core', 'qualified_dublin_core'))) {
      continue;
    }
    if (trim($value) != '') {
      $value = (valid_url($value, TRUE)) ? l($value, $value) : check_plain($value);
      $rows[] = array(_mediamosa_maintenance_browse_clean_string($key), $value);
    }
  }

  // Add the table to the output
  $output .= t('<h3>Asset attributes</h3>');
  $output .= theme('table', array(), $rows);

  // Metadata
  foreach (array(
    'czp' => t('<h3>CZP Metadata</h3>'),
    'dublin_core' => t('<h3>Dublin Core Metadata</h3>'),
    'qualified_dublin_core' => t('<h3>Qualified Dublin Core Metadata</h3>'),
  ) as $subject => $title) {
    // Construct the table rows
    $rows = array();
    foreach ($asset[$subject] as $key => $value) {
      if (trim($value) != '') {
        $value = (valid_url($value, TRUE)) ? l($value, $value) : check_plain($value);
        $rows[] = array(_mediamosa_maintenance_browse_clean_string($key), $value);
      }
    }

    // Add the table to the output
    $output .= $title;
    $output .= theme('table', array(), $rows);
  }

  // Mediafiles
  foreach ($response->xml->items->item->xpath('mediafiles/mediafile') as $mediafile) {
    // Construct the table rows
    $rows = array();
    foreach ((array)$mediafile as $key => $value) {
      if (in_array($key, array('@attributes'))) {
        continue;
      }
      if (trim($value) != '') {
        $value = (valid_url($value, TRUE)) ? l($value, $value) : check_plain($value);
        $rows[] = array(_mediamosa_maintenance_browse_clean_string($key), $value);
      }
    }

    // Add the table to the output
    $output .= t('<h3>Mediafile: @mediafile_id</h3>', array('@mediafile_id' => $mediafile->mediafile_id));
    $output .= theme('table', array(), $rows);
  }

  return $output;
}

function _mediamosa_maintenance_browse_asset_get_all_collections($app_id) {
  $app_ids = explode(' ', $app_id);

  $response = _mediamosa_restcall('collection', 'GET', array('app_id' => $app_ids));

  $collections = array();
  foreach ($response->xml->xpath('//items/item') as $collection) {
    $title = trim((string)$collection->title);
    $collections[(string)$collection->coll_id] = ($title != '') ? $title : t('<title not set>');
  }

  return $collections;
}

/**
 * Helper function for the asset filter form
 */
function _mediamosa_maintenance_browse_asset_list_filter() {
  $search_terms = array(
    'asset_info' => array(
      'asset_id' => t('Asset id'),
      'owner_id' => t('Owner id'),
      'group_id' => t('Group id'),
      'reference_id' => t('Reference id'),
      'batch_id' => t('Batch id'),
      'app_id' => t('App id'),
      //'provider_id' => t('provider_id'),
      'videotimestamp' => t('Video timestamp'),
      'videotimestampmodified' => t('Video timestamp modified'),
      //'play_restriction_start' => t('play_restriction_start'),
      //'play_restriction_end' => t('play_restriction_end'),
    ),
    'dublin_core' => array(
      'type' => t('Type'),
      'format' => t('Format'),
      'language' => t('Language'),
      'title' => t('Title'),
      'creator' => t('Creator'),
      'publisher' => t('Publisher'),
      'subject' => t('Subject'),
      'description' => t('Description'),
      'contributor' => t('Contributor'),
      'date' => t('Date'),
      'identifier' => t('Identifier'),
      'source' => t('Source'),
      'relation' => t('Relation'),
      'coverage_temporal' => t('Coverage temporal'),
      'coverage_spatial' => t('Coverage spatial'),
      'rights' => t('Rights'),
    ),
    'qualified_dublin_core' => array(
      'title_alternative' => t('Title alternative'),
      'description_abstract' => t('Description abstract'),
      'created' => t('Created'),
      'issued' => t('Issued'),
      'hasformat' => t('Has format'),
      'isformatof' => t('Is format of'),
      'format_medium' => t('Format medium'),
      'format_extent' => t('Format extent'),
      'license' => t('License'),
      'rightsholder' => t('Rightsholder'),
      'isreferencedby' => t('Is referenced by'),
    ),
  );
  foreach ($search_terms as $key => $value) {
    $search_terms[$key] = $value;
    asort($search_terms[$key]);
  }

  $search = array(
    0 => '', // Empty row
    'Asset information' => $search_terms['asset_info'],
    1 => '', // Empty row
    'Dublin Core' => $search_terms['dublin_core'],
    2 => '', // Empty row
    'Qualified Dublin Core' => $search_terms['qualified_dublin_core'],
  );

  return $search;
}

function _mediamosa_maintenance_browse_asset_get_cql_from_session() {
  $filters = $data = array();

  // Iterate all items stored in the form submit function
  foreach ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION] as $key => $filter) {
    switch ($key) {
      case 'app_id':
        $filters['app_id'] = explode(' ', $filter);
        break;

      case 'coll_id':
        $filters['coll_id'][] = $filter;
        break;

      case 'filters':
        foreach ($filter as $filter_item) {
          $filter_key = reset(array_keys($filter_item));
          $filter_value = reset(array_values($filter_item));

          // Avoid duplicates
          if ((isset($filters[$filter_key]) && !in_array($filter_value, $filters[$filter_key])) || !isset($filters[$filter_key])) {
            $filters[$filter_key][] = addslashes($filter_value);
          }
        }
        break;

    }
  }

  // App id's must be passed through separate GET parameters
  if (isset($filters['app_id'])) {
    $data['app_ids'] = $filters['app_id'];
    unset($filters['app_id']);
  }

  // Construct the cql
  $cql = array();
  foreach ($filters as $filter_group => $filter) {
    $cql[] = sprintf('%s == "%s"', $filter_group, implode(' ', $filter));
  }
  $data['cql'] = '('. implode(" AND ", $cql) .')';

  // Add the optional sort order
  if ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['tablesort']) {
    $sortby = $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['tablesort']['sql'];
    $sortorder = ($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['tablesort']['sort'] == 'desc') ? 'descending' : 'ascending';
    $data['cql'] .= 'sortBy '. $sortby .'/'. $sortorder;
  }

  return $data;
}

/**
 * The browse form which also contains a list of browsable assets.
 */
function mediamosa_maintenance_browse_asset_form() {
  if (isset($_GET['coll_id'])) {
    $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION] = array('coll_id' => check_plain($_GET['coll_id']));
  }

  // Gather and prepare some variables.
  $form = array();
  $item_limit = 50;
  $page = isset($_GET['page']) ? (int)$_GET['page'] : 0;
  $offset = $page * $item_limit;

  // Gather all apps.
  $all_apps_raw = array();
  foreach (mediamosa_app::get_all_apps(array(mediamosa_app_db::APP_ID, mediamosa_app_db::NAME), mediamosa_app_db::NAME, 'ASC') as $app) {
    $all_apps_raw[$app[mediamosa_app_db::APP_ID]] = $app[mediamosa_app_db::NAME];
  }
  $all_apps = array(implode(' ', array_keys($all_apps_raw)) => t('<All applications>'));
  $all_apps += $all_apps_raw;

  $app_id = (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['app_id'])) ? $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['app_id'] : reset(array_keys($all_apps));
  $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_COLLECTIONS_SESSION]['app_id'] = $app_id;
  $form['app_id'] = array(
    '#type' => 'select',
    '#options' => $all_apps,
    '#title' => t('Application'),
    '#default_value' => $app_id,
  );

  $all_collections_raw = _mediamosa_maintenance_browse_asset_get_all_collections($app_id);

  if (count($all_collections_raw)) {
    $all_collections = array_merge(array(0 => ''), $all_collections_raw);
  }
  else {
    $all_collections = array(0 => t('No collections found...'));
  }

  $coll_id = (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['coll_id'])) ? $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['coll_id'] : 0;
  $form['coll_id'] = array(
    '#type' => 'select',
    '#options' => $all_collections,
    '#title' => t('Collection'),
    '#default_value' => $coll_id,
  );

  // Get the current filter count
  $filter_count = (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['filters'])) ? count($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['filters']) : 0;

  // Add one empty filter item
  $filter_count++;

  // Get the maximum number of filters
  $filter_max = variable_get('mediamosa_maintenance_browse_filter_max', 25);

  // Load and set all javascript related items
  drupal_add_js(drupal_get_path('module', 'mediamosa_maintenance_browse') . '/mediamosa_maintenancebrowse.js');
  drupal_add_js(
    array(
      'mediamosa_maintenance_browse' => array(
        'filter_count' => $filter_count,
        'filter_max' => $filter_max,
      ),
    ),
    'setting'
  );

  $filter_types = _mediamosa_maintenance_browse_asset_list_filter();
  for ($i = 1; $i <= $filter_max; $i++) {
    $display = ($i <= $filter_count) ? '' : 'none';
    $extra_class = '';
    if ($i == 1) {
      $extra_class .= ' first';
    }
    elseif ($i == $filter_max) {
      $extra_class .= ' last';
    }

    $form['filter_type_'. $i] = array(
      '#prefix' => '<div id="mediamosa-browse-asset-filter-'. $i .'" style="display: '. $display .'" class="mediamosa-browse-asset-filter'. $extra_class .'">',
      '#type' => 'select',
      '#options' => $filter_types,
      '#title' => t('Filter item @count', array('@count' => $i)),
    );

    $form['filter_value_'. $i] = array(
      '#type' => 'textfield',
      '#title' => '',
      '#suffix' => '</div>',
    );

    // Restore submitted values
    if (isset($_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['filters'][($i - 1)])) {
      $info = $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['filters'][($i - 1)];
      $form['filter_type_'. $i]['#default_value'] = reset(array_keys($info));
      $form['filter_value_'. $i]['#default_value'] = reset(array_values($info));
    }
  }

  // What's a form without a submit button?
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Apply filter'),
  );

  $form['add_filter_row'] = array(
    '#type' => 'submit',
    '#value' => t('Add filter item'),
    '#suffix' => '<a href="#" id="edit-clear">Clear form</a>',
  );

  // Only show results on GET requests to prevent Drupal from fetching the results on form validation.
  if ($_SERVER['REQUEST_METHOD'] != 'POST') {

    $header = array(
      array('field' => 'title', 'data' => t('Title')),
      array('field' => 'app_id', 'data' => t('App ID')),
      array('field' => 'videotimestampmodified', 'data' => t('Last modified')),
    );
    $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['tablesort'] = (isset($_GET['order'])) ? tablesort_init($header) : FALSE;

    // Fetch the assets
    $cql_data = _mediamosa_maintenance_browse_asset_get_cql_from_session();
    $data = array(
      'limit' => $item_limit,
      'offset' => $offset,
      'cql' => $cql_data['cql'],
    );

    // Add app_id's
    if (isset($cql_data['app_ids'])) {
      $data['app_id'] = $cql_data['app_ids'];
    }

    $response = _mediamosa_restcall('asset', 'GET', $data);

    // Extract the item count.
    $item_count = (int)$response->xml->header->item_count;
    $item_count_total = (int)$response->xml->header->item_count_total;

    // Return the form if no assets were found.
    if ($item_count_total == 0) {
      $form['results'] = array(
        '#markup' => '<p>'. t('No assets found...') .'</p>',
      );
      return $form;
    }

    // Correct the item limit if the count is less than it.
    if ($item_count_total < $item_limit) {
      $item_limit = $item_count_total;
    }
    drupal_set_title(t('Showing assets @from-@to of @total', array(
      '@from' => $offset + 1,
      '@to' => $offset + $item_count,
      '@total' => $item_count_total,
    )));

    $rows = array();
    foreach ($response->xml->xpath('//items/item') as $asset) {
      $title = (isset($asset->dublin_core->title) && trim((string)$asset->dublin_core->title) != '') ? $asset->dublin_core->title : t('<title not set>');
      $rows[] = array(
        l($title, 'admin/mediamosa/browse/asset/' . $asset->asset_id),
        $asset->app_id,
        $asset->videotimestampmodified,
      );
    }

    $form['pager_top'] = array(
      '#markup' => _mediamosa_maintenance_browse_pager($page, $item_count_total, $item_limit),
    );

    $form['results'] = array(
      '#markup' => theme('table', $header, $rows),
    );

    $form['pager_bottom'] = array(
      '#markup' => _mediamosa_maintenance_browse_pager($page, $item_count_total, $item_limit),
    );
  }

  return $form;
}

/**
 * The browse filter form submit function.
 */
function mediamosa_maintenance_browse_asset_form_submit($form, $form_state) {
  $filters = array();
  foreach ($form_state['values'] as $key => $value) {
    if (strpos($key, 'filter_type_') === 0 && !is_numeric($value)) {
      $id = str_replace('filter_type_', '', $key);
      if (trim($form_state['values']['filter_value_'. $id]) != '') {
        $filters[] = array($value => trim($form_state['values']['filter_value_'. $id]));
      }
    }
  }

  $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION] = array(
    'filters' => $filters,
    'app_id' => $form_state['values']['app_id'],
  );

  if ($form_state['values']['coll_id'] != 0) {
    $_SESSION[MEDIAMOSA_MAINTENACE_BROWSE_ASSETS_SESSION]['coll_id'] = $form_state['values']['coll_id'];
  }
}
