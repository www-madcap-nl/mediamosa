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
 * Installer for mediamosa_server_streaming module.
 */

require_once('container/mediamosa_server_streaming_container.install.inc');
require_once('mediamosa_server_streaming_db.inc');
/**
 * Implements hook_install().
 */
function mediamosa_server_streaming_install() {

  // Download server.
  drupal_install_schema(mediamosa_server_streaming_db::TABLE_NAME);

  // Our child.
  mediamosa_server_streaming_container_install();

  $a_data = array(
    array(1, 0, "windows media (avi)", "rtsp://wms.example.org/tickets/{TICKET}", 1, "avi", "{IF_EXTERNAL}\r\n<asx version=\"3.0\">\r\n<entry>\r\n<ref href=\"{TICKET_URI}\" />\r\n{IF_START}\r\n<starttime value=\"{START_TIME}\" />\r\n{/IF_START}\r\n{IF_DURATION}\r\n<duration value=\"{DURATION_TIME}\" />\r\n{/IF_DURATION}\r\n</entry>\r\n</asx>\r\n{/IF_EXTERNAL}\r\n<object id='{MEDIAFILE_ID}' classid='CLSID:22D6F312-B0F6-11D0-94AB-0080C74C7E95' standby='Loading Media Player components...' type='application/x-oleobject' width='{WIDTH}' height='{HEIGHT}'>\r\n<param name='filename' value='{WMA_TICKET_URI}' />\r\n<param name='autostart' value='{AUTOSTART}' />\r\n<embed type='application/x-mplayer2' src='{WMA_TICKET_URI}' autostart='{AUTOPLAY_NUM}' name='{MEDIAFILE_ID}' width='{WIDTH}' height='{HEIGHT}'/>\r\n</embed>\r\n</object>"),
    array(2, 0, "flash server (flv)", "http://flash.streaming.example.org/StroboScopeLoader.php?movieName={TICKET}", 1, "flv|mp3", "<script type='text/javascript' src='http://example.org/swfobject.js'></script>\r\n<div id='mediaspace'>This div will be replaced</div>\r\n<script type='text/javascript'>\r\nvar s1 = new SWFObject('http://example.org/player.swf','ply','{WIDTH}','{HEIGHT}','9');\r\ns1.addParam('allowfullscreen','true');\r\ns1.addParam('allowscriptaccess','always');\r\ns1.addParam('wmode','opaque');\r\ns1.addParam('flashvars','streamer={TICKET_URI}&stretching=exactfit&autostart={AUTOPLAY}');\r\ns1.write('mediaspace');\r\n</script>"),
    array(3, 0, "mpeg / QT server (mp4)", "rtsp://mpeg4.streaming.example.org/vpx-test/{TICKET}", 1, "mov;mp4;m4a;3gp;3g2;mj2", "<script type='text/javascript' src='http://example.org/swfobject.js'></script>\r\n<div id='mediaspace'>This div will be replaced</div>\r\n<script type='text/javascript'>\r\nvar s1 = new SWFObject('http://example.org/player.swf','ply','{WIDTH}','{HEIGHT}','9');\r\ns1.addParam('allowfullscreen','true');\r\ns1.addParam('allowscriptaccess','always');\r\ns1.addParam('wmode','opaque');\r\ns1.addParam('flashvars','streamer={TICKET_URI}&autostart={AUTOPLAY}');\r\ns1.addVariable('type', 'mp4');\r\ns1.write('mediaspace');\r\n</script>\r\n"),
    array(5, 0, "wowza streaming server (mp4)", "rtmp://example.org:554/simplevideostreaming/&file=vpx-acc/nibg_test1.mp4&f={TICKET}", 1, "mov;mp4;m4a;3gp;3g2;mj2", "<object id='{MEDIAFILE_ID}' classid='CLSID:02Bf25D5-8C17-4B23-BC80-D3488ABDDC6B' standby='Loading Quicktime Player components...' width='{WIDTH}' height='{HEIGHT}'>\r\n{IF_START}\r\n<param name='starttime' value=\"{START_TIME}\" />\r\n{/IF_START}\r\n{IF_DURATION}\r\n<param name='endtime' value=\"{END_TIME}\" />\r\n{/IF_DURATION}\r\n<param name='src' value='{TICKET_URI}' />\r\n<param name='autoplay' value='{AUTOPLAY}' />\r\n<embed type='video/quicktime' src='{TICKET_URI}' name='{MEDIAFILE_ID}' width='{WIDTH}' height='{HEIGHT}' autoplay='{AUTOPLAY}'/>\r\n</object>\r\n"),
    array(6, 0, "Test flv video", "rtmp://example.org:554/simplevideostreaming/&file=vpx-acc/test_flv.flv&f={TICKET}", 1, "flv", "<object id='{MEDIAFILE_ID}' classid='CLSID:D27CDB6E-AE6D-11CF-96B8-444553540000'\r\nstandby='Loading Flash Player components...' codebase='http://fpdownload.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=7,0,0,0' width='{WIDTH}' height='{HEIGHT}'>\r\n<param name='movie' value='{TICKET_URI}&movieWidth={WIDTH}&movieHeight={HEIGHT}&autoStart={AUTOPLAY}' />\r\n<param name='quality' value='high' />\r\n<param name='settings' value='hide' />\r\n<embed type='application/x-shockwave-flash'\r\npluginspace='http://www.macromedia.com/go/getflashplayer'\r\nsrc='{TICKET_URI}&movieWidth={WIDTH}&movieHeight={HEIGHT}&autoStart={AUTOPLAY}'\r\nname='{MEDIAFILE_ID}'\r\nquality='high'\r\nsettings='hide'\r\nwidth='{WIDTH}'\r\nheight='{HEIGHT}' />\r\n</object>\r\n"),
    array(7, 0, "Test wowza (mp3)", "rtmp://example.org:554/simplevideostreaming/&file=vpx-acc/sound.mp3&f={TICKET}", 1, "mp3", "<script type='text/javascript' src='http://example.org/swfobject.js'></script>\r\n<div id='mediaspace'>This div will be replaced</div>\r\n<script type='text/javascript'>\r\nvar s1 = new SWFObject('http://example.org/player.swf','ply','200','20','9');\r\ns1.addParam('allowfullscreen','true');\r\ns1.addParam('allowscriptaccess','always');\r\ns1.addParam('wmode','opaque');\r\ns1.addParam('flashvars','type=rtmp&streamer={TICKET_URI}&autostart={AUTOPLAY}');\r\ns1.write('mediaspace');\r\n</script>"),
    array(8, 0, "flash server (MP3)", "http://flash.streaming.example.org/StroboScopeLoader.php?movieName={TICKET}", 1, "mp3", "<script type='text/javascript' src='http://example.org/swfobject.js'></script>\r\n<div id='mediaspace'>This div will be replaced</div>\r\n<script type='text/javascript'>\r\nvar s1 = new SWFObject('http://example.org/player.swf','ply','{WIDTH}','{HEIGHT}','9');\r\ns1.addParam('allowfullscreen','true');\r\ns1.addParam('allowscriptaccess','always');\r\ns1.addParam('wmode','opaque');\r\ns1.addParam('flashvars','streamer={TICKET_URI}&autostart={AUTOPLAY}');\r\ns1.addVariable('type', 'mp4');\r\ns1.write('mediaspace');\r\n</script>\r\n"),
    array(9, 0, "windows media (asf)", "rtsp://wms.acceptatie.streaming.example.org/vpx-test/{TICKET}", 1, "asf|asx", "{IF_EXTERNAL}\r\n<asx version=\"3.0\">\r\n<entry>\r\n<ref href=\"{TICKET_URI}\" />\r\n<starttime value=\"{START_TIME}\" />\r\n<duration value=\"{DURATION_TIME}\" />\r\n</entry>\r\n</asx>\r\n{/IF_EXTERNAL}\r\n<object id='{MEDIAFILE_ID}' classid='CLSID:22D6F312-B0F6-11D0-94AB-0080C74C7E95' standby='Loading Media Player components...' type='application/x-oleobject' width='{WIDTH}' height='{HEIGHT}'>\r\n<param name='filename' value='{WMA_TICKET_URI}' />\r\n<param name='autostart' value='{AUTOPLAY}' />\r\n<embed type='application/x-mplayer2' src='{WMA_TICKET_URI}' autostart='{AUTOPLAY_NUM}' name='{MEDIAFILE_ID}' width='{WIDTH}' height='{HEIGHT}'/>\r\n</embed>\r\n</object>"),
    array(10, 0, "mpeg / QT server (flv)", "rtsp://mpeg4.streaming.example.org/vpx-test/{TICKET}", 1, "flv", "<script type='text/javascript' src='http://example.org/swfobject.js'></script>\r\n<div id='mediaspace'>This div will be replaced</div>\r\n<script type='text/javascript'>\r\nvar s1 = new SWFObject('http://example.org/player.swf','ply','{WIDTH}','{HEIGHT}','9');\r\ns1.addParam('allowfullscreen','true');\r\ns1.addParam('allowscriptaccess','always');\r\ns1.addParam('wmode','opaque');\r\ns1.addParam('flashvars','streamer={TICKET_URI}&autostart={AUTOPLAY}');\r\ns1.addVariable('type', 'mp4');\r\ns1.write('mediaspace');\r\n</script>\r\n"),
  );

  $query = "INSERT INTO {#mediamosa_server_streaming} (#streaming_server_id, #version, #description, #uri, #active, #containers, #objectcode) VALUES \n";
  $a_args = array(
    '#mediamosa_server_streaming' => mediamosa_server_streaming_db::TABLE_NAME,
    '#streaming_server_id' => mediamosa_server_streaming_db::ID,
    '#version' => mediamosa_server_streaming_db::VERSION,
    '#description' => mediamosa_server_streaming_db::DESCRIPTION,
    '#uri' => mediamosa_server_streaming_db::URI,
    '#active' => mediamosa_server_streaming_db::ACTIVE,
    '#containers' => mediamosa_server_streaming_db::CONTAINERS,
    '#objectcode' => mediamosa_server_streaming_db::OBJECTCODE,
  );

  // Fill in ours.
  $query = strtr($query, $a_args);
  $a_args = array();

  $x = 0;
  $query_rows = array();
  foreach ($a_data as $a_row) {
    $a_values = array();

    foreach ($a_row as $value) {
      $tag = ':column_' . $x++;
      $a_args[$tag] = $value;
      $a_values[] = $tag;
    }

    $query_rows[] = '(' . implode(', ', $a_values) . ')';
  }

  $query .= implode(",\n", $query_rows);

  // Insert default.
  db_query($query, $a_args);
}

/**
 * Implements hook_uninstall().
 */
function mediamosa_server_streaming_uninstall() {

  // Our child.
  mediamosa_server_streaming_container_uninstall();

  // Download server.
  drupal_uninstall_schema(mediamosa_server_streaming_db::TABLE_NAME);
}

/**
 * Implements hook_schema().
 */
function mediamosa_server_streaming_schema() {

  $a_schema[mediamosa_server_streaming_db::TABLE_NAME] = array(
    'description' => t('The streaming server table.'),
    'fields' => array(
      mediamosa_server_streaming_db::ID => array(
        'description' => t('Primary key.'),
        'type' => 'serial',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_db::VERSION => array(
        'description' => t('Version of the server.'),
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_db::DESCRIPTION => array(
        'description' => t('Description of the server.'),
        'type' => 'varchar',
        'length' => '255',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_db::URI => array(
        'description' => t('The URI of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_streaming_db::URI_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_db::ACTIVE => array(
        'description' => t('Active setting.'),
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_db::CONTAINERS => array(
        'description' => t('Containers of the server.'),
        'type' => 'varchar',
        'length' => mediamosa_server_streaming_db::CONTAINERS_LENGTH,
        'not null' => TRUE,
      ),
      mediamosa_server_streaming_db::OBJECTCODE => array(
        'description' => t('The objectcode to use in HTML.'),
        'type' => 'text',
        'not null' => FALSE,
      ),
    ),
    'primary key' => array(mediamosa_server_streaming_db::ID),
  );

  return $a_schema;
}
