<?php
// $Id$

/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://mediamosa.org)
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
function mediamosa_maintenance_admin_page($path) {
  $blocks = array();
  $admin = db_query("SELECT menu_name, mlid FROM {menu_links} WHERE link_path = 'admin/mediamosa' AND module = 'system'", array())->fetchAssoc();

  if ($admin) {
    $admin[':path'] = $path . '%';
    $result = db_query("
      SELECT m.*, ml.*
      FROM {menu_links} ml
      INNER JOIN {menu_router} m ON ml.router_path = m.path
      WHERE ml.link_path != 'admin/help' AND ml.link_path LIKE :path AND menu_name = :menu_name AND ml.plid = :mlid AND hidden = 0", $admin, array('fetch' => PDO::FETCH_ASSOC));

    foreach ($result as $item) {
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
      $block['show'] = FALSE;
      if ($item['block_callback'] && function_exists($item['block_callback'])) {
        $function = $item['block_callback'];
        $block['content'] .= $function();
      }
      $content = system_admin_menu_block($item);

      if ((isset($item['page_callback']) && !in_array($item['page_callback'], array('mediamosa_maintenance_admin_page'))) || count($content)) {
        // Only show blocks for items which are not containers, or those which
        // are containers and do have items we can show.
        $block['show'] = TRUE;
        if (empty($content)) {
          // If no items found below, but access checks did not fail, show.
          $block['title'] = l($item['title'], $item['href'], $item['localized_options']);
        }
        else {
          // Theme items below.
          $block['content'] .= theme('admin_block_content', $content);
        }
      }
      // Prepare for sorting as in function _menu_tree_check_access().
      // The weight is offset so it is always positive, with a uniform 5-digits.
      $blocks[(50000 + $item['weight']) . ' ' . $item['title'] . ' ' . $item['mlid']] = $block;
    }
  }
  if ($blocks) {
    ksort($blocks);
    return theme('admin_page', $blocks);
  }
  else {
    return t('You do not have any administrative items.');
  }
}

