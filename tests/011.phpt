--TEST--
Test the stats logging over callback php function
--SKIPIF--
<?php 
if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'e6048d9f1ed7367259270db93b43cf74';

ini_set('rsync.log_stats', 1);
function log_callback($level, $message) {
    print "$level $message";
};
rsync_set_log_callback('log_callback');

$ret = rsync_generate_signature('tests/002test.png','tests/002signatur.sig');  
unlink('tests/002signatur.sig');
?>
--EXPECT--
6 signature statistics: signature[3 blocks, 2048 bytes per block]
