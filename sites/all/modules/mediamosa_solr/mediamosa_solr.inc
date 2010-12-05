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
 * All non-hook functions are here.
 */

/**
 * Called when pressed on test button.
 *
 * @param array $form_state
 * @param array $form
 */
function _mediamosa_solr_ping_submit(&$form_state, $form) {
  $mediamosa_solr_url = $form['values']['mediamosa_solr_url'];
  $check_connection_save = $form['values']['check_connection_save'];

  if (MediaMosa_Apache_Solr_Service::mediamosaPing($mediamosa_solr_url)) {
    if ($check_connection_save) {
      drupal_set_message(t('Connection test successful, URL saved.'));

      MediaMosa_Apache_Solr_Service::mediamosaSetUrl($mediamosa_solr_url);
    }
    else {
      drupal_set_message(t('Connection test successful. Warning: URL was not saved.'));
    }
  }
  else {
    drupal_set_message(t('Unable to connect to server, check URL.'), 'error');
  }
}
