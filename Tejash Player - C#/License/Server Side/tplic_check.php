<?php
// function to find the key in database
if(isset($_GET["find_key"]))
{
	$in = $_GET["find_key"];
	$email = $_GET["email"];
	$name = $_GET["name"];
	date_default_timezone_set('Asia/Kolkata');
	$date = date('Y-m-d H:i:s');
	if(!empty($in))
	{
		$username = "tejasbpd_tplayer";
		$password = "kronos1991";
		$hostname = "localhost";
		$dbname	  = "tejasbpd_tp_keys";

		//connection to the database
		$dbhandle = mysql_connect($hostname, $username, $password);

		//select a database to work with
		$selected = mysql_select_db($dbname,$dbhandle);
		
		$found = "";
		$reg_email = "";
		$reg_name = "";
		$find = mysql_real_escape_string($_GET["find_key"]);
		$find .= "+TKTWMS";
		$query  = "SELECT * FROM `keys` WHERE serial = '$find' LIMIT 1";
		$result = mysql_query($query);
		if($result)
		{
			while($row = mysql_fetch_assoc($result)) {
				$found = $row['id'];
				$reg_email = $row['email'];
				$reg_name = $row['name'];
			}
			if(!empty($found)){
				// key is valid, now check if email and name is registered to then match it, else add to db
				if(!empty($reg_email) and !empty($reg_name)){
					if(strtolower($reg_email) == strtolower($email) and strtolower($reg_name) == strtolower($name)){
						echo "1";
					}else
						echo "2";
				}else{
					// the customer details are not in database, and if provided by user then add it to database
					if(!empty($email) and !empty($name)){
						$query = "UPDATE `keys` SET email='".$email."',name='".$name."',date_reg='".$date."' WHERE id='".$found."'";
						if(mysql_query($query))
							echo "3";
						else{
							echo "4";
						}
					}else echo "0";
				}
			}
			// key is not valid
			else echo "0";
		}
		//finally close the db connection_aborted
		mysql_close($dbhandle);
	}
}
?>