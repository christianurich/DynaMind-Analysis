[List of Modules](List_Of_Modules.md)

# ExportGeoJSON

Exports vector data as geojson file

##Parameter
|        Name       |          Type          |       Description         | 
|-------------------|------------------------|---------------------------|
| ViewName   | STRING |  Name of the exported view    |
| EPSGCode   | INT | EPSG code of the maps to export |
| FileName   | STRING | Folder in which the data are stored |

##Datastream
|     Identifier    |     Attribute    |      Type             |Access |    Description    |
|-------------------|------------------|-----------------------|-------|-------------------|
| ViewName | 		all			| FACE; |  read | currently only faces are supported |



