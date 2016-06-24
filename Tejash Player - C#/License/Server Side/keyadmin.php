<?php
include
?>
<HTML>
<head>
<title>Tejash Player License Database Management</title>
<style>
p.pic {
    width: 285px;
    margin: 0 auto;
}
</style>
</head>
<body>
<p class="pic"><img src="logo.png"/></p><br><br><center>---------------------------------------------------------------------------</center><br>
<center><b>Upload the License file to store in database:</b><center>
<form action="" method="POST" enctype="multipart/form-data" style="text-align:center">
    <input type="file" name="keyfile" />
    <input type="submit" value="Upload"/>
</form>
<b>Enter the single key to add to database:</b> 
<form action="" method="POST" style="text-align:center">
	Enter Key* : <input type="text" name="single_key" size="50"/><br>
	Enter Name: <input type="text" name="name" size="50"/><br>
	Enter Email: <input type="text" name="email" size="50"/><br>
	<input type="submit" value="Submit"/>
</form>
<b>Find a key:</b>
<form action="" method="POST" style="text-align:center">
	Enter the key: <input type="text" name="find_key" size="50"/>
	<input type="submit" value="Find"/>
</form>
<br><center>&#169; Tejashwi Kalp Taru</center>
<b>-----------------------------------------------------------------------------------------------------------------------------</b><br>
</body>
</HTML>
<?php
// function to delete files
if(isset($_POST["delete_already"]))
{
	$file = "already.txt";
	if(file_exists($file)){
		if(unlink($file)) echo "Deleted file<br>";
		else echo "Unable to delete the file<br>";
	}else echo "File is not there, may be already deleted<br>";
}

// function to find the key in database
if(isset($_POST["find_key"]))
{
	$in = $_POST["find_key"];
	if(!empty($in))
	{
		$username = "tejasbpd_tplayer";
		$password = "";
		$hostname = "localhost";
		$dbname	  = "tejasbpd_tp_keys";

		//connection to the database
		$dbhandle = mysql_connect($hostname, $username, $password)
		 or die("Unable to connect to database<br>");
		echo "Connected to database<br>";

		//select a database to work with
		$selected = mysql_select_db($dbname,$dbhandle)
		  or die("Could not select tejasbpd_tp_keys");
		echo "Selected database to find key<br>";
		
		$found = "";
		$name = "";
		$email = "";
		$date = "";
		$find = mysql_real_escape_string($_POST["find_key"]);
		if(strlen($find)==36){
			$query  = "SELECT * FROM `keys` WHERE serial = '$find' LIMIT 1";
			$result = mysql_query($query);
			if($result)
			{
				while($row = mysql_fetch_assoc($result)) {
					$found = $row['id'];
					$name = $row['name'];
					$email = $row['email'];
					$date = $row['date_reg'];
				}
				if(!empty($found)){
					echo "Found key: $find at ID: $found <br>";
					if(!empty($name) and !empty($email))	echo	"Registered Name: $name, Registered Email: $email <br>";
					if(!empty($date)) echo "This key has been activated on $date <br>";
					else	echo "This serial key is yet not activated<br>";
				}
				else echo "No entry found for the key: $find <br>";
			}
			else echo "Unable to get details from database, please check the script<br>";
		}else echo "Invalid Key! Key must be 36 chars in length<br>";
		//finally close the db connection_aborted
		if(mysql_close($dbhandle))
			echo "Database closed successfully<br>";
		else
			echo "Unable to close database<br>";
	}
	else	echo "Please provide a key to find!<br>";
}
// check for existing key function, used to check if a key is already inserted then dont add
function check_existing_key($find)
{
		/*($username = "tejasbpd_tplayer";
		$password = "";
		$hostname = "localhost";
		$dbname	  = "tejasbpd_tp_keys";

		//connection to the database
		$dbhandle = mysql_connect($hostname, $username, $password);

		//select a database to work with
		$selected = mysql_select_db($dbname,$dbhandle);*/
		$find = mysql_real_escape_string($find);
		$found = "";
		$query  = "SELECT * FROM `keys` WHERE serial = '$find' LIMIT 1";
		$result = mysql_query($query);
		
		if($result)
		{
			while($row = mysql_fetch_assoc($result)) {
				$found = $row['id'];
			}
			if(!empty($found)){
				//mysql_close();
				return true;
			}
			else{
				//mysql_close();
				return false;
			}
		}
}

//function to read the key file, store in db and then delete it
function write_to_db($file)
{
	$username = "tejasbpd_tplayer";
	$password = "";
	$hostname = "localhost";
	$dbname	  = "tejasbpd_tp_keys";

	//connection to the database
	$dbhandle = mysql_connect($hostname, $username, $password)
	 or die("Unable to connect to database<br>");
	echo "Connected to database<br>";

	//select a database to work with
	$selected = mysql_select_db($dbname,$dbhandle)
	  or die("Could not select tejasbpd_tp_keys");
	echo "Selected database to store<br>";
	
	// read the file and store the keys...
	$handle = fopen($file,"r");
	$arry = array();
	if(!handle) echo "Unable to open the key file to read<br>";
	else
	{
		$key_count = 0;
		$already = 0;
		while (!feof($handle)) // Loop till end of file
		{
			$buffer = fgets($handle, 37); // Read a line
			if(strlen($buffer)==36)
			{
				$buffer = mysql_real_escape_string($buffer);
				if(!empty($buffer))
				{
					if(!check_existing_key($buffer)){
						$key_count++;
						$sql = "INSERT INTO `keys` (serial) VALUES('".$buffer."')";
						if(!mysql_query($sql)){
							echo "<br>-----------------------------------------------------<br>";
							echo "Unable to add the value in database!<br>";
							$key_count--;
							echo "Total keys added till now: $key_count <br>";
							echo "Tried to insert the key: $buffer <br>";
							echo mysql_error();
							echo "<br>-----------------------------------------------------<br>";
							goto error;
						}
					}else{
						$arry[$already] = $buffer;
						$already++;
					}
				}
			}
		}
		error:
		if($already>0){
			echo "There are $already keys which are already present in database, hence they are not added<br>";
			file_put_contents('already.txt', print_r($arry, true));
			echo "You can check existing keys at <a href=\"already.txt\" target=\"_blank\">this link</a><br>";
			echo "<form action=\"\" method=\"POST\" style=\"text-align:center\">";
			echo "<input type=\"submit\" name=\"delete_already\" value=\"Delete Existing key file\"/>";
			echo "</form>";
		}
		echo "Total number of keys added in database:  $key_count <br>";
		if(fclose($handle))	echo	"File handle closed, now it will be deleted<br>";
		else	echo	"Unable to close the file handle<br>";
	}
	
	// now all keys are stored in database, so delete the key file which is uploaded
	if(unlink($file))	echo "Key file is deleted now!<br>";
	else	echo "Unable to delete the key file, please delete it manually now for security purpose<br>";

	//finally close the db connection_aborted
	if(mysql_close($dbhandle))
		echo "Database closed successfully<br>";
	else
		echo "Unable to close database<br>";
}

// function to handle single key submission
if(isset($_POST["single_key"])){
	$key = $_POST["single_key"];
	$name = $_POST["name"];
	$email = $_POST["email"];
	if(!empty($key))
	{
		$username = "tejasbpd_tplayer";
		$password = "";
		$hostname = "localhost";
		$dbname	  = "tejasbpd_tp_keys";

		//connection to the database
		$dbhandle = mysql_connect($hostname, $username, $password)
		 or die("Unable to connect to database<br>");
		echo "Connected to database<br>";

		//select a database to work with
		$selected = mysql_select_db($dbname,$dbhandle)
		  or die("Could not select tejasbpd_tp_keys");
		echo "Selected database to store<br>";
		$sql = "";
		$key = mysql_real_escape_string($key);
		if(strlen($key)==36){
			if(!check_existing_key($key)){
				if(!empty($email) and !empty($email)){
					$sql = "INSERT INTO `keys` (serial,name,email) VALUES('".$key."','".$name."','".$email."')";
				}else{
					$sql = "INSERT INTO `keys` (serial) VALUES('".$key."')";
				}
				if(!mysql_query($sql)){
					echo "<br>-----------------------------------------------------<br>";
					echo "Unable to add the value in database!<br>";
					echo "Tried to insert the key: $key <br>";
					echo mysql_error();
					echo "<br>-----------------------------------------------------<br>";
				}
				else{
					echo "Key submitted to database<br>";
					if(!empty($name))	echo "Registered Name: $name <br>";
					if(!empty($email))	echo "Registered Email: $email <br>";
				}
			}else echo "Key already in database, so it's not added<br>";
		} else echo "Invalid key length! Key must be of 36 chars in length<br>";
		
		//finally close the db connection_aborted
		if(mysql_close($dbhandle))
			echo "Database closed successfully<br>";
		else
			echo "Unable to close database<br>";
	}
	else echo "Please provide a key to add to database<br>";
}

// handles the key file submission
if(isset($_FILES['keyfile'])){
    $errors= array();
    $file_name = $_FILES['keyfile']['name'];
    $file_size =$_FILES['keyfile']['size'];
    $file_tmp =$_FILES['keyfile']['tmp_name'];
    $file_type=$_FILES['keyfile']['type'];   
	$upload_loc = $_SERVER["DOCUMENT_URI"];
    $file_ext=strtolower(end(explode('.',$_FILES['keyfile']['name'])));
    $extensions = array("txt"); 		
    if(in_array($file_ext,$extensions )=== false){
     $errors[]="extension not allowed, only text(txt) files are allowed.";
    }
    if($file_size > 2097152){
    $errors[]='File size must not be larger than 2 MB';
    }				
    if(empty($errors)==true){
        if(move_uploaded_file($file_tmp, $upload_loc.$file_name))
		{
			echo "File uploaded successfully, now adding it to database...<br>";
			// read the file contents and add it to the database this function will delete key file too
			write_to_db($upload_loc.$file_name);
		}
		else	echo "Unable to upload the file, please check the script<br>";
    }else{
        for($x=0;$x<count(errors);$x++)
		{
			echo $errors[$x];
			echo "<br>";
		}
		echo "<br><br>";
    }
}
?>
