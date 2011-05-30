--TEST--
Test function rsync_generate_delta with stream as input and output.
--SKIPIF--
<?php if (!extension_loaded("rsync")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'b6efe00f2049d221626e63c2a2534361';
$sig = fopen('tests/006signatur.sig', 'rb');
$file = fopen('tests/006test.png', 'rb');
$patch = fopen('tests/006test.patch', 'wb');
$ret = rsync_generate_delta($sig, $file, $patch);
fclose($sig);
fclose($file);
fclose($patch);
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
