<?php if ($show_messages && $messages): ?>
<div id='console'><div class='limiter clearfix'><?php print $messages; ?></div></div>
<?php endif; ?>

<div id='page'><div id='main-content' class='limiter clearfix'>
  <div id='content' class='page-content clearfix'>
    <?php
      unset($page['content']['user_login']);
      unset($page['content']['mediamosa_mediamosa-version']);
    ?>
    <?php if (!empty($page['content'])) print render($page['content']) ?>
  </div>
</div></div>
