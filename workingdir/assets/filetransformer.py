
import struct
import os

class ObjData:
	def __init__(self, folder, groupname):
		self.positions = []
		self.uvs = []
		self.normals = []

		self.vertices = []
		self.indices = []

		self.vertexmap = dict()
		
		self.folder = folder
		self.groupname = groupname

	def reset(self, groupname):
		if len(self.indices) > 0:
			self.writeToFile()
		self.groupname = groupname
		self.vertices = []
		self.indices = []
		self.vertexmap = dict()

	
	def writeToFile(self):
		print "Writing File '%s'" % (self.groupname + ".data")
		with open(self.folder + "/" + self.groupname + ".data", "wb") as f:
			f.write(self.groupname + "\x00")
			f.write(struct.pack("I",len(self.vertices)))
			print "\tVertices %d" % len(self.vertices)
			print "\tIndices %d" % len(self.indices)
			for vert in self.vertices:
				for i in range(3):
					for j in range(3):
						f.write(struct.pack("f",vert[i][j]))
			f.write(struct.pack("I",len(self.indices)))
			for ind in self.indices:
				f.write(struct.pack("I",ind))
		
	def addVertex(self, vertex):
		if vertex in self.vertexmap:
			self.indices.append(self.vertexmap[vertex])
		else:
			i = len(self.vertices)
			self.indices.append(i)
			self.vertexmap[vertex] = i
			assert vertex[0] != 0 and vertex[1] != 0 and vertex[2] != 0
			self.vertices.append((self.positions[vertex[0] - 1 if vertex[0] > 0 else vertex[0]], self.uvs[vertex[1] - 1 if vertex[1] > 0 else vertex[1]], self.normals[vertex[2] - 1 if vertex[2] > 0 else vertex[2]]))
		
def parseObjFile(folder, filename, scaling = 1.0):
	file = open(folder + "/" + filename, "r")
	groupname = filename.split(".")[0].split("/")[-1]
	
	print "Parsing Obj-File '%s'" % filename
	print "In Folder '%s'" % folder
	objdata = ObjData(folder, groupname)

	biggest_pos = 0.0
	
	for i in file.readlines():
		words = i.split()
		try:
			if len(words) == 0:
				continue
			elif words[0] == "v":
				objdata.positions.append([float(words[1]) / scaling,float(words[2]) / scaling,float(words[3]) / scaling])
				biggest_pos = max(max(max(biggest_pos, float(words[1])), float(words[2])), float(words[3]))
			elif words[0] == "vt":
				if len(words) == 4:
					objdata.uvs.append([float(words[1]), float(words[2]), float(words[3])])
				else:
					objdata.uvs.append([float(words[1]), float(words[2]), 0.0])
			elif words[0] == "vn":
				objdata.normals.append([float(words[1]),float(words[2]),float(words[3])])
			elif words[0] == "f":
				x = words[1].split("/")
				objdata.addVertex( (int(x[0]),int(x[1]),int(x[2])) )
				y = words[2].split("/")
				objdata.addVertex( (int(y[0]),int(y[1]),int(y[2])) )
				z = words[3].split("/")
				objdata.addVertex( (int(z[0]),int(z[1]),int(z[2])) )
			elif words[0] == "g":
				objdata.reset(words[1])
			elif words[0] == "o":
				objdata.reset(words[1])
			elif words[0] == "#":
				pass
			elif words[0] == "usemtl":
				pass
			elif words[0] == "mtllib":
				pass
			elif words[0] == "s":
				pass
			elif words[0] == "l":
				pass#we dont parse lines because reasons
			else:
				print words
		except:
			print i
			
	objdata.writeToFile()
	print "Biggest Position", biggest_pos

parseObjFile("X", "XWing.obj", 759.4686)
parseObjFile("Tie", "Tie_Fighter.obj", 378.103607)
parseObjFile("Gallofree", "Gallofree.obj", 4.403362)

