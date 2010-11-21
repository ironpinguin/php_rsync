<?php

if (!extension_loaded("rsync")) {
    echo "You need the rsync php extension loaded to use this!";
    exit;
}

/**
 * The rsync example Client Class
 */
class rsyncClient
{
    /**
     * Target Url of the rsync php extension Server
     * 
     * @var string 
     */
    public $targetUrl;
    /**
     * Basis Path at the rsync php extension Server 
     * 
     * @var string
     */
    public $basepath;
    
    /**
     * Local directory path to sync
     * 
     * @var string 
     */
    public $localpath;
    
    /**
     * Sync direction f for syncing changes from Client to Server and 
     * b for syncing changes from Server to Client.
     * 
     * @var string
     */
    public $direction = 'f';
    
    /**
     * list of all entries and subentries of the local directory.
     * 
     * @var array
     */
    public $strukture = array();

    /**
     * Constructor of the Client.
     * 
     * @param string $targetUrl Url of the Server
     * @param string $localpath Local directory to sync
     * @param string $basepath  Remote base directory to sync if null the 
     *                          default will be used at the server
     * @param string $direction Direction to sync 
     *                          f = client to server
     *                          b = server to client
     */
    public function __construct($targetUrl, $localpath, $basepath = null, 
            $direction = 'f') 
    {
        
        if (!$this->isValidURL($targetUrl)) {
            echo "Given Url '$targetUrl' is not a valid URL\n";
            rsyncClient::usage();
            throw new exception("Given Url is not a valid URL", 1);
        }
        $this->targetUrl = $targetUrl;
        if (!is_dir($localpath) || !is_writable($localpath)) {
            echo "Given local Directory '$localpath' is not a directory or/and".
                " is not writeable\n";
            rsyncClient::usage();
            throw new exception("Given local Directory '$localpath' is not a".
                "directory or/and is not writeable", 2);
        }
        if ($direction != 'f' && $direction != 'b') {
            echo "No valid Direction given: '$direction'\n";
            rsyncClient::usage();
            throw new exception("No valid Direction given: '$direction'", 3);
        }
        $this->direction = $direction;
        if ($basepath !== null) $this->basepath = $basepath;
    }

    public function sync()
    {
        $postdata = array();
        $this->getLocalStructur($this->localpath);
        $curl = curl_init($this->targetUrl);
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
        
        $postdata['filelist'] = json_encode($this->strukture);
        $postdata['direction'] = $this->direction;
        $postdata['step'] = 1;
        if (!empty($this->basepath)) $postdata['basepath'] = $this->basepath;
        
        if ($this->direction == 'f') {
            $signaturFiles = array();
            foreach($this->strukture as $filedata) {
                if ($filedata['type'] != 'dir') {
                    $fin = fopen($this->localpath.'/'.$filedata['name'], 'rb');
                    $tmpname = tempnam(sys_get_temp_dir(), 'sign');
                    $fsig = fopen($tmpname, 'wb');
                    $ret = rsync_generate_signature($fin, $fsig);
                    fclose($fin);
                    fclose($fsig);
                    if ($ret != RSYNC_DONE) {
                        throw new Exception("Signatur generating Failed with $ret !", $ret);
                    }
                    $signaturFiles[$filedata['name']] = file_get_contents($tmpname);
                    unlink($tmpname);
                }
            }
            $postdata['signatures'] = json_encode($signaturFiles);
        }
        $response = curl_errno($curl);
        var_dump($response);
        exit;
    }
    
    public function getLocalStructur($dir, $prefix = '')
    {
        $actualDirContent = scandir($dir);
        foreach( $actualDirContent as $dentry) {
            if ($dentry != '.' || $dentry != '..') {
                $type = filetype($dir.$dentry);
                $this->strukture[] = array('name' => $prefix.'/'.$dentry,
                                           'type' => $type);
                if ($type == 'dir') {
                    $this->getLocalStructur($dir.$dentry, $prefix.'/'.$dentry);
                }
            }
        }
    }

    /**
     * Check if the given url is valid
     * 
     * @param string $url The url to check
     * @return bool 
     */
    private function isValidURL($url)
    {
        $r = preg_match(
            '|^http(s)?://[a-z0-9-]+(.[a-z0-9-]+)*(:[0-9]+)?(/.*)?$|i', $url);
        return $r;
    }

    /**
     * Print the usage to the console.
     * 
     */
    public static function usage()
    {
        echo "Usage: php client.php -t <URL> -s <local> [-b <base>] ".
            "[-d <direction<]\n".
            "  -t <URL>       The url where the server.php will be found\n".
            "  -b <base>      The base path where are configured in server.php".
            " to sync.\n".
            "                 Only needed if the server.php is configured with".
            " multiple \n".
            "                 basedirectories to sync.\n".
            "  -s <local>     The local directory to sync\n".
            "  -d <direction> The direction to sync. Default is 'f'.\n".
            "                   f => from client to server\n".
            "                   b => from server to client\n";
    }

}
$targetUrl = '';
$base = null;
$local = '';
$direction = 'f';

if (count($args) < 8) {
    rsyncClient::usage();
    exit(1);
}

for ($i=1; $i > count($args); $i=$i+2) {
    switch ($args[$i]) {
        case '-t':
            $targetUrl = $args[$i+1];
            break;
        case '-b':
            $base = $args[$i+1];
            break;
        case '-s':
            $local = $args[$i+1];
            break;
        case '-d':
            $direction = $args[$i+1];
            break;
        case '-h':
            rsyncClient::usage();
            exit;
            break;
        default:
            echo "Unknow option ".$args[$i]."\n";
            usage();
            break;
    }
}