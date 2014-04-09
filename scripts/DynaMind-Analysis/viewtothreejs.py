# -*- coding: utf-8 -*-
"""
/***************************************************************************
 DAnCEKit

                              A web interface to bring DynaMind online
                              -------------------
        begin                : 2014-04-01
        copyright            : (C) 2014 Christian Urich
        email                : christian.urich@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

import pydmtoolbox as DMtool
import pydmextensions as DMext
import pydynamind as DM



class ViewToThreeJS(DM.Module):
    _outputView = DM.View()


    def __init__(self):
        DM.Module.__init__(self)
        self.createParameter("Filename", DM.STRING, "Filename")
        self.Filename = ""

        self.createParameter("ViewName", DM.STRING, "ViewName")
        self.ViewName = ""

        self.createParameter("ViewType", DM.STRING, "FACE")
        self.ViewType = ""

        self.createParameter("center_x", DM.DOUBLE, "center_x")
        self.center_x = 0

        self.createParameter("center_y", DM.DOUBLE, "center_y")
        self.center_y = 0

        self.createParameter("width", DM.DOUBLE, "width")
        self.length = 0

        self.createParameter("height", DM.DOUBLE, "height")
        self.height = 0

        datastream = []
        datastream.append(DM.View("dummy", DM.SUBSYSTEM, DM.READ))
        self.addData("Data", datastream)

    def addCubeToView(self, system=DM.System()):
        n1 = system.addNode(0, 0, 0)
        n2 = system.addNode(0, 1, 0)
        n3 = system.addNode(1, 1, 0)
        n4 = system.addNode(1, 0, 0)

        face = DM.nodevector()
        face.append(n1)
        face.append(n4)
        face.append(n3)
        face.append(n2)

        f = system.addFace(face, self._buildings)

        DMtool.TBVectorData_ExtrudeFace(system, self._buildings, face, 1., 0, True)

    def init(self):
        if self.ViewName == "":
            return

        # self._outputView = DM.View(self.ViewName, DM.FACE, DM.READ)
        # datastream = []
        # datastream.append(self._outputView)
        # self.addData("Data", datastream)

    def run(self):
        if self.ViewName == "":
            print "No view defined"
            return
        self._outputView = DM.View(self.ViewName, DM.COMPONENT, DM.READ)
        system = self.getData("Data")
        #self.addCubeToView(system)

        #Get Export Elements
        uuids = system.getUUIDs(self._outputView)

        objects = []
        for uuid in uuids:
            if self.ViewType == "COMPONENT":
                building = system.getComponent(uuid)
                LinkAttributes = building.getAttribute("Geometry").getLinks()
                for attribute in LinkAttributes:
                    objects.append(attribute.uuid)
            if self.ViewType == "FACE":
                objects.append(uuid)


        if len(objects) == 0:
            print "No " + str(self.ViewName) + " found"
            return

        #Create File
        output = open(self.Filename, "w")

        #Write Header
        self.writeHeader(output)

        #Create Vector
        self.viewToThreeJS(output, objects, system)

        output.close()

    def writeHeader(self, output):
        output.write("{\n")
        output.write("\"metadata\" :\n")
        output.write("{\n")
        output.write("\"formatVersion\" : 3,\n")
        output.write("\"generatedBy\"   : \"Blender 2.63 Exporter\",\n")
        output.write("\"vertices\"      : 3,\n")
        output.write("\"faces\"         : 1,\n")
        output.write("\"normals\"       : 0,\n")
        output.write("\"colors\"        : 1,\n")
        output.write("\"uvs\"           : 0,\n")
        output.write("\"materials\"     : 1,\n")
        output.write("\"morphTargets\"  : 0\n")
        output.write("},\n")

        output.write("\"scale\" : 1.000000,\n")


    def viewToThreeJS(self, output, objects, sys):
        vert_floats = []
        normal_floats = []
        colors = []

        color_index = []
        x = []
        y = []
        z = []
        print "start"
        for obj in objects:
            f = sys.getFace(obj)
            rgbVector = DM.doublevector()
            rgbVector = f.getAttribute("color").getDoubleVector()
            if len(rgbVector) != 3:
              rgbVector.append(0)
              rgbVector.append(1)
              rgbVector.append(0)
            color = str(rgbVector[0]) + ',' + str(rgbVector[1]) + ',' + str(rgbVector[2])
            try:
                color_index.index(color)
            except ValueError:
                color_index.append(color)

            triangles = DMext.CGALGeometry_FaceTriangulation(sys, f)
            for n in triangles:
                x.append(n.getX())
                y.append(n.getY())
                z.append(n.getZ())

                vert_floats.append(n.getX())
                vert_floats.append(n.getY())
                vert_floats.append(n.getZ())
                normal_floats.append(0)
                normal_floats.append(1)
                normal_floats.append(0)
            for i in range(len(triangles)/3):
                colors.append(color_index.index(color))

        print "done"
        x_min = min(x)
        y_min = min(y)

        x_max = max(x)
        y_max = max(y)

        self.center_x = (x_max - x_min) / 2. + x_min
        self.center_y = (y_max - y_min) / 2. + y_min

        self.height = x_max - x_min
        self.width = y_max - y_min

        vertices = ""
        counter = 0

        stop = len(vert_floats)

        for v in vert_floats:
            vertices += str(v)
            counter += 1
            if counter != stop:
                vertices += str(",")

        faces = ""
        counter = 0

        for i in range(len(vert_floats)/9):
            faces += str("2")
            faces += str(",")
            for j in range(3):
                faces += str(counter)
                counter += 1
                faces += str(",")

            #Write Color
            faces += str(colors[i])
            if counter != stop/3:
                faces += str(",")

        output.write("\"materials\": [ \n")
        counter = 0
        print "done2"
        for color in color_index:
            if counter != 0:
                output.write(",")

            output.write("{\n")
            output.write("\"DbgColor\" : 15658734,\n") # => 0xeeeeee
            output.write("\"DbgIndex\" :" + str(counter) + ",\n")
            output.write("\"DbgName\" : \"dummy\",\n")
            #output.write("\"colorDiffuse\" : [ 0, 1, 0 ],\n")
            output.write("\"colorAmbient\" : [ " + color + " ]\n")
            output.write("}\n")
            counter+=1
        output.write("],\n")
        output.write("\"vertices\": [" + vertices + "],\n")

        output.write("\"morphTargets\": [],\n")
        output.write("\"normals\": [],\n")
        output.write("\"colors\": [16580725],\n")

        output.write("\"uvs\": [[]],\n")
        output.write("\"faces\":[" + faces + "]\n")
        output.write("}\n")

