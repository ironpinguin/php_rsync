--TEST--
Test function rsync_generate_delta with binary string as input.
--SKIPIF--
<?php if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'b6efe00f2049d221626e63c2a2534361';
$sig = file_get_contents('tests/006signatur.sig');
$file = file_get_contents('tests/006test.png');
$ret = rsync_generate_delta($sig, $file, 'tests/006test.patch');
$resultMD5 = md5_file('tests/006test.patch');
if ($resultMD5 == $targetMD5 && $ret == RSYNC_DONE) {
	echo "SUCCESS\n";
} else {
	echo "FAILED with Return $ret and MD5 $resultMD5\n";
}	
unlink('tests/006test.patch');
?>
--EXPECT--
SUCCESS
