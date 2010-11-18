--TEST--
Test function rsync_patch_file with stream as input and output.
--SKIPIF--
<?php if (!extension_loaded("rsync") || 
          !file_exists("tests/010test.png") || 
          !file_exists("tests/010test.patch")) print "skip"; ?>
--FILE--
<?php 
$targetMD5 = 'e869400efb68a96bafc33bda9a940111';
$file1 = fopen('tests/010test.png', 'rb');
$patch = fopen('tests/010test.patch', 'rb');
$file2 = fopen('tests/010test2.png', 'wb');
$ret = rsync_patch_file($file1, $patch, $file2);
fclose($file1);
fclose($file2);
fclose($patch);
$resultMD5 = md5_file('tests/010test2.png');

if ($resultMD5 == $targetMD5 && $ret == RSYNC_DONE) {
	echo "SUCCESS\n";
} else {
	echo "FAILED with Return $ret and MD5 $resultMD5\n";
}	
unlink('tests/010test2.png');
?>
--EXPECT--
SUCCESS
