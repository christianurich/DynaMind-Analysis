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



class AddBox(DM.Module):
    _outputView = DM.View()


    def __init__(self):
        DM.Module.__init__(self)

        self.createParameter("ViewName", DM.STRING, "ViewName")
        self.ViewName = ""

        self.createParameter("x", DM.DOUBLE, "x")
        self.x = 0

        self.createParameter("y", DM.DOUBLE, "y")
        self.y = 0

        self.createParameter("width", DM.DOUBLE, "width")
        self.width = 0

        self.createParameter("height", DM.DOUBLE, "height")
        self.height = 0

        datastream = []
        datastream.append(DM.View("dummy", DM.SUBSYSTEM, DM.WRITE))

        self.addData("Data", datastream)

    def addRect(self, system=DM.System()):
        n1 = system.addNode(self.x, self.y, 0)
        n2 = system.addNode(self.x+self.width, self.y, 0)
        n3 = system.addNode(self.x+self.width, self.y+self.height, 0)
        n4 = system.addNode(self.x, self.y+self.height, 0)

        face = DM.nodevector()
        face.append(n1)
        face.append(n2)
        face.append(n3)
        face.append(n4)

        f = system.addFace(face, self._outputView)


    def init(self):
        if self.ViewName == "":
            return

        self._outputView = DM.View(self.ViewName, DM.FACE, DM.WRITE)
        datastream = []
        datastream.append(self._outputView)
        self.addData("Data", datastream)

    def run(self):
        if self.ViewName == "":
            print "No view defined"
            return
        self._outputView = DM.View(self.ViewName, DM.COMPONENT, DM.WRITE)
        system = self.getData("Data")
        self.addRect(system)
