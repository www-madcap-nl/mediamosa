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
  *
  */

function play_proxy_cron() {
  play_proxy_ticket_cleanup();
}

function play_proxy_ticket_cleanup() {
  $i_play_expire_timestamp = time() - PLAY_TICKET_EXPIRATION;
  $i_download_expire_timestamp = time() - DOWNLOAD_TICKET_EXPIRATION;
  $i_still_expire_timestamp = time() - STILL_TICKET_EXPIRATION;
  $s_play_ticket_path = SAN_NAS_BASE_PATH ."/". PLAY_TICKET_LOCATION;
  $s_download_ticket_path = SAN_NAS_BASE_PATH ."/". DOWNLOAD_TICKET_LOCATION;
  $s_still_ticket_path = SAN_NAS_BASE_PATH ."/". STILL_TICKET_LOCATION;

  # FIX ME, make it nice: maak een helper functie aan.
  // ruim de play tickets op
  $a_keep_these_tickets = array();

  // first we remove expired tickets
  db_set_active('data');
  $result = db_query("SELECT ticket_id, app_id FROM {ticket} WHERE ticket_type = '%s' AND issued <= FROM_UNIXTIME(%d)", TICKET_TYPE_PLAY, $i_play_expire_timestamp);
  while ($o_ticket = db_fetch_object($result)) {
    $ticket_file = $s_play_ticket_path . '/' . (int) $o_ticket->app_id  . '/' . $o_ticket->ticket_id;
    //$ticket_file = $s_play_ticket_path . '/' . $o_ticket->ticket_id;
    if (@readlink($ticket_file) !== FALSE) {
      @unlink($ticket_file);
      @unlink($ticket_file . ".asx");
      @unlink($ticket_file . ".mp4");
    }
  }
  db_query("DELETE FROM {ticket} WHERE ticket_type = '%s' AND issued <= FROM_UNIXTIME(%d)", TICKET_TYPE_PLAY, $i_play_expire_timestamp);
  db_set_active();

  // then a "recursive" file directory scan to remove missed files.
  db_set_active('data');
  $result = db_query("SELECT ticket_id FROM {ticket} WHERE ticket_type = '%s' AND issued > FROM_UNIXTIME(%d)", TICKET_TYPE_PLAY, $i_play_expire_timestamp);
  db_set_active();

  while ($o_ticket = db_fetch_object($result)) {
    $a_keep_these_tickets[] = $o_ticket->ticket_id;
  }

  $a_tickets = file_scan_directory($s_play_ticket_path, "^", array('.', '..', 'CVS'), 0, FALSE);
  foreach ($a_tickets as $o_ticket) {

    if (is_dir($o_ticket->filename)) {
      $a_sub_tickets = file_scan_directory($o_ticket->filename, "^", array('.', '..', 'CVS'), 0, FALSE);

      foreach ($a_sub_tickets as $o_ticket) {
        if (!in_array($o_ticket->basename, $a_keep_these_tickets) && @readlink($o_ticket->filename) !== FALSE) {
          unlink($o_ticket->filename);
        }
      }
    }
    elseif (!in_array($o_ticket->basename, $a_keep_these_tickets) && @readlink($o_ticket->filename) !== FALSE) {
      unlink($o_ticket->filename);
    }
  }

  // ruim de download tickets op
  $a_keep_these_tickets = array();

  db_set_active('data');
  $result = db_query("SELECT ticket_id FROM {ticket} WHERE ticket_type = '%s' AND issued > FROM_UNIXTIME(%d)", TICKET_TYPE_DOWNLOAD, $i_download_expire_timestamp);
  while ($o_ticket = db_fetch_object($result)) {
    $a_keep_these_tickets[] = $o_ticket->ticket_id;
  }
  db_set_active();

  db_set_active('data');
  db_query("DELETE FROM {ticket} WHERE ticket_type = '%s' AND issued <= FROM_UNIXTIME(%d)", TICKET_TYPE_DOWNLOAD, $i_download_expire_timestamp);
  db_set_active();

  $a_download_tickets = file_scan_directory($s_download_ticket_path, "^", array('.', '..', 'CVS'), 0, TRUE);
  foreach ($a_download_tickets as $o_ticket) {
    $s_ticket = basename(dirname($o_ticket->filename));
    if (!in_array($s_ticket, $a_keep_these_tickets)) {
      if (is_link($o_ticket->filename)) {
        unlink($o_ticket->filename);
        rmdir(dirname($o_ticket->filename));
      }
    }
  }

  // ruim de still tickets op
  exec('find '. $s_still_ticket_path .'/. -maxdepth 1 -mindepth 1 ! -name "*.asx" -name "????????????????????????" -type l -mmin +'. (int)(STILL_TICKET_EXPIRATION / 60) .' -delete');

  db_set_active('data');
  db_query("DELETE FROM {ticket} WHERE ticket_type = '%s' AND issued <= FROM_UNIXTIME(%d)", TICKET_TYPE_STILL, $i_still_expire_timestamp);
  db_set_active();

  exec('find '. $s_still_ticket_path .'/. -maxdepth 1 -mindepth 1 -name "????????????????????????.asx" ! -type l -mmin +'. (int)(STILL_TICKET_EXPIRATION / 60) .' -delete');
}











