--TEST--
Test the logging stats over php error_log
--SKIPIF--
<?php 
if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'e6048d9f1ed7367259270db93b43cf74';

ini_set('rsync.log_stats', 1);

$ret = rsync_generate_signature('tests/002test.png','tests/002signatur.sig');  
unlink('tests/002signatur.sig');
?>
--EXPECTF--
Notice: rsync_generate_signature(): Rsync INFO: signature statistics: signature[3 blocks, 2048 bytes per block]
 in %stests/012.php on line %d
