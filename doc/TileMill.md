[List of Modules](List_Of_Modules.md)

# TileMill

Exports vector and raster data as tiles that can be viewed with a webviewer e.g. leaflet using [Mapnik](hhtp://www.mapnik.org)

##Parameter
|        Name       |          Type          |       Description         | 
|-------------------|------------------------|---------------------------|
| ExportMaps   | STRING_MAP |  (map_id,View Name) maps to export (see detailed description)     |
| Styles   | STRING_MAP | (map_id, style file) styles assigned to maps      |
| ZoomLevels   | INT | max zoom level     |
| EPSGCode   | INT | EPSG code of the maps to export |
| fileName   | STRING | Folder in which the tiles are stored |

##Datastream
|     Identifier    |     Attribute    |      Type             |Access |    Description    |
|-------------------|------------------|-----------------------|-------|-------------------|
| User defined map a | 					| NODE; EDGE; FACE; |  read |name of the views exported is defined in ExportMaps |
| User defined map a |                  | COMPONENT   | read  | only needed if year_from_city is true |


#Detailed Description

TileMill uses [Mapnik](hhtp://www.mapnik.org) to render the maps into tiles. A Map can contain several views. 

##Export Maps
The views used are defined with the Parameter ExportMaps. An entry in ExportMaps consists of value pair _"map_id"_ and _"View Name"_.  _"map_id"_ is used in the Styles parameter to assign styles to the view. _"View Name"_ is the name of the view rendered. The order in which the maps are render. Since the ExportMaps is sorted apathetically after _"map_id"_ maps on top are rendered first or last?.

##Styles
TileMill allows to assign different styles to each view. If now style is defined a default style is use (black lines, gray faces).
For user defined style sheets a style file is used. 
