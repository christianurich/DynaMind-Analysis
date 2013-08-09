DynaMind-Analysis
=================



<!DOCTYPE html>
<html>
<head>
  <title>DynaMind Results</title>
	<meta charset="utf-8" />

	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<link rel="stylesheet" href="http://cdn.leafletjs.com/leaflet-0.6.4/leaflet.css" />
	<script src="http://cdn.leafletjs.com/leaflet-0.6.4/leaflet.js"></script>
  	<script src="http://code.jquery.com/jquery-1.9.1.js"></script>
</head>

<body>
	<form id="states">
	  <label for="state">States</label>
	  <select name="state" id="state">
	  	<option>0</option>
	    <option>1</option>
	    <option>2</option>
	    <option>3</option>
	    <option>4</option>
	    <option>5</option>
	    <option>6</option>
	  	<option>7</option>
	    <option>8</option>
	    <option>9</option>
	    <option>10</option>
	    <option>11</option>
	    <option>12</option>
	    <option>13</option>
	    <option>14</option>
	    <option>15</option>
	    <option>16</option>
	    <option>17</option>
	    <option>18</option>
	    <option>19</option>
	  </select>
	</form>

	<div id="map" style="width: 800px; height: 400px"></div>

	<script>
		var map = L.map('map').setView([51.505, -0.09], 2);
		var layers = L.tileLayer('/tmp/test/0/{z}/{x}/{y}.png', {
			maxZoom: 3,
			attribution: 'Map data &copy; <a href="http://openstreetmap.org">OpenStreetMap</a> contributors, <a href="http://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, Imagery © <a href="http://dynamind-toolbox.org">DynaMind-Toolbox</a>'
		})
		layers.addTo(map);
		//Conect Slider
		 $(function() {
		    var select = $( "#state" );
		    $( "#state" ).change(function() {

		       var state_location =  '/tmp/test/' + this.selectedIndex + '/' + '{z}/{x}/{y}.png'
		       console.log(state_location);		       
		       layers.setUrl(state_location)
		    });
	  	});
	</script>
</body>
</html>
