--TEST--
MongoDB\Driver\Manager::__construct(): invalid read preference (maxStalenessSeconds range)
--SKIPIF--
<?php if (8 !== PHP_INT_SIZE) { die('skip Only for 64-bit platform'); } ?>
--FILE--
<?php

require_once __DIR__ . '/../utils/tools.php';

echo throws(function() {
    new MongoDB\Driver\Manager(null, ['readPreference' => 'secondary', 'maxStalenessSeconds' => 2147483648]);
}, "MongoDB\Driver\Exception\InvalidArgumentException"), "\n";

?>
===DONE===
<?php exit(0); ?>
--EXPECT--
OK: Got MongoDB\Driver\Exception\InvalidArgumentException
Expected maxStalenessSeconds to be <= 2147483647, 2147483648 given
===DONE===
