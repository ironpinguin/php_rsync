# php extension for librsync #
[![Build Status](https://secure.travis-ci.org/ironpinguin/php_rsync.png)](http://travis-ci.org/ironpinguin/php_rsync)

The [PECL/rsync][pecl] extension provides a binding too the librsync c library.
The librsync library is a opensource implementing of the rsync version 1 algoritmus to exchange data differences form on side to the other (over net or local).

The librsync library itself can be found at [librsync] and implements rsync remote-delta algorithm ([Rsync Algoritm][rsync]).

This algorithm is using a rolling check-sum and a md4 check-sum for blocks of the file to generate a signature file. The signature file is used to generate a diff from the file on the remote site. This diff can be used on the local site to patch the file.

The Algorithm is from rsync 1.x and not compatible with rsync 2.x.

The main function is to have file a in one location and file b in a other. With the extension you can get the diffrent between the files and patch on file with the changes from the other.

## Install ##

### Prerequirement ###
You need the [librsync] and php5 devel packet on your system.

### Steps ###
1. Install extension

 1.a. Install with one commando
 
        sudo pecl install rsync
 1.b. Install manualy:

        wget http://pecl.php.net/get/rsync-0.1.0.tgz
        tar xzf rsync-0.1.0.tgz
        cd rsync-0.1.0
        phpize
        ./configure --with-rsync
        make
        sudo make install
2. Add `extension=rsync.so` to your `php.ini`

## Workflow ##

### Prerequirements ###
* __Side/Directory 1__ -> file a.txt
* __Side/Directory 2__ -> file b.txt

### Steps ###
1. __Side 1__
    
    `rsync_generate_signature("a.txt", "a.signature");`
2. Transfer "a.signature" to Side 2
3. __Side 2__

    `rsync_generate_delta("a.signature", "b.txt", "patch_a_with_b.patch");`
4. Transfer "patch_a_with_b.patch" to Side 1
5. __Side 1__

    `rsync_patch_file("a.txt", "patch_a_with_b.patch", "new_a.txt");`
6. replace "a.txt" with "new_a.txt"

[pecl]: http://pecl.php.net/package/rsync "PECL/rsync"
[librsync]: http://librsync.sourceforge.net/ "librsync"
[rsync]: http://rsync.samba.org/tech_report "Rsync Algoritm"
