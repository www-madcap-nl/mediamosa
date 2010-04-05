<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
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
  * MediaMosa admin code.
  */

/**
 * Provide a single block from the administration menu as a page.
 * This function is often a destination for these blocks.
 * For example, 'admin/structure/types' needs to have a destination to be valid
 * in the Drupal menu system, but too much information there might be
 * hidden, so we supply the contents of the block.
 *
 * @return
 *   The output HTML.
 */

/**
 * Menu callback; Provide the administration overview page.
 */
function _mediamosa_maintenance_admin_page($path) {
  $blocks = array();

  $admin = db_query("SELECT menu_name, mlid FROM {menu_links} WHERE link_path = :link_path AND module = 'system'", array(':link_path' => 'admin/mediamosa'))->fetchAssoc();

  if ($admin) {
    $result = db_query("
      SELECT m.*, ml.*
      FROM {menu_links} ml
      INNER JOIN {menu_router} m ON ml.router_path = m.path
      WHERE ml.link_path != 'admin/help' AND menu_name = :menu_name AND ml.plid = :mlid AND hidden = -1", $admin, array('fetch' => PDO::FETCH_ASSOC));

    foreach ($result as $item) {
      if ($item['path'] != $path) {
        continue;
      }

      _menu_link_translate($item);

      if (!$item['access']) {
        continue;
      }
      // The link 'description' either derived from the hook_menu 'description'
      // or entered by the user via menu module is saved as the title attribute.
      if (!empty($item['localized_options']['attributes']['title'])) {
        $item['description'] = $item['localized_options']['attributes']['title'];
      }

      $block = $item;
      $block['content'] = '';
      $block['show'] = TRUE;

      if ($item['block_callback'] && function_exists($item['block_callback'])) {
        $function = $item['block_callback'];
        $block['content'] .= $function();
      }

      $block['content'] .= theme('admin_block_content', array('content' => system_admin_menu_block($item)));

      // Prepare for sorting as in function _menu_tree_check_access().
      // The weight is offset so it is always positive, with a uniform 5-digits.
      $blocks[(50000 + $item['weight']) . ' ' . $item['title'] . ' ' . $item['mlid']] = $block;
    }
  }

  if ($blocks) {
    ksort($blocks);
    return theme('admin_page', array('blocks' => $blocks));
  }
  else {
    return t('You do not have any administrative items.');
  }
}

/**
 * Our default themed date.
 *
 * @param array $variables
 */
function theme_mediamosa_maintenance_date($variables) {
  $type = $variables['type'];
  $datetime = $variables['datetime'];

  switch ($type) {
    case 'short':
    case 'medium':
    case 'long':
      break;

    default:
      assert(0);
      break;

  }

  return check_plain(empty($datetime) ? t('<empty date>') : format_date($datetime, $type));
}

/**
 * Our default themed table.
 *
 * @param array $variables
 */
function theme_mediamosa_maintenance_table($variables) {
  $pager_variables = array();
  foreach (array('page', 'item_count_total', 'item_limit', 'element', 'quantity') as $var) {
    if (isset($variables[$var])) {
      $pager_variables[$var] = $variables[$var];
    }
  }

  if (!empty($pager_variables['item_count_total'])) {
    $build['pager_top'] = theme('mediamosa_maintenance_pager', $pager_variables);
  }

  $table_variables = array();
  foreach (array('header', 'rows', 'attributes', 'caption', 'colgroups', 'sticky', 'empty') as $var) {
    if (isset($variables[$var])) {
      $table_variables[$var] = $variables[$var];
    }
  }

  if (!empty($variables['title'])) {
    $build['title'] = '<h1>' . check_plain($variables['title']) . '</h1>';
  }

  if (!empty($variables['description'])) {
    $build['description'] = '<p>' . check_plain($variables['description']) . '</p>';
  }

  $build['table'] = theme('table', $variables);

  if (!empty($variables['note'])) {
    $build['note'] = '<div align="right"><sup><small>' . check_plain($variables['note']) . '</small></sup><div>';
  }

  if (!empty($variables['caption_bottom'])) {
    $build['caption_bottom'] = '<div align="right">' . check_plain($variables['caption_bottom']) . '</div>';
  }

  if (!empty($pager_variables['item_count_total'])) {
    $build['pager_bottom'] = theme('mediamosa_maintenance_pager', $pager_variables);
  }

  return implode('', $build);
}

/**
 * Our pager wrapper.
 *
 * @param array $variables
 */
function theme_mediamosa_maintenance_pager($variables) {
  global $pager_page_array, $pager_total, $pager_total_items, $pager_limits;

  $page = $variables['page'];
  $item_count_total = $variables['item_count_total'];
  $item_limit = $variables['item_limit'];
  $element = $variables['element'];
  $quantity = $variables['quantity'];

  $pager_page_array = array($element => $page);

  // Total items.
  $pager_total_items[$element] = $item_count_total;

  // Number of pagelinks.
  $pager_total[$element] = ceil($item_count_total / $item_limit);

  // Page limit.
  $pager_limits[$element] = $item_limit;

  return theme('pager', array('quantity' => $quantity));
}


/**
 * Create a link to a asset.
 *
 * @param $variables
 */
function theme_l_mediamosa_asset($variables) {
  return mediamosa_lib::l_asset($variables['id'], empty($variables['title']) ? t('<asset has no title>') : $variables['title']);
}

/**
 * Create a link to a collection.
 *
 * @param $variables
 */
function theme_l_mediamosa_collection($variables) {
  return mediamosa_lib::l_collection($variables['id'], empty($variables['title']) ? t('<collection has no title>') : $variables['title']);
}
