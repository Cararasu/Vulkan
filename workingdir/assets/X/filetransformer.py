
import struct

groupname = "XWing"

file = open(groupname + ".obj", "r")


positions = []
uvs = []
normals = []

vertices = []
indices = []

vertexmap = dict()

def addVertex(wulps):
	if wulps in vertexmap:
		indices.append(vertexmap[wulps])
	else:
		i = len(vertices)
		indices.append(i)
		vertexmap[wulps] = i
		arr = [wulps[0], wulps[1], wulps[2]]
		if arr[0] > 0:
			arr[0] -= 1
		if arr[1] > 0:
			arr[1] -= 1
		if arr[2] > 0:
			arr[2] -= 1
		vertices.append((positions[arr[0]],uvs[arr[1]],normals[arr[2]]))
def writeToFile():
	print groupname + ".data"
	with open(groupname + ".data", "wb") as f:
		f.write(groupname + "\x00")
		f.write(struct.pack("I",len(vertices)))
		print "Vertices", len(vertices)
		print "Indices", len(indices)
		print "MaxIndices", max(indices)
		for vert in vertices:
			for i in range(3):
				for j in range(3):
					f.write(struct.pack("f",vert[j][i]))
		f.write(struct.pack("I",len(indices)))
		for ind in indices:
			f.write(struct.pack("I",ind))

for i in file.readlines():
	words = i.split()
	if len(words) == 0:
		continue
	elif words[0] == "v":
		positions.append([float(words[1]) / 400.0,float(words[2]) / 400.0,float(words[3]) / 400.0])
	elif words[0] == "vt":
		uvs.append([float(words[1]),float(words[2]),float(words[3])])
	elif words[0] == "vn":
		normals.append([float(words[1]),float(words[2]),float(words[3])])
	elif words[0] == "f":
		x = words[1].split("/")
		xx = (int(x[0]),int(x[1]),int(x[2]))
		y = words[2].split("/")
		yy = (int(y[0]),int(y[1]),int(y[2]))
		z = words[3].split("/")
		zz = (int(z[0]),int(z[1]),int(z[2]))
		
		addVertex(xx)
		addVertex(yy)
		addVertex(zz)
	elif words[0] == "g":
		if len(indices) > 0:
			writeToFile()
		groupname = words[1]
		vertices = []
		indices = []
		vertexmap = dict()
		print "reset", groupname
	elif words[0] == "#":
		pass
	elif words[0] == "usemtl":
		pass
	elif words[0] == "mtllib":
		pass
	elif words[0] == "s":
		pass
	else:
		print words
		
writeToFile()
